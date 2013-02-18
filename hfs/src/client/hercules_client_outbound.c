/*
 * hercules_client_outbound.c
 *
 * Handle outbound packets from this client.
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
#include <hercules_client.h>

// Well this is the end of the road for the packet here don't return errors --//
// here is where we bound our rationality about failure handling  --//

// -Client socket transmission lock is held and the request is still on the list-//
// -We always the get the encoded headers and request !   -//
HFS_STATUS
clientOutBoundFailureHandler(PHFS_QUEUE_ITEM pQueueItem)
{
    PHFS_SERVER_REQ pServReq=NULL;
    HFS_STATUS  status;

    HFS_ENTRY();
    HFS_LOG_ERROR("Send failed %d",errno);

    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);

    // Remove this thinggy from the socket pending list Remember this function 
    // is called with the transmission lock held //
    getHFSClient()->kernel.pServerConnCtx[pQueueItem->clientIdx].PendingCommands--;
    list_del_init(&pQueueItem->listHead);

    // Notify the poor client so that he handles it
    // Undo the network order swapping done before sending failed //
    hfsDecodeProtoHeader(&pServReq->hdr);
    status = hfsDecodeCommandBuffer((char *)((&pServReq->hdr) + 1),pServReq->hdr.command);
    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR("I'm seeing a unknown command at the end please check coding logic2");
    }

    //- just to make sure we set in the right error code -//
    pServReq->hdr.status = HFS_TRANSMISSION_FAILED;

    //- Now let the client see the failed packet -//
    hfsSignalQItem(pQueueItem);
    return pServReq->hdr.status;
}


HFS_STATUS
clientOutBoundProcessing(PHFS_QUEUE_ITEM pQueueItem,
                         HFS_STATUS status_q)
{
    HFS_STATUS  status = HFS_STATUS_SUCCESS;
    int   sendSize = 0;
    int   sendBytes = 0;
    int   offset =0 ;
    int   commandSize=0;
    PHFS_SERVER_REQ pServReq=NULL;

    HFS_ENTRY();
    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);
    pServReq->hdr.status = HFS_TRANSMISSION_FAILED;

    //________PREPrOcessIng______________________________//
    // Cookie for fast search when the packet comes back //
    pServReq->hdr.cookies[COOKIE_QITEM_IDX] =
        (typeof(pServReq->hdr.cookies[COOKIE_QITEM_IDX])) pQueueItem;
    assert(sizeof(pServReq->hdr.cookies[COOKIE_QITEM_IDX])
           >= sizeof(pQueueItem));

    //________SizeCalculations__________________________ //
    sendSize = sizeof(pServReq->hdr);
    commandSize = hfsGetCommandSize(&pServReq->hdr);
    if (-1 == commandSize) {
        HFS_LOG_ERROR("I'm seeing a unknown command at the end please check coding logic");
        hfsSignalQItem(pQueueItem);
        return HFS_STATUS_SUCCESS;
    }else {
        sendSize += commandSize;
    }

    if (pduHasPayLoad(pServReq->hdr.command))
        sendSize += pServReq->hdr.pduExtentSize;

    //________Network order Encoding_____________________ //
    status = hfsEncodeCommandBuffer((char *)((&pServReq->hdr) + 1),pServReq->hdr.command);
    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR("I'm seeing a unknown command at the end please check coding logic2");
        hfsSignalQItem(pQueueItem);
        return HFS_STATUS_SUCCESS;
    }
    hfsEncodeProtoHeader(&pServReq->hdr);


    //________PacketNowReadyToFly__________________________//


    //_-_-_-_-_-CritSectionBegin -_-_-_-_-_-_-_-_-_-_-_-_-_//
    hfsSemWait(&getHFSClient()->kernel.clientOutBoundCritSections); // Lock Hierarchy is as its coded //
    hfsMutexLock(&getHFSClient()->kernel.pServerConnCtx[pQueueItem->clientIdx].mutexPendingCommandslst);
    getHFSClient()->kernel.pServerConnCtx[pQueueItem->clientIdx].PendingCommands++;
    list_add_tail(&pQueueItem->listHead,&getHFSClient()->kernel.pServerConnCtx[pQueueItem->clientIdx].lstAnchorPendingCommands);


    //________SendTheStuffOut_____________________________//
    sendBytes = 0;
    offset = 0;
    do {
        sendBytes = send(getHFSClient()->kernel.pPollFds[pQueueItem->clientIdx].fd,
                         (char *)pServReq + offset,
                         sendSize,
                         0);
        if (-1 == sendBytes) {
            // Stop sending the packet and drop the connection
            status = HFS_INTERNAL_ERROR;
            goto packetSentFailed;
        }
        sendSize -= sendBytes;
        offset += sendBytes;
    }while(sendSize>0);

    HFS_LOG_INFO("Packet sent out successfully");
    status = HFS_STATUS_SUCCESS;
    goto packetSentSuccess;



    //_______TailsForThisfunction___________________________//
 packetSentFailed:
    status = clientOutBoundFailureHandler(pQueueItem);
    // free the socket now we know that the retries have also failed //
    if (!HFS_SUCCESS(status))
        DEACTIVATE_CLIENT_SIDE_SOCKET(getHFSClient(),pQueueItem->clientIdx);

 packetSentSuccess:
    //________________ Nothing to do here_________________//

    goto releaseCritSection; //__CompilerWarningKludge__//
 releaseCritSection:
    hfsMutexUnlock(&getHFSClient()->kernel.pServerConnCtx[pQueueItem->clientIdx].mutexPendingCommandslst);
    hfsSempost(&getHFSClient()->kernel.clientOutBoundCritSections);
    //_-_-_-_-_-CritSectionEnd-_-_-_-_-_-_-_-_-_-_-_-_-_-_//
    HFS_LEAVE();
    return status;
}
