/*
 * hercules_inbound.c
 *
 * Hercules server meta data server command dispatch
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

/*MDS Command Processing */
//REQ_MDS_PING
//RESP_MDS_PING
HFS_STATUS
hfsService_REQ_MDS_PING(PHFS_QUEUE_ITEM pQueueItem)
{
    HFS_STATUS status=HFS_STATUS_SUCCESS;
    PHFS_SERVER_REQ pServReq=NULL;
    PHFS_SERVER_RESP pServResp=NULL;
    HFS_ENTRY(hfsService_REQ_MDS_PING);

    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);
    HFS_UNUSED(pServReq);
    pServResp = (PHFS_SERVER_RESP) (pQueueItem + 1);


    //- Just build up the response packet and sent it out -//
    pServResp->hdr.status = HFS_PROTO_STATUS_SUCCESS;
    pServResp->resp.respMDSPing.cookie = getHFSServer()->kernel.lastOperation;
    getHFSServer()->kernel.lastOperation -= getHFSServer()->kernel.lastOperation;
    //-! Begin Command Processing !-//
    // None just reflect back stuff //
    //-! End Command Processing !-//
    status = HFS_STATUS_SUCCESS;
    HFS_LEAVE(hfsService_REQ_MDS_PING);
    return status;
}

//REQ_MDS_ALLOC_HANDLE
//RESP_MDS_ALLOC_HANDLE
HFS_STATUS
hfsService_REQ_MDS_ALLOC_HANDLE(PHFS_QUEUE_ITEM pQueueItem)
{
    HFS_STATUS status=HFS_STATUS_SUCCESS;
    PHFS_SERVER_REQ pServReq=NULL;
    PHFS_SERVER_RESP pServResp=NULL;
    HFS_ENTRY(hfsService_REQ_MDS_ALLOC_HANDLE);

    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);
    pServResp = (PHFS_SERVER_RESP) (pQueueItem + 1);

    //- Just build up the response packet and sent it out -//
    //-! Begin Command Processing !-//
    pServResp->resp.respMDSAllocHandle.serverId = pServReq->req.reqMDSAllocHandle.serverId;

    status = hfsMutexLock(&getHFSServer()->kernel.serverMetaDataUpdateLock);
    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR("Failed to acquire server metadata update lock");
        pServResp->hdr.status = status;
        return HFS_STATUS_SUCCESS;
    }

    pServResp->hdr.status =
        dbAllocMetaDataHandle(
                              &getHFSServer()->kernel.hfsDbCtx,
                              &pServResp->resp.respMDSAllocHandle.metaDataHandle);

    status = hfsMutexUnlock(&getHFSServer()->kernel.serverMetaDataUpdateLock);
    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR("Failed to release metadata update lock bailing out");
        exit(0);
    }


    //-! End Command Processing !-//
    status = HFS_STATUS_SUCCESS;
    HFS_LEAVE(hfsService_REQ_MDS_ALLOC_HANDLE);
    return status;
}

//REQ_MDS_FREE_HANDLE
//RESP_MDS_FREE_HANDLE
HFS_STATUS
hfsService_REQ_MDS_FREE_HANDLE(PHFS_QUEUE_ITEM pQueueItem)
{
    HFS_STATUS status=HFS_STATUS_SUCCESS;
    PHFS_SERVER_REQ pServReq=NULL;
    PHFS_SERVER_RESP pServResp=NULL;
    HFS_ENTRY(hfsService_REQ_MDS_ALLOC_HANDLE);

    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);
    HFS_UNUSED(pServReq);
    pServResp = (PHFS_SERVER_RESP) (pQueueItem + 1);

    //- Just build up the response packet and sent it out -//
    pServResp->hdr.status = HFS_PROTO_STATUS_NOT_IMPLEMENTED;
    //-! Begin Command Processing !-//

    //-! End Command Processing !-//
    status = HFS_STATUS_SUCCESS;
    goto leave;

    goto cleanup; // F**C the compiler
 cleanup:
 leave:
    HFS_LEAVE(hfsService_REQ_MDS_ALLOC_HANDLE);
    return status;
}

//REQ_MDS_READDIR
//RESP_MDS_READDIR
HFS_STATUS
hfsService_REQ_MDS_READDIR(PHFS_QUEUE_ITEM pQueueItem)
{
    HFS_STATUS status=HFS_STATUS_SUCCESS;
    PHFS_SERVER_REQ pServReq=NULL;
    PHFS_SERVER_RESP pServResp=NULL;
    __u32 copiedDirents;

    HFS_ENTRY(hfsService_REQ_MDS_READDIR);

    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);
    pServResp = (PHFS_SERVER_RESP) (pQueueItem + 1);

    //-! Begin Command Processing !-//
    //- Just build up the response packet and sent it out -//
    pServResp->hdr.status =
        dbGetDirListing(
                        &getHFSServer()->kernel.hfsDbCtx,
                        pServReq->req.reqMDSReaddir.metaDataHandle,
                        (char *)pServResp + sizeof(HFS_PROTO_HEADER) + sizeof(RESP_MDS_READDIR),
                        getHFSFSCtx()->stripeSize,
                        pServReq->req.reqMDSReaddir.direntOffset,
                        &copiedDirents
                        );
    pServResp->resp.respMDSReaddir.direntCount = copiedDirents;
    pServResp->resp.respMDSReaddir.metaDataHandle = pServReq->req.reqMDSReaddir.metaDataHandle;
    pServResp->hdr.pduExtentSize  = copiedDirents * sizeof(HFS_DIRENT);

    //-! End Command Processing !-//
    status = HFS_STATUS_SUCCESS;
    HFS_LEAVE(hfsService_REQ_MDS_READDIR);
    return status;
}

//REQ_MDS_CREATE_DIRENT
//RESP_MDS_CREATE_DIRENT
HFS_STATUS
hfsService_REQ_MDS_CREATE_DIRENT(PHFS_QUEUE_ITEM pQueueItem)
{
    HFS_STATUS status=HFS_STATUS_SUCCESS;
    PHFS_SERVER_REQ pServReq=NULL;
    PHFS_SERVER_RESP pServResp=NULL;
    HFS_ENTRY(hfsService_REQ_MDS_CREATE_DIRENT);

    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);
    pServResp = (PHFS_SERVER_RESP) (pQueueItem + 1);

    //-! Begin Command Processing !-//
    //- Just build up the response packet and sent it out -//
    pServResp->hdr.status =
        dbAddDirent(
                    &getHFSServer()->kernel.hfsDbCtx,
                    &pServReq->req.reqMDSCreateDirent.hfsDirent
                    );
    pServResp->resp.respMDSCreateDirent.cookie = 0xDEADBEAF;
    //-! End Command Processing !-//
    status = HFS_STATUS_SUCCESS;
    HFS_LEAVE(hfsService_REQ_MDS_CREATE_DIRENT);
    return status;
}

//REQ_MDS_DELETE_DIRENT
//RESP_MDS_DELETE_DIRENT
HFS_STATUS
hfsService_REQ_MDS_DELETE_DIRENT(PHFS_QUEUE_ITEM pQueueItem)
{
    HFS_STATUS status=HFS_STATUS_SUCCESS;
    PHFS_SERVER_REQ pServReq=NULL;
    PHFS_SERVER_RESP pServResp=NULL;
    HFS_ENTRY(hfsService_REQ_MDS_DELETE_DIRENT);

    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);
    HFS_UNUSED(pServReq);
    pServResp = (PHFS_SERVER_RESP) (pQueueItem + 1);

    //- Just build up the response packet and sent it out -//
    pServResp->hdr.status = HFS_PROTO_STATUS_NOT_IMPLEMENTED;

    //-! Begin Command Processing !-//

    //-! End Command Processing !-//
    status = HFS_STATUS_SUCCESS;
    HFS_LEAVE(hfsService_REQ_MDS_DELETE_DIRENT);
    return status;
}

//REQ_MDS_LOOK_UP
//RESP_MDS_LOOK_UP
HFS_STATUS
hfsService_REQ_MDS_LOOK_UP(PHFS_QUEUE_ITEM pQueueItem)
{
    HFS_STATUS status=HFS_STATUS_SUCCESS;
    PHFS_SERVER_REQ pServReq=NULL;
    PHFS_SERVER_RESP pServResp=NULL;
    HFS_ENTRY(hfsService_REQ_MDS_LOOK_UP);

    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);
    pServResp = (PHFS_SERVER_RESP) (pQueueItem + 1);

    //- Just build up the response packet and sent it out -//
    pServResp->hdr.status = HFS_PROTO_STATUS_NOT_IMPLEMENTED;
    //-! Begin Command Processing !-//
    pServResp->hdr.status = dbLookUp(
                                     &getHFSServer()->kernel.hfsDbCtx,
                                     pServReq->req.reqMDSLookUp.parentMetaDataHandle,
                                     (char *)pServReq->req.reqMDSLookUp.dname,
                                     &pServResp->resp.respMDSLookUp.selfHandle,
                                     &pServResp->resp.respMDSLookUp.serverId);
    //-! End Command Processing !-//
    status = HFS_STATUS_SUCCESS;
    HFS_LEAVE(hfsService_REQ_MDS_LOOK_UP);
    return status;
}

//REQ_MDS_GET_ATTR
//RESP_MDS_GET_ATTR
HFS_STATUS
hfsService_REQ_MDS_GET_ATTR(PHFS_QUEUE_ITEM pQueueItem)
{
    HFS_STATUS status=HFS_STATUS_SUCCESS;
    PHFS_SERVER_REQ pServReq=NULL;
    PHFS_SERVER_RESP pServResp=NULL;
    HFS_ENTRY(hfsService_REQ_MDS_GET_ATTR);

    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);
    pServResp = (PHFS_SERVER_RESP) (pQueueItem + 1);

    //- Just build up the response packet and sent it out -//
    //-! Begin Command Processing !-//
    pServResp->hdr.status = dbGetAttr(
                                      &getHFSServer()->kernel.hfsDbCtx,
                                      pServReq->req.reqMDSGetAttr.selfMetaDataHandle,
                                      &pServResp->resp.respMDSGetAttr.attr);
    //-! End Command Processing !-//
    status = HFS_STATUS_SUCCESS;
    HFS_LEAVE(hfsService_REQ_MDS_GET_ATTR);
    return status;
}

//REQ_MDS_SET_ATTR
//RESP_MDS_SET_ATTR
HFS_STATUS
hfsService_REQ_MDS_SET_ATTR(PHFS_QUEUE_ITEM pQueueItem)
{
    HFS_STATUS status=HFS_STATUS_SUCCESS;
    PHFS_SERVER_REQ pServReq=NULL;
    PHFS_SERVER_RESP pServResp=NULL;
    HFS_ENTRY(hfsService_REQ_MDS_SET_ATTR);

    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);
    pServResp = (PHFS_SERVER_RESP) (pQueueItem + 1);

    //- Just build up the response packet and sent it out -//
    //-! Begin Command Processing !-//
    pServResp->hdr.status = dbSetAttr(
                                      &getHFSServer()->kernel.hfsDbCtx,
                                      &pServReq->req.reqMDSSetAttr.attr,
                                      pServReq->req.reqMDSSetAttr.attrMask);
    pServResp->resp.respMDSSetAttr.cookie = 0xDEADBEAF;
    //-! End Command Processing !-//
    status = HFS_STATUS_SUCCESS;
    HFS_LEAVE(hfsService_REQ_MDS_SET_ATTR);
    return status;
}

//REQ_MDS_GET_EXTENT_SIZE
//RESP_MDS_GET_EXTENT_SIZE
HFS_STATUS
hfsService_REQ_MDS_GET_EXTENT_SIZE(PHFS_QUEUE_ITEM pQueueItem)
{
    HFS_STATUS status=HFS_STATUS_SUCCESS;
    PHFS_SERVER_REQ pServReq=NULL;
    PHFS_SERVER_RESP pServResp=NULL;
    __u32 extentSize = 0;
    HFS_ENTRY(hfsService_REQ_MDS_GET_EXTENT_SIZE);

    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);
    HFS_UNUSED(pServReq);
    pServResp = (PHFS_SERVER_RESP) (pQueueItem + 1);

    //- Just build up the response packet and sent it out -//
    pServResp->hdr.status = HFS_PROTO_STATUS_SUCCESS;
    //-! Begin Command Processing !-//
    pServResp->hdr.status =
        dbGetExtentSize(
                        &getHFSServer()->kernel.hfsDbCtx,
                        &extentSize);
    pServResp->resp.respMDSGetExtentSize.extent_size = extentSize;
    //-! End Command Processing !-//
    status = HFS_STATUS_SUCCESS;
    HFS_LEAVE(hfsService_REQ_MDS_GET_EXTENT_SIZE);
    return status;
}



//REQ_MDS_GET_CONFIG
//RESP_MDS_GET_CONFIG
HFS_STATUS
hfsService_REQ_MDS_GET_CONFIG(PHFS_QUEUE_ITEM pQueueItem)
{
    HFS_STATUS status=HFS_STATUS_SUCCESS;
    PHFS_SERVER_REQ pServReq=NULL;
    PHFS_SERVER_RESP pServResp=NULL;
    __u32 copiedSrvrents;

    HFS_ENTRY(hfsService_REQ_MDS_GET_CONFIG);

    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);
    pServResp = (PHFS_SERVER_RESP) (pQueueItem + 1);

    //-! Begin Command Processing !-//
    //- Just build up the response packet and sent it out -//
    pServResp->hdr.status =
        dbGetSrvrListing(
                         &getHFSServer()->kernel.hfsDbCtx,
                         (char *)pServResp + sizeof(HFS_PROTO_HEADER) + sizeof(RESP_MDS_GET_CONFIG),
                         getHFSFSCtx()->stripeSize,
                         pServReq->req.reqMDSGetConfig.srvrentOffset,
                         &copiedSrvrents
                         );
    pServResp->resp.respMDSGetConfig.srvrentCount = copiedSrvrents;
    pServResp->hdr.pduExtentSize  = copiedSrvrents * sizeof(HFS_SERVER_INFO);
    //-! End Command Processing !-//
    status = HFS_STATUS_SUCCESS;
    HFS_LEAVE(hfsService_REQ_MDS_GET_CONFIG);
    return status;
}



//REQ_MDS_GET_EXTENT_SIZE
//RESP_MDS_GET_EXTENT_SIZE
HFS_STATUS
hfsService_REQ_MDS_LOGIN_PHASE1(PHFS_QUEUE_ITEM pQueueItem)
{
    HFS_STATUS status=HFS_STATUS_SUCCESS;
    PHFS_SERVER_REQ pServReq=NULL;
    PHFS_SERVER_RESP pServResp=NULL;
    HFS_CLIENT_CONN_CTX *pClientConnectionCtx;

    HFS_ENTRY(hfsService_REQ_MDS_LOGIN_PHASE1);

    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);
    pServResp = (PHFS_SERVER_RESP) (pQueueItem + 1);
    pClientConnectionCtx = getHFSServer()->kernel.pClientConnCtx + pQueueItem->clientIdx;

    //- Just build up the response packet and sent it out -//
    pServResp->hdr.status = HFS_PROTO_STATUS_SUCCESS;
    //-! Begin Command Processing !-//
    memcpy(pClientConnectionCtx->userName, pServReq->req.reqMDSLoginPhase1.user, HFS_MAX_USER_NAME);
    pServResp->hdr.status =
        dbGetEncryptedNonce(
                            &getHFSServer()->kernel.hfsDbCtx,
                            pServReq->req.reqMDSLoginPhase1.user,
                            &pClientConnectionCtx->randomNonce,
                            &pServResp->resp.respMDSLoginPhase1.u.encryptedNonce);
    //-! End Command Processing !-//
    status = HFS_STATUS_SUCCESS;
    HFS_LEAVE(hfsService_REQ_MDS_LOGIN_PHASE1);
    return status;
}

//REQ_MDS_GET_EXTENT_SIZE
//RESP_MDS_GET_EXTENT_SIZE
HFS_STATUS
hfsService_REQ_MDS_LOGIN_PHASE2(PHFS_QUEUE_ITEM pQueueItem)
{
    HFS_STATUS status=HFS_STATUS_SUCCESS;
    PHFS_SERVER_REQ pServReq=NULL;
    PHFS_SERVER_RESP pServResp=NULL;
    HFS_CLIENT_CONN_CTX *pClientConnectionCtx;

    HFS_ENTRY(hfsService_REQ_MDS_LOGIN_PHASE2);

    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);
    pServResp = (PHFS_SERVER_RESP) (pQueueItem + 1);
    pClientConnectionCtx = getHFSServer()->kernel.pClientConnCtx + pQueueItem->clientIdx;

    //- Just build up the response packet and sent it out -//
    pServResp->hdr.status = HFS_PROTO_AUTHENTICATION_FAILURE;
    //-! Begin Command Processing !-//

    if (strncmp(pClientConnectionCtx->userName, pServReq->req.reqMDSLoginPhase2.user, HFS_MAX_USER_NAME)) {
        HFS_LOG_ERROR("Possible attack, usernames in 2 phases don't match");
        pServResp->hdr.status = HFS_PROTO_AUTHENTICATION_FAILURE;
        pClientConnectionCtx->authenticated = 0;
        return HFS_STATUS_SUCCESS;
    }

    //- Ok so now we expect the client to have successful decrypted our random
    //- nonce
    if (pServReq->req.reqMDSLoginPhase2.u.dencryptedNonce == pClientConnectionCtx->randomNonce) {
        pServResp->hdr.status = HFS_PROTO_STATUS_SUCCESS;
        pClientConnectionCtx->authenticated = 1;
    }else {
        HFS_LOG_ERROR("Authentication failed for user name %s", pServReq->req.reqMDSLoginPhase2.user);
        pServResp->hdr.status = HFS_PROTO_AUTHENTICATION_FAILURE;
        pClientConnectionCtx->authenticated = 0;
    }

    //-! End Command Processing !-//
    status = HFS_STATUS_SUCCESS;
    HFS_LEAVE(hfsService_REQ_MDS_LOGIN_PHASE2);
    return status;
}






// Dispatch for the MDS commands //
#define SERVICE_REQUEST(REQ)                            \
    case CMD_##REQ:                                     \
    status = hfsService_##REQ(pQueueItem);              \
    if (!HFS_SUCCESS(status)){                          \
        HFS_LOG_ERROR("\nsServicing " #REQ " failed");  \
    }                                                   \
    break;


HFS_STATUS
mdsPendingCommandProcessing(PHFS_QUEUE_ITEM pQueueItem,
                            HFS_STATUS cmd_status)
{
    HFS_STATUS status;
    PHFS_SERVER_REQ pServReq;
    HFS_ENTRY(mdsPendingCommandProcessing);
    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);

    // Service the command //
    switch(pServReq->hdr.command) {
        SERVICE_REQUEST(REQ_MDS_PING)
            SERVICE_REQUEST(REQ_MDS_ALLOC_HANDLE)
            SERVICE_REQUEST(REQ_MDS_FREE_HANDLE)
            SERVICE_REQUEST(REQ_MDS_READDIR)
            SERVICE_REQUEST(REQ_MDS_CREATE_DIRENT)
            SERVICE_REQUEST(REQ_MDS_DELETE_DIRENT)
            SERVICE_REQUEST(REQ_MDS_LOOK_UP)
            SERVICE_REQUEST(REQ_MDS_GET_ATTR)
            SERVICE_REQUEST(REQ_MDS_SET_ATTR)
            SERVICE_REQUEST(REQ_MDS_GET_EXTENT_SIZE)
            SERVICE_REQUEST(REQ_MDS_GET_CONFIG)
            SERVICE_REQUEST(REQ_MDS_LOGIN_PHASE1);
        SERVICE_REQUEST(REQ_MDS_LOGIN_PHASE2);
    default:
        HFS_LOG_ERROR("UNKOWN COMMAND");
    }

    //- Push the response out -//
    status = hfsQueueItem(getHFSServer()->kernel.pQOutBound, pQueueItem);
    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR(" outBoundProcessing error.\n");
        return status;
    }

    HFS_LEAVE(mdsPendingCommandProcessing);
    return status;
}
