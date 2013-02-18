/*
 * hercules_ds_cmds.c
 *
 *      Hercules data server command handling
 *
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


// Read
// Write
//
// GetExtensize
// GetFreeHandle
// FreeHandle
HFS_STATUS
hfsProcessDSCMD(PHFS_QUEUE_ITEM pQueueItem)
{
    HFS_SERVER_REQ    *pServReq  = NULL;
    HFS_SERVER_RESP   *pServResp = NULL;
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    HFS_STATUS retStatus = HFS_STATUS_SUCCESS;
    PHFS_FS_CTX pFsCtx = NULL;
    __u32 stripe_size;

    do {
        //-- Get the stripe size --//
        pFsCtx = getHFSFSCtx();
        if (pFsCtx == NULL) {
            HFS_LOG_ERROR("Fatal error: FS Context not found."
                          "Dropping Packet: %u.\n", pServReq->hdr.command);
            retStatus = HFS_INTERNAL_ERROR;
            break;
        }

        stripe_size = pFsCtx->stripeSize;

        // Get the proto - header  //
        pServReq  = (HFS_SERVER_REQ  *) (pQueueItem + 1);
        pServResp = (HFS_SERVER_RESP *) (pQueueItem + 1);

        // Get the request command //
        // Process command //
        switch (pServReq->hdr.command) {
        case CMD_REQ_DS_PING:
            pServResp->resp.respDSPing.cookie     = getHFSServer()->kernel.lastOperation;
            getHFSServer()->kernel.lastOperation -= getHFSServer()->kernel.lastOperation;
            status                                = HFS_STATUS_SUCCESS;
            break;
        case CMD_REQ_DS_ALLOC_HANDLE:
            //-- TODO: Check for valid server id --//
            status = hfsMutexLock(&getHFSServer()->kernel.serverMetaDataUpdateLock);
            if (!HFS_SUCCESS(status)) {
                HFS_LOG_ERROR("Failed to acquire server metadata update lock");
                break;
            }

            status = srvAllocDataHandle(&pServReq->req.reqDSAllocHandle.dataHandle);

            status = hfsMutexUnlock(&getHFSServer()->kernel.serverMetaDataUpdateLock);
            if (!HFS_SUCCESS(status)) {
                HFS_LOG_ERROR("Failed to release metadata update lock bailing out");
                exit(0);
            }

            break;
        case CMD_REQ_DS_FREE_HANDLE:
            //-- TODO: Check for valid server id --//
            status = srvFreeDataHandle(pServReq->req.reqDSFreeHandle.dataHandle);
            break;
        case CMD_REQ_DS_READ_STRIPE:
            //-- TODO: Check for valid server id --//
            status = srvReadStripe(
                                   pServReq->req.reqDSReadStripe.dataHandle,  stripe_size,
                                   pServReq->req.reqDSReadStripe.stripeNum,
                                   pServReq->req.reqDSReadStripe.stripeCnt,
                                   &pServReq->req.reqDSReadStripe.readSize,
                                   ((char *)&pServReq->req) + sizeof(pServReq->req.reqDSReadStripe));

            if (HFS_SUCCESS(status)) {
                pServResp->hdr.pduExtentSize = pServReq->req.reqDSReadStripe.readSize;
            } else {
                pServResp->hdr.pduExtentSize = 0;
            }

            break;
        case CMD_REQ_DS_WRITE_STRIPE:
            //-- TODO: Check for valid server id --//
            status = srvWriteStripe(
                                    pServReq->req.reqDSWriteStripe.dataHandle,  stripe_size,
                                    pServReq->req.reqDSWriteStripe.stripeNum,
                                    pServReq->req.reqDSWriteStripe.stripeCnt,
                                    pServReq->req.reqDSWriteStripe.writeInStripeOffset,
                                    pServReq->req.reqDSWriteStripe.writeSize,
                                    ((char *)&pServReq->req) + sizeof(pServReq->req.reqDSWriteStripe));
            break;
        case CMD_REQ_DS_GET_LENGTH:
            //-- TODO: Check for valid server id --//
            status = srvGetExtentSize(
                                      pServReq->req.reqDSGetLength.dataHandle,
                                      &pServReq->req.reqDSGetLength.extentSize);
            break;
        default:
            HFS_LOG_ERROR("DS command %u not implemented...\n",  pServReq->hdr.command);
            status = HFS_PROTO_STATUS_NOT_IMPLEMENTED;
            break;
        }

        // build the response //
        if (status != HFS_STATUS_SUCCESS && status != HFS_INVALID_HANDLE_ERROR &&
            status != HFS_INVALID_STRIPE_ERROR) {
            status = HFS_PROTO_STATUS_FATAL_ERROR;
        }
        pServReq->hdr.status = status;
        retStatus = HFS_STATUS_SUCCESS; // if packet has to go online
    } while (0);

    return retStatus;
}

HFS_STATUS
dsPendingCommandProcessing(PHFS_QUEUE_ITEM pQueueItem,
                           HFS_STATUS      cmd_status)
{
    HFS_STATUS status;
    HFS_ENTRY(dsPendingCommandProcessing);

    do {
        status = hfsProcessDSCMD (pQueueItem);
        if (!HFS_SUCCESS(status)) {
            HFS_LOG_ERROR("Could not process DS command.\n");
        }


        status = hfsQueueItem(getHFSServer()->kernel.pQOutBound, pQueueItem);
        if (!HFS_SUCCESS(status)) {
            HFS_LOG_ERROR(" outBoundProcessing error.\n");
            break;
        }

        status = HFS_STATUS_SUCCESS;
    } while(0);

    HFS_LEAVE(dsPendingCommandProcessing);
    return status;
}
