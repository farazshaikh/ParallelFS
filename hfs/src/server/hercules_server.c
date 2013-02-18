/*
 * hercules_server.c
 *
 * Hercules server binary
 *
 * fshaikh@cs.cmu.edu
 */

/* This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/> */

#include <hercules_common.h>
#include <hercules_server.h>

//////- Globals for our instance of the file system server -/////////////////////
//-- Get the running instance of the server --//
PHERCULES_SERVER
getHFSServer()
{
    static HERCULES_SERVER herculesServer;
    return &herculesServer;
}

//-- Command line options for starting the server.
PHFS_SERVER_OPTIONS
getHFSServerOptions()
{
    static HFS_SERVER_OPTIONS hfsServerOptions;
    return &hfsServerOptions;
}

//-- Overall file system Context --//
PHFS_FS_CTX *
getPPHFSFSCtx()
{
    static PHFS_FS_CTX     pHfsFSCtx;
    return &pHfsFSCtx;
}

PHFS_FS_CTX
getHFSFSCtx()
{
    return (*getPPHFSFSCtx());
}

////////////////////////////////////////////////////////////////////////////////
void printUsage()
{
    printf("hercules_server fsconfig_file serverid [mds|ds] max_clients");
}


#define MAX_CLIENTS_SUPPORTED 1024
HFS_STATUS
parse_cmd_line_options(int argc,
                       char **argv,
                       PHFS_SERVER_OPTIONS opts)
{
    HFS_STATUS ret=HFS_STATUS_SUCCESS;
    HFS_ENTRY(parse_cmd_line_options);

    if (argc != 5){
        HFS_LOG_ERROR("Cannot Parse Command Line");
        ret = HFS_INTERNAL_ERROR;
        goto leave;
    }
    //-- Get the config file name path --//
    opts->serverConfigFilePath = argv[1];
    opts->logMask       = -1;
    opts->serverID       = strtol(argv[2], NULL, 10);

    //-- Get the server configuration role --//
    if (!strncasecmp(argv[3], "mds", strlen(argv[3]))) {
        opts->role         = SERVER_CONFIG_ROLE_MDS;
    } else {
        opts->role         = SERVER_CONFIG_ROLE_DS;
    }

    opts->maxClientSupported  = strtol(argv[4], NULL, 10);
    if (opts->maxClientSupported > MAX_CLIENTS_SUPPORTED) {
        HFS_LOG_INFO("Unfortunately kernel supports on 1024 sockets in poll");
        HFS_LOG_INFO("Maximum Client Supported == 1024");
        opts->maxClientSupported = MAX_CLIENTS_SUPPORTED;
    }
    ret = HFS_STATUS_SUCCESS;
 leave:
    if (!HFS_SUCCESS(ret)) {
        printUsage();
    }

    HFS_LEAVE(parse_cmd_line_options);
    return ret;
}

HFS_STATUS
hfsBuildServer(PHFS_SERVER_OPTIONS opts,
               PHFS_FS_CTX     phFSFsCtx,
               PHERCULES_SERVER  pHerculesServer)
{
    HFS_STATUS     status=HFS_STATUS_SUCCESS;
    PHFS_SERVER_INFO  thisServerInfo=NULL;
    int         ret;

    HFS_ENTRY(hfsBuildServer);
    //-- Get the host name--//
    status = gethostname(pHerculesServer->config.hostName, MAX_HOST_NAME);
    if (!HFS_SUCCESS(status))
        {
            HFS_LOG_ERROR("Counld not get host name\n");
        }

    if (SERVER_CONFIG_ROLE_DS == opts->role )
        thisServerInfo = hfsGetDSFromServIdx(phFSFsCtx, opts->serverID);
    else if (SERVER_CONFIG_ROLE_MDS == opts->role)
        thisServerInfo = hfsGetMDSFromServIdx(phFSFsCtx, opts->serverID);
    else {
        HFS_LOG_ERROR("Cannot build server without a role");
        status = HFS_INTERNAL_ERROR;
        return status;
    }

    if (NULL == thisServerInfo) {
        HFS_LOG_ERROR("ServerID %d as %s is not valid for this configuration ",
                      opts->serverID, (SERVER_CONFIG_ROLE_DS==opts->role)?"DS":"MDS");
        status = HFS_INTERNAL_ERROR;
        return status;
    }

    // Do some sanity checks based on serverID to hostname matching//
    if (strncasecmp(thisServerInfo->hostName, pHerculesServer->config.hostName, HFS_MAX_HOST_NAME)) {
        HFS_LOG_ERROR("Host name mismatch error You provided %s HFS matched it to %s",
                      thisServerInfo->hostName, pHerculesServer->config.hostName);
        status = HFS_INTERNAL_ERROR;
        return status;
    }


    status = hfsMutexInit(&pHerculesServer->kernel.serverMetaDataUpdateLock);
    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR("Cannot initialize server metadata update mutex");
        return status;
    }

    //-- Ok now lets own the server Info from the configuration as own --//
    pHerculesServer->config.configFilePath = opts->serverConfigFilePath;
    //pHerculesServer->config.hostName   Already set by us
    pHerculesServer->config.maxClientsAllowed = opts->maxClientSupported;
    pHerculesServer->config.role      = opts->role;
    pHerculesServer->config.serverInfo   =*thisServerInfo;

    // Pre processing for the ds & MDS//
    if (SERVER_CONFIG_ROLE_DS == opts->role ) {
        ret = chdir(thisServerInfo->dataStorePath);
        if (ret) {
            HFS_LOG_ERROR("Cannot cd to data store %s %d", thisServerInfo->dataStorePath, errno);
            HFS_LEAVE(formatDS);
            status = HFS_INTERNAL_ERROR;
            return status;
        }
    }

    // Open up the DB Context
    if (SERVER_CONFIG_ROLE_MDS == opts->role) {
        strncpy(getHFSServer()->kernel.hfsDbCtx.filesystemName, getHFSFSCtx()->fsName, MAX_FILE_SYSTEM_NAME);
        status = dbOpenConnection(&getHFSServer()->kernel.hfsDbCtx);
        if (!HFS_SUCCESS(status))
            return status;
    }
    status = HFS_STATUS_SUCCESS;
    return status;
}

void *
hfsWaitForFSUpdates(void *pHercServer)
{
    PHERCULES_SERVER  pHerculesServer;
    struct sembuf    sop;
    int         ret;
    PHFS_QUEUE_ITEM   pQueueItem;
    HFS_PROTO_HEADER  hdr;
    int         i=0;
    HFS_STATUS     status;

    //-Create the semaphore -//
    pHerculesServer = (PHERCULES_SERVER)pHercServer;
    pHerculesServer->kernel.FsUpdateSem = semget(MDS_SERVER_FS_UPDATE_SEM, 1, IPC_CREAT);
    if (-1 == pHerculesServer->kernel.FsUpdateSem) {
        HFS_LOG_ERROR("Cannot create config updater thread, functionality won't work");
        return NULL;
    }

    //- Create the Queue Item -//
    INIT_HEADER(&hdr, CMD_REQ_MDS_FS_CONF_CHANGED); //-- Will be toggeled by out bound layer --//

    //-- wait for ever --//
    while (1) {
        sop.sem_num = 0;
        sop.sem_op = -1;
        sop.sem_flg = 0;

    loop:
        ret = semop(pHerculesServer->kernel.FsUpdateSem, &sop, 1);
        if (-1 == ret && errno == EINTR) {
            goto loop;
        }

        if (-1 == ret) {
            HFS_LOG_ERROR("Wait on fs config update semaphore resulted in %d", errno);
            exit(0);
        }

        //- Ok Now perform sending the unsolicited command to call connected clients -//
        HFS_LOG_INFO("Server Sending Update Notifications to all clients");
        hfsMutexLock(&pHerculesServer->kernel.connectAcceptMutex);
        for(i=0;i<pHerculesServer->config.maxClientsAllowed;i++) {
            if (-1 != pHerculesServer->kernel.pPollFds[i].fd &&
                pHerculesServer->kernel.pPollFds[i].fd != pHerculesServer->kernel.listeningFD) {
                pQueueItem = hfsAllocateProtoBuff(&hdr, getHFSFSCtx()->stripeSize);
                if (!pQueueItem) {
                    HFS_LOG_ERROR("Cannot create config updater thread, functionality won't work");
                    return NULL;
                }
                //- Push the response out -//
                pQueueItem->clientIdx  = i;
                status = hfsQueueItem(getHFSServer()->kernel.pQOutBound, pQueueItem);
                if (!HFS_SUCCESS(status))
                    HFS_LOG_ERROR("Failed to notify client. client may need reboot\n");
            }// for each active client
        }// end for each client
        hfsMutexUnlock(&pHerculesServer->kernel.connectAcceptMutex);
        HFS_LOG_INFO("Done Sending Update Notifications to all clients");
    }// end while
}


//-- Server --//
HFS_STATUS
hfsBuildServerKernel(PHERCULES_SERVER  pHerculesServer)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    int     ret;
    time_t   now;

    srandom((unsigned int)time(&now));
    srand((unsigned int)time(&now));

    //- Client List Information -//
    pHerculesServer->kernel.activeClients    = 0;
    pHerculesServer->kernel.pClientConnCtx   = NULL;
    pHerculesServer->kernel.pPollFds      = NULL;
    pHerculesServer->kernel.pQOutBound     = NULL;
    pHerculesServer->kernel.pQpendingCommands  = NULL;

    //- Server Listener -//
    pHerculesServer->kernel.listeningPort =
        strtoul(pHerculesServer->config.serverInfo.port, NULL, 10);

    pHerculesServer->kernel.addr.sin_family   = AF_INET;
    pHerculesServer->kernel.addr.sin_addr.s_addr = htonl(INADDR_ANY);
    pHerculesServer->kernel.addr.sin_port    = htons(pHerculesServer->kernel.listeningPort);

    pHerculesServer->kernel.timeout       = 0;

    pHerculesServer->kernel.listeningFD = -1;
    pHerculesServer->kernel.serverRunning = 1;


    status = hfsQueueAlloc(&pHerculesServer->kernel.pQOutBound);
    if (!HFS_SUCCESS(status)) {
        pHerculesServer->kernel.pQOutBound = NULL;
        goto cleanup;
    }
    status = hfsQueueAlloc(&pHerculesServer->kernel.pQpendingCommands);
    if (!HFS_SUCCESS(status)) {
        pHerculesServer->kernel.pQOutBound = NULL;
        pHerculesServer->kernel.pQpendingCommands = NULL;
        goto cleanup;
    }

    // TODO Increase threads and debug //

    // Crank up out bound //
    status = hfsQueueInit(pHerculesServer->kernel.pQOutBound,
                          outBoundProcessing,
                          1);
    if (!HFS_SUCCESS(status)) {
        pHerculesServer->kernel.pQpendingCommands = NULL;
        pHerculesServer->kernel.pQOutBound    = NULL;
        HFS_LOG_ERROR("Cannot start the out bound Queue for server");
        goto cleanup;
    }

    // Crank up the pending command Queue //
    if (SERVER_CONFIG_ROLE_MDS == pHerculesServer->config.role)
        {
            status = hfsQueueInit(pHerculesServer->kernel.pQpendingCommands,
                                  mdsPendingCommandProcessing,
                                  HFS_SERVER_INBOUND_THREAD_NR);
        }else if (SERVER_CONFIG_ROLE_DS == pHerculesServer->config.role)
        {
            status = hfsQueueInit(pHerculesServer->kernel.pQpendingCommands,
                                  dsPendingCommandProcessing,
                                  HFS_SERVER_INBOUND_THREAD_NR);
        }

    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR("Cannot start processing queeue for the server");
        pHerculesServer->kernel.pQpendingCommands = NULL;
        pHerculesServer->kernel.pQOutBound    = NULL;
        goto cleanup;
    }

    hfsQueueSetState(pHerculesServer->kernel.pQOutBound, QUEUE_STATE_STARTED);
    hfsQueueSetState(pHerculesServer->kernel.pQpendingCommands, QUEUE_STATE_STARTED);

    //-- Create the thread listening for fs config changes --//
    ret = pthread_create(&pHerculesServer->kernel.thrFsUpdate, NULL, hfsWaitForFSUpdates, pHerculesServer);
    if (ret) {
        HFS_LOG_ERROR("FS config updater thread cannot be created %d", errno);
        return HFS_STATUS_QUEUE_NOT_STARTED;
    }

    //-- Initialize the mutex to accepting connections --//
    hfsMutexInit(&pHerculesServer->kernel.connectAcceptMutex);
    status = HFS_STATUS_SUCCESS;
    goto done;

 cleanup:
    if (pHerculesServer->kernel.pQOutBound)
        hfsQueuehfsFree(pHerculesServer->kernel.pQOutBound);
    if (pHerculesServer->kernel.pQpendingCommands)
        hfsQueuehfsFree(pHerculesServer->kernel.pQpendingCommands);
 done:
    return status;
}


void
hfsServerMarkLastOperation(PHERCULES_SERVER pHerculesServer,
                           __u32 command)
{
    __u32 operationCode = 0;

    switch(command) {
        // MDS_READ
    case CMD_REQ_DS_READ_STRIPE:
    case CMD_RESP_DS_READ_STRIPE:

    case CMD_REQ_DS_READ_STRIPE_V:
    case CMD_RESP_DS_READ_STRIPE_V:

    case CMD_REQ_DS_GET_LENGTH:
    case CMD_RESP_DS_GET_LENGTH:

    case CMD_REQ_DS_GET_DISK_USAGE:
    case CMD_RESP_DS_GET_DISK_USAGE:

    case CMD_REQ_DS_GET_STATS:
    case CMD_RESP_DS_GET_STATS:
        operationCode = LAST_ACT_DS_READ;
        break;

        // MDS_WRITE
    case CMD_REQ_DS_ALLOC_HANDLE:
    case CMD_RESP_DS_ALLOC_HANDLE:

    case CMD_REQ_DS_FREE_HANDLE:
    case CMD_RESP_DS_FREE_HANDLE:

    case CMD_REQ_DS_WRITE_STRIPE:
    case CMD_RESP_DS_WRITE_STRIPE:

    case CMD_REQ_DS_WRITE_STRIPE_V:
    case CMD_RESP_DS_WRITE_STRIPE_V:
        operationCode = LAST_ACT_DS_WRITE;
        break;

        //MDS READ
    case CMD_REQ_MDS_READDIR:
    case CMD_RESP_MDS_READDIR:

    case CMD_REQ_MDS_LOOK_UP:
    case CMD_RESP_MDS_LOOK_UP:

    case CMD_REQ_MDS_GET_ATTR:
    case CMD_RESP_MDS_GET_ATTR:

    case CMD_REQ_MDS_GET_EXTENT_SIZE:
    case CMD_RESP_MDS_GET_EXTENT_SIZE:

    case CMD_REQ_MDS_GET_CONFIG:
    case CMD_RESP_MDS_GET_CONFIG:
        operationCode = LAST_ACT_MDS_READ;
        break;

        //MDS WRITE
    case CMD_REQ_MDS_ALLOC_HANDLE:
    case CMD_RESP_MDS_ALLOC_HANDLE:

    case CMD_REQ_MDS_FREE_HANDLE:
    case CMD_RESP_MDS_FREE_HANDLE:

    case CMD_REQ_MDS_CREATE_DIRENT:
    case CMD_RESP_MDS_CREATE_DIRENT:

    case CMD_REQ_MDS_DELETE_DIRENT:
    case CMD_RESP_MDS_DELETE_DIRENT:

    case CMD_REQ_MDS_SET_ATTR:
    case CMD_RESP_MDS_SET_ATTR:

    case CMD_REQ_MDS_FS_CONF_CHANGED:
    case CMD_RESP_MDS_FS_CONF_CHANGED:
        operationCode = LAST_ACT_MDS_WRITE;
        break;

        // NOP S
    case CMD_REQ_DS_PING:
    case CMD_RESP_DS_PING:

    case CMD_REQ_MDS_PING:
    case CMD_RESP_MDS_PING:
        operationCode = 0;
        break;

    default:
        operationCode = 0;
        break;
    }

    if (operationCode) {
        pHerculesServer->kernel.lastOperation |= operationCode;
    }

}



/*! \fn     main
 * \brief   Entry point for the server
 * \param   argc
 * \param   argv
 * \return   0 upon success error otherwise
 * \callgraph
 */
int main(int argc, char **argv)
{
    HFS_STATUS status=HFS_STATUS_SUCCESS;
    PHFS_FS_CTX pfilesystemContext;
    HFS_ENTRY(main);

    //-- Parse the command line //
    status = parse_cmd_line_options(
                                    argc,
                                    argv,
                                    getHFSServerOptions());
    if (!HFS_SUCCESS(status))
        goto leave;


    //-- Get the context for the entire file system --//
    pfilesystemContext = getHFSFSCtx();
    HFS_UNUSED(pfilesystemContext);
    status = hfsBuildFSContext(
                               getHFSServerOptions()->serverConfigFilePath,
                               getPPHFSFSCtx());
    if (!HFS_SUCCESS(status)) {
        goto leave;
    }

    //- Build up the role for this server --//
    status = hfsBuildServer(
                            getHFSServerOptions(),
                            getHFSFSCtx(),
                            getHFSServer());
    if (!HFS_SUCCESS(status)) {
        goto leave;
    }

    //-- Initialize the logging subsystem --//
    status = loggerInit(getHFSServer()->config.serverInfo.logFile,
                        getHFSServerOptions()->logMask);
    if (!HFS_SUCCESS(status)) {
        goto leave;
    }

    //-- Build up the Server Infra-structure
    //-- These are the core data-structures need for operations--//
    status = hfsBuildServerKernel(getHFSServer());
    if (!HFS_SUCCESS(status)) {
        goto leave;
    }

    //- Wait for inbound Connections now !!-//
    status = hfsInboundLoop(getHFSServer());
    if (!HFS_SUCCESS(status)) {
        goto leave;
    }

    status = HFS_STATUS_SUCCESS;
    fflush(stderr);
    fclose(stderr);
 leave:
    HFS_LEAVE(main);
    return ((int)status);
}
