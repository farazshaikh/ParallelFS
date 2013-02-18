/*
 * hercules_db_lib.c
 *
 * Hercules MYSQL data base library. Namespace resides in a mysql database.
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

#define USER_NAME "root"
#define PASSWORD "hercules"
#define CONN_READ_GRP "client"
#define ONE "1"


HFS_STATUS
__executeQuery(PHFS_DB_CTX pDbCtx,
               char *sql_query)
{
    int error;
    error = mysql_real_query(pDbCtx->pConn, sql_query, strlen(sql_query));

    if (error != 0) {
        HFS_LOG_ERROR("\nDB Error: %s", mysql_error(pDbCtx->pConn));
        return HFS_STATUS_DB_ERROR;
    }

    return HFS_STATUS_SUCCESS;
}


HFS_STATUS
dbFileSystemExist(PHFS_DB_CTX pDbCtx)
{
    HFS_STATUS status;
    char query[HFS_MAX_QUERY + HFS_MAX_FILE_SYSTEM_NAME];

    BUILD_QUERY(query, FMT_QUERY_CHECK_FS, pDbCtx->filesystemName);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        status = HFS_STATUS_DB_FS_NOT_FOUND;
        goto done;
    }
    status = HFS_STATUS_SUCCESS;

 done:
    return status;
}


HFS_STATUS
dbOpenConnection(PHFS_DB_CTX pDbCtx)
{
    HFS_STATUS status;
    char hostName[HFS_DB_HOST_NAME];
    int opt=1;
    MYSQL *p;

    // Open the connection and return it to the user //
    if ((pDbCtx->pConn = mysql_init(NULL))==NULL) {
        HFS_LOG_ERROR("mysql init failed");
        status = HFS_STATUS_DB_INIT_FAILED;
        goto done;
    }

    //-- Set connection options --//
    if (mysql_options (pDbCtx->pConn, MYSQL_READ_DEFAULT_GROUP, CONN_READ_GRP)) {
        status = HFS_STATUS_DB_INIT_FAILED;
        goto done;
    }

    if (mysql_options (pDbCtx->pConn, MYSQL_OPT_CONNECT_TIMEOUT, (char *)&opt)) {
        status = HFS_STATUS_DB_INIT_FAILED;
        goto done;
    }


    status = gethostname(hostName, HFS_DB_HOST_NAME);
    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR("Counld not get host name\n");
        goto done;
    }


    //-- Try connecting to the server --//
    p = mysql_real_connect(pDbCtx->pConn,
                           NULL,
                           USER_NAME,
                           PASSWORD,
                           NULL,
                           0,
                           NULL,
                           0);
    if (NULL == p) {
        HFS_LOG_ERROR("%s ", mysql_error(pDbCtx->pConn));
        status = HFS_STATUS_DB_INIT_FAILED;
        goto done;
    }

    pDbCtx->pConn = p;

    // Start using the db now
    status = dbFileSystemExist(pDbCtx);
 done:
    return status;
}

#define BUILD_AND_EXECUTE_QUERY(QUERY)          \
    BUILD_QUERY(query, FMT_QUERY_##QUERY);      \
    status = __executeQuery(pDbCtx, query);     \
    if (!HFS_SUCCESS(status)) {                 \
        goto done;                              \
    }

HFS_STATUS
dbFormatMDS(PHFS_DB_CTX pDbCtx,
            int serverIdx,
            PHFS_FS_CTX pHFSFSCtx)
{
    HFS_STATUS status;
    char query[HFS_MAX_QUERY];
    __u32 mdscount = 0, dscount = 0;
    PHFS_SERVER_INFO pHfsSrvrInfo;
    time_t now;

    HFS_ENTRY(dbFormatMDS);

    //-- Check if MDS Count and DS Count are within specified range --//
    if (pHFSFSCtx->mdsCount < HFS_MIN_MDS_CNT ||
        pHFSFSCtx->mdsCount > HFS_MAX_MDS_CNT ||
        pHFSFSCtx->dsCount < HFS_MIN_DS_CNT ||
        pHFSFSCtx->dsCount > HFS_MAX_DS_CNT) {
        HFS_LOG_ERROR("Configuration File Error. Check Server Count\n");
        status = HFS_STATUS_CONFIG_ERROR;
        goto done;
    }

    //-- Check if Stripe Size is outside bounds. Set within bounds if required --//
    if (pHFSFSCtx->stripeSize < HFS_MIN_STRIPE_SIZE) {
        pHFSFSCtx->stripeSize = HFS_MIN_STRIPE_SIZE;
        HFS_LOG_INFO("Stripe Size was below minimum, and hence set to Minimum Stripe Size\n");
    }

    if (pHFSFSCtx->stripeSize > HFS_MAX_STRIPE_SIZE) {
        pHFSFSCtx->stripeSize = HFS_MAX_STRIPE_SIZE;
        HFS_LOG_INFO("Stripe Size was above maximum, and hence set to Maximum Stripe Size\n");
    }

    //- If the table exists drop the table --/
    if (HFS_STATUS_SUCCESS == dbFileSystemExist(pDbCtx)) {
        BUILD_QUERY(query, FMT_QUERY_DROP_DATABASE, pDbCtx->filesystemName);
        status = __executeQuery(pDbCtx, query);
        if (!HFS_SUCCESS(status)) {
            goto done;
        }
    }

    //- Create the FS now -//
    BUILD_QUERY(query, FMT_QUERY_CREATE_FS, pDbCtx->filesystemName);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        goto done;
    }

    //- Use the file system -//
    status = dbFileSystemExist(pDbCtx);
    if (!HFS_SUCCESS(status)) {
        goto done;
    }

    //- Create the tables now --//
    BUILD_QUERY(query, FMT_QUERY_CREATE_INODE_TABLE);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        goto done;
    }

    BUILD_QUERY(query, FMT_QUERY_CREATE_DATA_TABLE);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        goto done;
    }

    BUILD_QUERY(query, FMT_QUERY_CREATE_FREE_HANDLE_TABLE);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        goto done;
    }

    BUILD_QUERY(query, FMT_QUERY_CREATE_FILE_SYSTEM_CONFIGURATION);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        goto done;
    }

    BUILD_QUERY(query, FMT_QUERY_CREATE_NAME_SPACE);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        goto done;
    }

    BUILD_QUERY(query, FMT_QUERY_CREATE_GEN_ID_TABLE);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        goto done;
    }

    BUILD_QUERY(query, FMT_QUERY_CREATE_STRIPE_SIZE_TABLE);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        goto done;
    }

    BUILD_QUERY(query, FMT_QUERY_CREATE_USER_TABLE);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        goto done;
    }

    //- Create the root inode -//
    //- (metadata_handle, imode, owner, link_count, grp, a_time, m_time, c_time, size, stripe_cnt)
    now = time(&now);
    BUILD_QUERY(query, FMT_QUERY_ADD_INODE,
                HFS_ROOT_HANDLE,
                HFS_S_IFDIR,
                0,
                2,
                0,
                (__u32)now,
                (__u32)now,
                (__u32)now,
                0,
                0,
                0);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        goto done;
    }

    //- Create the namespace entries for / . & .. -//
    BUILD_QUERY(query, FMT_QUERY_ADD_DIRENT, 0, HFS_ROOT_HANDLE, HFS_S_IFDIR, serverIdx, "/"); //"/"
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        goto done;
    }

    BUILD_QUERY(query, FMT_QUERY_ADD_DIRENT, HFS_ROOT_HANDLE, HFS_ROOT_HANDLE, HFS_S_IFDIR, serverIdx, "."); //"."
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        goto done;
    }

    BUILD_QUERY(query, FMT_QUERY_ADD_DIRENT, HFS_ROOT_HANDLE, HFS_ROOT_HANDLE, HFS_S_IFDIR, serverIdx, ".."); //".."
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        goto done;
    }


    //- If we are the root MDS we need to boot strap the root -//
    if (HFS_ROOT_SERVER_IDX != serverIdx) {
        status = HFS_STATUS_SUCCESS;
        goto done;
    }


    //- Add the Stripe Size value to the table -//
    BUILD_QUERY(query, FMT_QUERY_ADD_STRIPE_SIZE, pHFSFSCtx->stripeSize); //"."
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        goto done;
    }

    //-Add the config file entries in the database -//
    while (mdscount < pHFSFSCtx->mdsCount) {
        pHfsSrvrInfo = hfsGetMDSFromServIdx(pHFSFSCtx, mdscount);

        if (NULL == pHfsSrvrInfo) {
            HFS_LOG_ERROR("ServerID %d is not valid for this configuration\n", mdscount);
            status = HFS_INTERNAL_ERROR;
            goto done;
        }

        BUILD_QUERY(query,
                    FMT_QUERY_ADD_SRVR_CONFIG,
                    pHfsSrvrInfo->hostName,
                    pHfsSrvrInfo->serverIdx,
                    pHfsSrvrInfo->ipAddr,
                    atoi(pHfsSrvrInfo->port),
                    pHfsSrvrInfo->proto,
                    pHfsSrvrInfo->dataStorePath,
                    pHfsSrvrInfo->logFile,
                    pHfsSrvrInfo->role);

        status = __executeQuery(pDbCtx, query);
        if (!HFS_SUCCESS(status)) {
            goto done;
        }
        mdscount++;
    }

    while (dscount < pHFSFSCtx->dsCount) {
        pHfsSrvrInfo = hfsGetDSFromServIdx(pHFSFSCtx, dscount);

        if (NULL == pHfsSrvrInfo) {
            HFS_LOG_ERROR("ServerID %d is not valid for this configuration\n", dscount);
            status = HFS_INTERNAL_ERROR;
            goto done;
        }


        BUILD_QUERY(query,
                    FMT_QUERY_ADD_SRVR_CONFIG,
                    pHfsSrvrInfo->hostName,
                    pHfsSrvrInfo->serverIdx,
                    pHfsSrvrInfo->ipAddr,
                    atoi(pHfsSrvrInfo->port),
                    pHfsSrvrInfo->proto,
                    pHfsSrvrInfo->dataStorePath,
                    pHfsSrvrInfo->logFile,
                    pHfsSrvrInfo->role);

        status = __executeQuery(pDbCtx, query);
        if (!HFS_SUCCESS(status)) {
            goto done;
        }

        dscount++;
    }

    status = HFS_STATUS_SUCCESS;
 done:
    HFS_LEAVE(dbFormatMDS);
    return status;
}

HFS_STATUS
dbCloseConnection (PHFS_DB_CTX pDbCtx)
{
    HFS_ENTRY(dbCloseConnection);
    mysql_close(pDbCtx->pConn);
    HFS_LEAVE(dbCloseConnection);
    return HFS_STATUS_SUCCESS;
}

HFS_STATUS
dbGetError (PHFS_DB_CTX pDbCtx, char * errorStr, size_t strSize)
{
    snprintf (errorStr, strSize, "%s", mysql_error(pDbCtx->pConn));
    return HFS_STATUS_SUCCESS;
}


// Queries required for working //
HFS_STATUS dbGetDirListing(PHFS_DB_CTX pDbCtx,
                           __u32 parent_handle,
                           char *buffer,
                           int size,
                           IN __u32 offset,
                           IN OUT __u32 *copiedDirent)
{
    HFS_STATUS status;
    char query[HFS_MAX_QUERY];
    int i=0, direntPerExtent, nrFields;
    MYSQL_RES *result;
    MYSQL_ROW row;
    HFS_DIRENT *pDirent;
    HFS_ENTRY(dbGetDirListing);

    *copiedDirent = 0;
    BUILD_QUERY(query, FMT_QUERY_MDS_READDIR, parent_handle);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status))
        return status;

    result = mysql_store_result(pDbCtx->pConn);
    if (NULL == result) {
        HFS_LOG_ERROR("%s", mysql_error(pDbCtx->pConn));
        return HFS_STATUS_DB_ERROR;
    }

    // Crank up dir to dir offset //
    row = NULL;
    while (i++ < offset && (row = mysql_fetch_row(result)));

    if (offset && !row) {
        status = HFS_STATUS_DB_NO_SUCH_RECORD;
        goto done;
    }

    direntPerExtent = size / sizeof(HFS_DIRENT);
    pDirent = (HFS_DIRENT *) buffer;

    for( i=0 ; i < direntPerExtent ; i++) {
        row = NULL;
        row = mysql_fetch_row(result);

        if (!row) {
            status = HFS_STATUS_SUCCESS;
            goto done;
        }

        nrFields = mysql_num_fields(result);
        if (nrFields != 4) {
            status = HFS_STATUS_DB_ERROR;
            goto done;
        }

        pDirent->parentMetaDataHandle = htonl((__u32)atoi(row[0]));
        pDirent->selfMetaDataHandle = htonl((__u32)atoi(row[1]));
        pDirent->dirEntMode = htonl((__u32)atoi(row[2]));
        row[3]?strncpy((char *)pDirent->dname, row[3], HFS_MAX_FILE_NAME):NULL;

        (*copiedDirent)++;
        pDirent++;
    }
    status = HFS_STATUS_SUCCESS;

 done:
    if (result) {
        mysql_free_result(result);
    }

    HFS_LEAVE(dbGetDirListing);
    return status;
}

HFS_STATUS
dbLookUp(PHFS_DB_CTX pDbCtx,
         __u32 parent_handle,
         char *dname,
         HFS_META_DATA_HANDLE *pHFSMetaDataHandle,
         __u32 *serverId)
{
    HFS_STATUS status;
    char query[HFS_MAX_QUERY];
    MYSQL_RES *result;
    MYSQL_ROW row;

    HFS_ENTRY();
    BUILD_QUERY(query, FMT_QUERY_MDS_LOOKUP, parent_handle, dname);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        return status;
    }

    result = mysql_store_result(pDbCtx->pConn);
    if (NULL == result) {
        HFS_LOG_ERROR("%s", mysql_error(pDbCtx->pConn));
        return HFS_STATUS_DB_ERROR;
    }

    row = mysql_fetch_row(result);
    if (!row) {
        status = HFS_STATUS_DB_NO_SUCH_RECORD;
        goto done;
    }
    *pHFSMetaDataHandle = (HFS_META_DATA_HANDLE)atoi(row[0]);
    *serverId = (__u32)atoi(row[1]);
    status = HFS_STATUS_SUCCESS;

 done:
    if (result) {
        mysql_free_result(result);
    }

    HFS_LEAVE(dbGetDirListing);
    return status;
}



HFS_STATUS dbGetAttr(PHFS_DB_CTX pDbCtx,
                     HFS_META_DATA_HANDLE self_handle,
                     PHFS_IATTR pIAttr)
{
    HFS_STATUS status;
    char query[HFS_MAX_QUERY];
    MYSQL_RES *result=NULL, *result2=NULL;
    MYSQL_ROW row;
    int dataHandlesCnt;

    HFS_ENTRY();
    BUILD_QUERY(query, FMT_QUERY_MDS_GET_ATTR, self_handle);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        return status;
    }


    result = mysql_store_result(pDbCtx->pConn);
    if (NULL == result) {
        HFS_LOG_ERROR("%s", mysql_error(pDbCtx->pConn));
        return HFS_STATUS_DB_ERROR;
    }

    row = mysql_fetch_row(result);
    if (!row) {
        status = HFS_STATUS_DB_NO_SUCH_RECORD;
        goto done;
    }

    pIAttr->selfMetaDataHandle = (__u32)atoi(row[0]);
    pIAttr->iMode = (__u32)atoi(row[1]);
    pIAttr->owner = (__u32)atoi(row[2]);
    pIAttr->linkCount = (__u32)atoi(row[3]);
    pIAttr->group = (__u32)atoi(row[4]);
    pIAttr->a_time = (__u32)atoi(row[5]);
    pIAttr->m_time = (__u32)atoi(row[6]);
    pIAttr->c_time = (__u32)atoi(row[7]);
    pIAttr->size = (__u32)atoi(row[8]);
    pIAttr->stripe_cnt = (__u32)atoi(row[9]);
    pIAttr->rdev_t = (__u32)atoi(row[10]);

    // Also now we need the data handles //
    BUILD_QUERY(query, FMT_QUERY_MDS_GET_DATA_HANDLES, self_handle);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        goto done;
    }


    result2 = mysql_store_result(pDbCtx->pConn);
    if (NULL == result2) {
        HFS_LOG_ERROR("%s", mysql_error(pDbCtx->pConn));
        status = HFS_STATUS_DB_ERROR;
        goto done;
    }

    for(dataHandlesCnt = 0 ;
        dataHandlesCnt < pIAttr->stripe_cnt * TOTAL_COPIES ; dataHandlesCnt++) {
        row = mysql_fetch_row(result2);
        if (!row) {
            break; // OK TO HAVE FILES WITH NO EXTENT COULD BE DIRS
        }

        pIAttr->dataHandlesSrvs[dataHandlesCnt] = (HFS_SERVER_ID)atoi(row[0]);
        pIAttr->dataHandles[dataHandlesCnt] = (HFS_DATA_HANDLE)atoi(row[1]);
    }

    status = HFS_STATUS_SUCCESS;

 done:
    if (result) {
        mysql_free_result(result);
    }

    if (result2) {
        mysql_free_result(result2);
    }

    HFS_LEAVE(dbGetDirListing);
    return status;
}

HFS_STATUS
dbSetAttr(PHFS_DB_CTX pDbCtx,
          PHFS_IATTR pIAttr,
          __u32 attrMask)
{
    HFS_STATUS status;
    char query[HFS_MAX_QUERY];
    MYSQL_RES *result;
    MYSQL_ROW row;
    int dataHandlesCnt;
    HFS_ENTRY();
    //metadata_handle, imode, owner, link_count, grp, a_time, m_time, c_time, size, strip_cnt)
    BUILD_QUERY(query, FMT_QUERY_MDS_SET_ATTR,
                pIAttr->iMode,
                pIAttr->owner,
                pIAttr->linkCount,
                pIAttr->group,
                pIAttr->a_time,
                pIAttr->m_time,
                pIAttr->c_time,
                pIAttr->size,
                pIAttr->stripe_cnt,
                pIAttr->rdev_t,
                pIAttr->selfMetaDataHandle);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR("Cannot Set attributes for meta-data handle %x", pIAttr->selfMetaDataHandle);
        return status;
    }

    // We are not requested to set the data handles
    if (!(attrMask & MDS_ATTR_MASK_DH)) {
        return status;
    }


    // Update the data handles if requested and is logically valid//
    // Also now we need the data handles //
    BUILD_QUERY(query, FMT_QUERY_MDS_GET_DATA_HANDLES, pIAttr->selfMetaDataHandle);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        return status;
    }

    result = mysql_store_result(pDbCtx->pConn);
    if (NULL == result) {
        HFS_LOG_ERROR("%s", mysql_error(pDbCtx->pConn));
        return HFS_STATUS_DB_ERROR;
    }

    row = mysql_fetch_row(result);
    if (row) {
        HFS_LOG_ERROR("Once set the data-handles mapping cannot be reset"
                      "File already has a datahandle mapping. Please check client logic");
        status = HFS_PROTOCOL_ERROR;
        goto done;
    }

    // Ok so now update the handles //
    for(dataHandlesCnt = 0;
        dataHandlesCnt < pIAttr->stripe_cnt * 2;
        dataHandlesCnt++) {
        BUILD_QUERY(query, FMT_QUERY_MDS_SET_DATA_HANDLES,
                    pIAttr->selfMetaDataHandle,
                    pIAttr->dataHandlesSrvs[dataHandlesCnt],
                    pIAttr->dataHandles[dataHandlesCnt]);
        status = __executeQuery(pDbCtx, query);
        if (!HFS_SUCCESS(status)) {
            goto done;
        }
    }

 done:
    if (result) {
        mysql_free_result(result);
    }

    HFS_LEAVE(dbSetAttr);
    return status;
}

HFS_STATUS dbAllocMetaDataHandle(PHFS_DB_CTX pDbCtx,
                                 HFS_META_DATA_HANDLE *pMetaDataHandle)
{
    char query[HFS_MAX_QUERY];
    MYSQL_RES *result=NULL, *result2=NULL;
    MYSQL_ROW row;
    HFS_META_DATA_HANDLE newDataHandle=HFS_INVALID_HANDLE;
    HFS_STATUS status;
    time_t now;

    HFS_ENTRY();
    // Select the minimum from the data handle list //
    BUILD_QUERY(query, FMT_QUERY_GET_MIN_FREE_META_HANDLE);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        return status;
    }

    result = mysql_store_result(pDbCtx->pConn);
    if (NULL == result) {
        HFS_LOG_ERROR("%s", mysql_error(pDbCtx->pConn));
        return HFS_STATUS_DB_ERROR;
    }

    row = mysql_fetch_row(result);
    if (row && row[0]) {
        newDataHandle = (__u32)atoi(row[0]);
        BUILD_QUERY(query, FMT_QUERY_REMOVE_FREE_META_HANDLE, newDataHandle);
        status = __executeQuery(pDbCtx, query);

        if (!HFS_SUCCESS(status)) {
            newDataHandle = HFS_INVALID_HANDLE;
            goto done;
        }

    } else {
        newDataHandle = HFS_INVALID_HANDLE;
    }

    // If we have not found a meta-data handle in the free list
    // Select maximum from the inode table //
    if (HFS_INVALID_HANDLE == newDataHandle) {
        BUILD_QUERY(query, FMT_QUERY_GET_MAX_META_HANDLE);
        status = __executeQuery(pDbCtx, query);
        if (!HFS_SUCCESS(status)) {
            goto done;
        }

        result2 = mysql_store_result(pDbCtx->pConn);
        if (NULL == result2) {
            HFS_LOG_ERROR("%s", mysql_error(pDbCtx->pConn));
            status = HFS_STATUS_DB_ERROR;
            goto done;
        }

        row = mysql_fetch_row(result2);
        if (row && row[0]) {
            newDataHandle = (__u32)atoi(row[0]) + 1;
        } else {
            newDataHandle = HFS_INVALID_HANDLE;
        }
    }

    //- Ok by now we should have a valid handle if not then we are in trouble -//
    if (HFS_INVALID_HANDLE == newDataHandle) {
        status = HFS_STATUS_DB_ERROR;
        goto done;
    }

    now = time(&now);
    //- (metadata_handle, imode, owner, link_count, grp, a_time, m_time, c_time, size, stripe_cnt)
    BUILD_QUERY(query, FMT_QUERY_ADD_INODE,
                newDataHandle,
                0,
                0,
                0,
                0,
                (__u32)now,
                (__u32)now,
                (__u32)now,
                0,
                0,
                0);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        status = HFS_STATUS_DB_ERROR;
        goto done;
    }

    *pMetaDataHandle = newDataHandle;
    status = HFS_STATUS_SUCCESS;

 done:
    if (result) {
        mysql_free_result(result);
    }

    if (result2) {
        mysql_free_result(result2);
    }
    HFS_LEAVE(dbSetAttr);
    return status;
}

HFS_STATUS dbFreeHandle(PHFS_DB_CTX pDbCtx,
                        HFS_META_DATA_HANDLE metaDataHandle)
{
    return HFS_PROTO_STATUS_NOT_IMPLEMENTED;
}

HFS_STATUS dbAddDirent(PHFS_DB_CTX pDbCtx,
                       PHFS_DIRENT pDirent)
{
    HFS_STATUS status;
    char query[HFS_MAX_QUERY];

    BUILD_QUERY(query, FMT_QUERY_ADD_DIRENT,
                pDirent->parentMetaDataHandle,
                pDirent->selfMetaDataHandle,
                pDirent->dirEntMode,
                pDirent->serverId,
                pDirent->dname);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status)) {
        return status;
    }

    return HFS_STATUS_SUCCESS;
}

HFS_STATUS dbGetEncryptedNonce(PHFS_DB_CTX pDbCtx,
                               char *userName,
                               __u64 *prandomNonce,
                               __u64 *pencryptedRandomNonce)
{
    HFS_STATUS status;
    char query[HFS_MAX_QUERY];
    int encrpytion_status;
    MYSQL_RES *result;
    MYSQL_ROW row;
    __u64 randomNonce;
    __u32 *pTemp;
    __u64 encryptedRandomNonce;
    char password[HFS_MAX_ENC_PASSWORD + 1];

    pTemp = (__u32 *)&randomNonce;
    pTemp[0] = random();
    pTemp[1] = random();

    encryptedRandomNonce = randomNonce;
    memset(password, 0, HFS_MAX_ENC_PASSWORD + 1);


    HFS_ENTRY();
    BUILD_QUERY(query, FMT_QUERY_GET_ENCRYPTED_PASS, userName);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status))
        return status;

    result = mysql_store_result(pDbCtx->pConn);
    if (NULL == result) {
        HFS_LOG_ERROR("%s", mysql_error(pDbCtx->pConn));
        return HFS_STATUS_DB_ERROR;
    }

    row = mysql_fetch_row(result);
    if (!row) {
        status = HFS_STATUS_DB_NO_SUCH_RECORD;
        goto done;
    }

    if (!row[0]) {
        return HFS_STATUS_DB_NO_SUCH_RECORD;
    }

    strncpy(password, row[0], HFS_MAX_ENC_PASSWORD);
    des_setparity(password);
    encrpytion_status = ecb_crypt(password, (char *)&encryptedRandomNonce, sizeof(encryptedRandomNonce), DES_ENCRYPT | DES_SW);

    if (DES_FAILED(encrpytion_status)) {
        return HFS_STATUS_AUTH_FAILED;
    }

    *prandomNonce = randomNonce;
    *pencryptedRandomNonce = encryptedRandomNonce;

    status = HFS_STATUS_SUCCESS;
 done:
    //mysql_free_result(result);
    HFS_LEAVE();
    return status;
}



HFS_STATUS dbGetExtentSize(PHFS_DB_CTX pDbCtx,
                           __u32 *extentSize)
{
    HFS_STATUS status;
    char query[HFS_MAX_QUERY];
    MYSQL_RES *result;
    MYSQL_ROW row;

    HFS_ENTRY();
    BUILD_QUERY(query, FMT_QUERY_MDS_GET_EXTENT_SIZE);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status))
        return status;

    result = mysql_store_result(pDbCtx->pConn);
    if (NULL == result) {
        HFS_LOG_ERROR("%s", mysql_error(pDbCtx->pConn));
        return HFS_STATUS_DB_ERROR;
    }

    row = mysql_fetch_row(result);
    if (!row) {
        status = HFS_STATUS_DB_NO_SUCH_RECORD;
        goto done;
    }
    *extentSize = (__u32)atoi(row[0]);
    status = HFS_STATUS_SUCCESS;

 done:
    mysql_free_result(result);
    HFS_LEAVE();
    return status;
}


HFS_STATUS dbGetSrvrListing(PHFS_DB_CTX pDbCtx,
                            char *buffer,
                            int size,
                            IN __u32 offset,
                            IN OUT __u32 *copiedSrvrent)
{
    HFS_STATUS status;
    char query[HFS_MAX_QUERY];
    int i=0, srvrentPerExtent, nrFields;
    MYSQL_RES *result;
    MYSQL_ROW row;
    HFS_SERVER_INFO *pSrvrInfo;
    HFS_ENTRY(dbGetSrvrListing);

    *copiedSrvrent = 0;
    BUILD_QUERY(query, FMT_QUERY_MDS_GET_CONFIG);
    status = __executeQuery(pDbCtx, query);
    if (!HFS_SUCCESS(status))
        return status;

    result = mysql_store_result(pDbCtx->pConn);
    if (NULL == result)
        {
            HFS_LOG_ERROR("%s", mysql_error(pDbCtx->pConn));
            return HFS_STATUS_DB_ERROR;
        }

    // Crank up dir to dir offset //
    row = NULL;
    while (i++ < offset && (row = mysql_fetch_row(result)));
    if (offset && !row) {
        status = HFS_STATUS_DB_NO_SUCH_RECORD;
        goto done;
    }

    srvrentPerExtent = size / sizeof(HFS_SERVER_INFO);
    pSrvrInfo = (HFS_SERVER_INFO *) buffer;
    for(i=0;i<srvrentPerExtent;i++) {
        row = NULL;
        row = mysql_fetch_row(result);

        if (!row) {
            status = HFS_STATUS_SUCCESS;
            goto done;
        }

        nrFields = mysql_num_fields(result);
        if (nrFields != 8) {
            status = HFS_STATUS_DB_ERROR;
            goto done;
        }

        row[0]?strncpy(pSrvrInfo->hostName, row[0], HFS_MAX_HOST_NAME):NULL;
        pSrvrInfo->serverIdx = htonl((__u32)atoi(row[1]));
        row[2]?strncpy(pSrvrInfo->ipAddr, row[2], HFS_MAX_IP_ADDRESS):NULL;
        row[3]?strncpy(pSrvrInfo->port, row[3], HFS_MAX_PORT_STR):NULL;
        row[4]?strncpy(pSrvrInfo->proto, row[4], HFS_MAX_PROTO_STR):NULL;
        row[5]?strncpy(pSrvrInfo->dataStorePath, row[5], HFS_MAX_DATA_STORE_PATH):NULL;
        row[6]?strncpy(pSrvrInfo->logFile, row[6], HFS_MAX_LOG_FILE_PATH):NULL;
        pSrvrInfo->role = htonl((__u32)atoi(row[7]));

        (*copiedSrvrent)++;
        pSrvrInfo++;
    }
    status = HFS_STATUS_SUCCESS;
 done:
    if (result) {
        mysql_free_result(result);
    }
    HFS_LEAVE(dbGetDirListing);
    return status;
}
