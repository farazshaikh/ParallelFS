/*
 * hercules_inbound.c
 *
 * Hercules server outbound command dispatch. There are a very few
 * commands initiated by the server. One of them is asking clients to
 * update the fs config.
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

HFS_STATUS
outBoundProcessing(PHFS_QUEUE_ITEM pQueueItem,
                   HFS_STATUS cmd_status)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    int sendSize = 0;
    int sendBytes = 0;
    int offset =0 ;

    int commandSize=0;
    PHFS_SERVER_RESP pServResp=NULL;

    HFS_ENTRY(outBoundProcessing);
    pServResp = (PHFS_SERVER_RESP) (pQueueItem + 1);

    // Update Servers Last Action MASK //
    hfsServerMarkLastOperation(getHFSServer(), pServResp->hdr.command);

    // Flip the bit to indicate and response //
    pServResp->hdr.command = toggleReqResp(pServResp->hdr.command);

    // Calculate the length that needs to go out //
    sendSize = sizeof(pServResp->hdr);
    commandSize = hfsGetCommandSize(&pServResp->hdr);
    if (-1 == commandSize) {
        HFS_LOG_ERROR("I'm seeing a unknown command at the end please check coding logic");
        goto dropconnection;
    }else {
        sendSize += commandSize;
    }

    // Does the new command has an extent //
    if (pduHasPayLoad(pServResp->hdr.command)) {
        sendSize += pServResp->hdr.pduExtentSize;
    }

    // Encode the things now //
    status = hfsEncodeCommandBuffer((char *)((&pServResp->hdr) + 1), pServResp->hdr.command);
    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR("I'm seeing a unknown command at the end please check coding logic2");
        goto dropconnection;
    }
    hfsEncodeProtoHeader(&pServResp->hdr);

    // Send the stuff out //
    sendBytes = 0;
    offset = 0;
    do {
        sendBytes = send(getHFSServer()->kernel.pPollFds[pQueueItem->clientIdx].fd, 
                         (char *)pServResp + offset, 
                         sendSize, 
                         0);
        if (-1 == sendBytes) {
            // Stop sending the packet and drop the connection
            status = HFS_INTERNAL_ERROR;
            goto dropconnection;
        }
        sendSize -= sendBytes;
        offset += sendBytes;
    } while (sendSize>0);

    HFS_LOG_INFO("Packet sent out successfully");
    status = HFS_STATUS_SUCCESS;
    goto doneOk;



    // tails //
 dropconnection:

    getHFSServer()->kernel.pClientConnCtx[pQueueItem->clientIdx].PendingCommands--;
    HFS_LOG_ERROR("Send failed %d", errno);
    // free the socket //
    DEACTIVATE_SOCKET(getHFSServer(), pQueueItem->clientIdx);
    RELEASE_SOCKET_IF_NO_PENDING(getHFSServer(), pQueueItem->clientIdx);
    // free the data buff
    hfsFreeProtoBuff(pQueueItem);
    return HFS_CONNECTION_DROPPED;

 doneOk:
    getHFSServer()->kernel.pClientConnCtx[pQueueItem->clientIdx].PendingCommands--;
    // free the data buff
    hfsFreeProtoBuff(pQueueItem);
    HFS_LEAVE(outBoundProcessing);
    return HFS_STATUS_SUCCESS;
}
