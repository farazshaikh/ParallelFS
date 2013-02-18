/*
 * mkfs_hercules.c --
 *
 *      mkfs for the hercules filesystem
 *      fshaikh@cs.cmu.edu
 */

/*  This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/> */

#include <hercules_common.h>
#include <hercules_server.h>

void
printUsage() {
    HFS_LOG_ERROR("mkfs.hercules fsconfig_file serverid [mds|ds]");
}

//-- Command line options for formatting the server --//
PHFS_SERVER_OPTIONS getHFSServerOptions() {
    static HFS_SERVER_OPTIONS  hfsServerOptions;
    return &hfsServerOptions;
}

//-- Overall file system Context --//
PHFS_FS_CTX * getPPHFSFSCtx() {
    static PHFS_FS_CTX          pHfsFSCtx;
    return &pHfsFSCtx;
}

PHFS_FS_CTX
getHFSFSCtx()
{
    return (*getPPHFSFSCtx());
}

HFS_STATUS
parse_cmd_line_options(int argc,
                       char **argv,
                       PHFS_SERVER_OPTIONS opts)
{
    HFS_STATUS ret=HFS_STATUS_SUCCESS;
    HFS_ENTRY(parse_cmd_line_options);

    if (argc != 4){
        HFS_LOG_ERROR("Cannot Parse Command Line");
        ret = HFS_INTERNAL_ERROR;
        goto leave;
    }
    //-- Get the config file name path --//
    opts->serverConfigFilePath = argv[1];
    opts->logMask              = -1;
    opts->serverID             = strtol(argv[2],  NULL,  10);

    //-- Get the server configuration role --//
    if (!strncasecmp(argv[3], "mds", strlen(argv[3]))) {
        opts->role                 = SERVER_CONFIG_ROLE_MDS;
    } else {
        opts->role                 = SERVER_CONFIG_ROLE_DS;
    }

    ret = HFS_STATUS_SUCCESS;
 leave:
    if (!HFS_SUCCESS(ret)) {
        printUsage();
    }

    HFS_LEAVE(parse_cmd_line_options);
    return ret;
}

/*
 *  formatDS
 *
 *     Format a data server
 *
 *  Parameters:
 *     thisServerInfo       - server info
 *
 *  Results:
 *      HFS_STATUS_OK on success.
 *
 *  Side Effects:
 *      None
 */
HFS_STATUS
formatDS(PHFS_SERVER_INFO thisServerInfo)
{
    HFS_STATUS  status;
    int         ret;
    char        c;
    int         fd;
    char            fileName[MAX_EXTENT_FILE_NAME_LEN];
    HFS_DATA_HANDLE handle;

    HFS_ENTRY(formatDS);
    ret = chdir(thisServerInfo->dataStorePath);
    if (ret) {
        HFS_LOG_ERROR("Cannot cd to data store %s %d", thisServerInfo->dataStorePath, errno);
        HFS_LEAVE(formatDS);
        return HFS_INTERNAL_ERROR;
    }

    // create the data file handle //
    status = hdlToFileName(HFS_DATA_SERVER_META_FILE_HANDLE, fileName);
    if (!HFS_SUCCESS(status)) {
        return HFS_INTERNAL_ERROR;
    }

    // Check if the file is present
    ret = access(fileName, F_OK);
    if (!ret) {
        printf("Old fs handle file exists do you want to proceed with the format[Y]/N:");
        scanf("%c", &c);
        if (c != 'Y' && c != 'y') {
            return(HFS_INTERNAL_ERROR);
        }
    }

    //- Create the handle file -//
    fd = creat(fileName, O_RDWR);
    if (-1 == fd) {
        HFS_LOG_ERROR("Cannot create handle file %s", fileName);
        return HFS_INTERNAL_ERROR;
    }

    // Write out the first handle to the ds meta - file //
    handle = HFS_FIRST_DATA_HANDLE;
    ret = write(fd, &handle, sizeof(HFS_FIRST_DATA_HANDLE));
    if (sizeof(HFS_FIRST_DATA_HANDLE) != ret) {
        HFS_LOG_ERROR("Error writing back updated handle");
        return HFS_STATUS_CONFIG_ERROR;
    }
    close(fd);

    printf("Success\n");
    return HFS_STATUS_SUCCESS;
}

/*
 *  formatMDS
 *
 *     Format a meta data server
 *
 *  Parameters:
 *     thisServerInfo       - server info
 *
 *  Results:
 *      HFS_STATUS_OK on success.
 *
 *  Side Effects:
 *      None
 */
HFS_STATUS
formatMDS(PHFS_SERVER_INFO thisServerInfo)
{
    HFS_STATUS   status;
    HFS_DB_CTX   dbCtx;
    char         c;

    HFS_ENTRY(formatMDS);
    strncpy(thisServerInfo->filesystemName, getHFSFSCtx()->fsName, MAX_FILE_SYSTEM_NAME);
    strncpy(dbCtx.filesystemName, getHFSFSCtx()->fsName, MAX_FILE_SYSTEM_NAME);
    status = dbOpenConnection(&dbCtx);

    if (!HFS_SUCCESS(status) && HFS_STATUS_DB_FS_NOT_FOUND != status) {
        return status;
    }

    if (HFS_STATUS_DB_FS_NOT_FOUND != status) {
        printf("Old fs exists do you really want to proceed with the \
                format[Y]/N:");
        scanf("%c", &c);
        if (c != 'Y' && c != 'y') {
            dbCloseConnection (&dbCtx);
            return(HFS_INTERNAL_ERROR);
        }
    }

    //- Format the database -//
    status = dbFormatMDS(&dbCtx, thisServerInfo->serverIdx,  getHFSFSCtx());


    // Add the users now //
    if (HFS_SUCCESS(status) && thisServerInfo->serverIdx == HFS_ROOT_SERVER_IDX) {
        while(1) {
            char userName[HFS_MAX_USER_NAME+1];
            char password[HFS_MAX_ENC_PASSWORD+1];
            char *pass;
            char query[HFS_MAX_QUERY];

            fflush(stdin);
            fflush(stdout);

            memset(userName, 0, HFS_MAX_USER_NAME+1);
            memset(password, 0, HFS_MAX_ENC_PASSWORD+1);

            printf("\nUser Name:");
            scanf("%8s", userName);

            pass = getpass("Password:");
            memcpy(password, pass, HFS_MAX_ENC_PASSWORD);
            pass = crypt(password, HFS_CRYPT_SALT);
            memcpy(password, pass, HFS_MAX_ENC_PASSWORD);

            //-- password --//
            BUILD_QUERY(query,  FMT_QUERY_ADD_USER, userName, password);
            status = __executeQuery(&dbCtx, query);
            if (!HFS_SUCCESS(status)) {
                printf("Failed to add User");
            } else {
                printf("User Added Successfully");
            }

            printf(" Add More Users [Y/N]:");
            scanf(" %c",  &c);

            if (c != 'y' && c != 'Y') {
                break;
            }
        }
    }

    dbCloseConnection (&dbCtx);
    HFS_LEAVE(formatMDS);
    return status;
}

int main(int argc, char **argv)
{
    HFS_ENTRY(main);
    HFS_STATUS          status;
    char                hostName[MAX_HOST_NAME];
    PHFS_SERVER_INFO    thisServerInfo=NULL;

    //-- Parse the command line //
    status = parse_cmd_line_options(argc,
                                    argv,
                                    getHFSServerOptions());
    if (!HFS_SUCCESS(status)) {
        goto leave;
    }

    //-- Read the FS context --//
    status = hfsBuildFSContext(getHFSServerOptions()->serverConfigFilePath,
                               getPPHFSFSCtx());
    if (!HFS_SUCCESS(status)) {
        goto leave;
    }

    //-- Read the hostname of this machine --//
    status = gethostname(hostName, MAX_HOST_NAME);
    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR("Could not get host name\n");
        goto leave;
    }

    //-- Does this machine appear in the configuration -//
    if (SERVER_CONFIG_ROLE_DS == getHFSServerOptions()->role ) {
        thisServerInfo = hfsGetDSFromServIdx(getHFSFSCtx(), getHFSServerOptions()->serverID);
    } else if (SERVER_CONFIG_ROLE_MDS == getHFSServerOptions()->role) {
        thisServerInfo = hfsGetMDSFromServIdx(getHFSFSCtx(), getHFSServerOptions()->serverID);
    } else {
        HFS_LOG_ERROR("Cannot build server without a role");
        status = HFS_INTERNAL_ERROR;
        goto leave;
    }

    //-- Is the server the same as it appears in the configuration --//
    if (strncasecmp(thisServerInfo->hostName, hostName, HFS_MAX_HOST_NAME)) {
        HFS_LOG_ERROR("Host name mismatch error You provided %s  HFS matched it to %s", 
                      thisServerInfo->hostName, hostName);
        status = HFS_INTERNAL_ERROR;
        goto leave;
    }

    //-- Format the filesystem -//
    //-- Is a data server      -//
    if (SERVER_CONFIG_ROLE_DS == getHFSServerOptions()->role ) {
        HFS_LOG_INFO("Formating %s server as DS", hostName);
        status = formatDS(thisServerInfo);
    } else {
        HFS_LOG_INFO("Formating %s server as MDS", hostName);
        status = formatMDS(thisServerInfo);
    }

 leave:
    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR("Server formatting failed %ld", status);
    }

    HFS_LEAVE(main);
    return ((int) status);
}
