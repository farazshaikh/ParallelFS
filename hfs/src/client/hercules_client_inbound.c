/*
 * hercules_client_inbound.c
 *
 *      Process inbound requests to this client.
 *      and data servers to provide filesystem services
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
#include <hercules_client.h>

// Pumps bytes into the connection state machine //
// REQ_COM_STAGE_WAITING -> REQ_COM_STAGE_HEADER -> REQ_COM_STAGE_COMMAND -> REQ_COM_STAGE_COMMAND
//   ^    ^                                                      ^                   ^
//   |    |______________________________________________________|                   |
//   |_______________________________________________________________________________|
//

#define RESET_CONNECTION_STATE_MACHINE(pClientConnCtx) do {             \
        (pClientConnCtx)->inStage           = RESP_COM_STAGE_WAITING;   \
        (pClientConnCtx)->bytesReadOff      = 0;                        \
        (pClientConnCtx)->bounceHdr.command = 0;                        \
        (pClientConnCtx)->pQueueItem        = 0;                        \
        (pClientConnCtx)->commandSize       = 0;                        \
    }while(0);

int
IsCommandUnsolicited(int command)
{
    return (CMD_RESP_MDS_FS_CONF_CHANGED == command);
}

int
IsValidRespHeader(HFS_PROTO_HEADER *p)
{
    return 1;
}


HFS_STATUS
hfsClientPumpBytesToConnSM(char *buffer,
                           int  size,
                           int  clientfdIdx)
{
    int      i;
    HFS_STATUS    status=HFS_STATUS_SUCCESS;
    PHFS_SERVER_CONN_CTX pServerConnCtx;

    pServerConnCtx = &getHFSClient()->kernel.pServerConnCtx[clientfdIdx];

    for (i = 0 ; i < size ; i++) {
        // Adjust the current stage //
        switch(pServerConnCtx->inStage) {
        case RESP_COM_STAGE_WAITING:
            if (pServerConnCtx->bytesReadOff) {
                HFS_LOG_ERROR("Command stream ClientIDX out of sync");
                return HFS_PROTOCOL_ERROR;
            }
            // Transit to next stage //
            pServerConnCtx->inStage++; // Null state go ahead //

        case RESP_COM_STAGE_HEADER:
            if (pServerConnCtx->bytesReadOff < sizeof(HFS_PROTO_HEADER)) {
                ((char *)(&pServerConnCtx->bounceHdr))[pServerConnCtx->bytesReadOff] = buffer[i];
                pServerConnCtx->bytesReadOff++;
            }

            if (pServerConnCtx->bytesReadOff != sizeof(HFS_PROTO_HEADER))
                continue;

            // TRANSIT TO NEXT STAGE //
            hfsDecodeProtoHeader(&pServerConnCtx->bounceHdr);

            // Sanity //
            if (!IsValidRespHeader(&pServerConnCtx->bounceHdr)) {
                HFS_LOG_ERROR("Decoded wrong header");
                return HFS_PROTOCOL_ERROR;
            }

            if (pServerConnCtx->pQueueItem) {
                HFS_LOG_INFO("Possible memory leak");
            }

            // Allocation for next stage //
            if (IsCommandUnsolicited(pServerConnCtx->bounceHdr.command)) {
                pServerConnCtx->pQueueItem = hfsAllocateProtoBuff(&pServerConnCtx->bounceHdr,getHFSFSCtx()->stripeSize);
                if (!pServerConnCtx->pQueueItem) {
                    HFS_LOG_ERROR("Cannot allocate the memory for unsolicited command");
                    exit(0);
                }
            }else {
                pServerConnCtx->pQueueItem = (PHFS_QUEUE_ITEM) pServerConnCtx->bounceHdr.cookies[COOKIE_QITEM_IDX];
            }

            memcpy(pServerConnCtx->pQueueItem + 1,&pServerConnCtx->bounceHdr,sizeof(pServerConnCtx->bounceHdr));

            pServerConnCtx->commandSize = hfsGetCommandSize(&pServerConnCtx->bounceHdr);
            if (-1 == pServerConnCtx->commandSize) {
                HFS_LOG_ERROR("ClientIDX out of sync2");
                return HFS_PROTOCOL_ERROR;
            }

            pServerConnCtx->inStage++;
            continue;

        case RESP_COM_STAGE_COMMAND:
            if (pServerConnCtx->bytesReadOff < sizeof(HFS_PROTO_HEADER)||
                !pServerConnCtx->commandSize || -1==pServerConnCtx->commandSize) {
                HFS_LOG_ERROR("HHGG Press panic");
                return HFS_PROTOCOL_ERROR;
            }

            if (pServerConnCtx->bytesReadOff <
                sizeof(HFS_PROTO_HEADER) + pServerConnCtx->commandSize) {
                ((char *)(pServerConnCtx->pQueueItem + 1))[pServerConnCtx->bytesReadOff] = buffer[i];
                pServerConnCtx->bytesReadOff++;
            }

            if (pServerConnCtx->bytesReadOff !=
                (sizeof(HFS_PROTO_HEADER) + pServerConnCtx->commandSize)) {
                continue;
            }

            // TRANSIT TO NEXT STAGE //
            status = hfsDecodeCommandBuffer((char *)(pServerConnCtx->pQueueItem + 1) +
                                            sizeof(HFS_PROTO_HEADER),
                                            pServerConnCtx->bounceHdr.command);
            if (!HFS_SUCCESS(status)) {
                HFS_LOG_ERROR("cannot decode command buffer");
                return HFS_PROTOCOL_ERROR;
            }

            // If we have payload go to state extent else attach and reset //
            if (pduHasPayLoad(pServerConnCtx->bounceHdr.command) && pServerConnCtx->bounceHdr.pduExtentSize) {
                pServerConnCtx->inStage++;
                continue;
            }else {
                pServerConnCtx->pQueueItem->clientIdx = clientfdIdx;

                if (IsCommandUnsolicited(pServerConnCtx->bounceHdr.command)){
                    hfsUnsolicitedCommand(pServerConnCtx->pQueueItem);
                }else {
                    // DeQueue the item & reset state machine.
                    hfsMutexLock(&pServerConnCtx->mutexPendingCommandslst);
                    pServerConnCtx->PendingCommands--;
                    list_del_init(&pServerConnCtx->pQueueItem->listHead);
                    hfsMutexUnlock(&pServerConnCtx->mutexPendingCommandslst);
                    hfsSignalQItem(pServerConnCtx->pQueueItem);
                }

                RESET_CONNECTION_STATE_MACHINE(pServerConnCtx);
                continue;
            }

        case RESP_COMM_STAGE_EXTENT:
            if (pServerConnCtx->bytesReadOff < sizeof(HFS_PROTO_HEADER) ||
                !pServerConnCtx->commandSize || -1==pServerConnCtx->commandSize) {
                HFS_LOG_ERROR("HHGG Press panic2");
                return HFS_PROTOCOL_ERROR;
            }



            if (pServerConnCtx->bytesReadOff < 
                sizeof(HFS_PROTO_HEADER) + 
                pServerConnCtx->commandSize + 
                pServerConnCtx->bounceHdr.pduExtentSize) {
                ((char *)(pServerConnCtx->pQueueItem + 1))[pServerConnCtx->bytesReadOff] = buffer[i];
                pServerConnCtx->bytesReadOff++;
            }

            if (pServerConnCtx->bytesReadOff !=
                sizeof(HFS_PROTO_HEADER) +
                pServerConnCtx->commandSize +
                pServerConnCtx->bounceHdr.pduExtentSize) {
                continue;
            }


            // Queue the item & reset state machine.
            pServerConnCtx->pQueueItem->clientIdx = clientfdIdx;

            if (IsCommandUnsolicited(pServerConnCtx->bounceHdr.command)) {
                hfsUnsolicitedCommand(pServerConnCtx->pQueueItem);
            } else {
                // DeQueue the item & reset state machine.
                hfsMutexLock(&pServerConnCtx->mutexPendingCommandslst);
                pServerConnCtx->PendingCommands--;
                list_del_init(&pServerConnCtx->pQueueItem->listHead);
                hfsMutexUnlock(&pServerConnCtx->mutexPendingCommandslst);
                hfsSignalQItem(pServerConnCtx->pQueueItem);
            }

            RESET_CONNECTION_STATE_MACHINE(pServerConnCtx);
            continue;

        default:
            HFS_LOG_ERROR("Invalid state for Protocol Reader State machine");
        } // End switch
    }// End for 

    status = HFS_STATUS_SUCCESS;
    return HFS_STATUS_SUCCESS;
}



HFS_STATUS
hfsClientSocketProcessCommands(PHERCULES_CLIENT pHerculesClient,
                               int    clientfdIdx)
{
    HFS_STATUS   status=HFS_STATUS_SUCCESS;
    int     rc;
    HFS_PROTO_HEADER header;

    // Read Protocol //
    int totalBytesRead = 0;
    rc = recv(pHerculesClient->kernel.pPollFds[clientfdIdx].fd,
              &header,
              sizeof(header),
              0);
    if (rc < 0) {
        if (errno != EWOULDBLOCK) {
            goto drop_connection;
        }
        //This is a special case here where socket says we would block//
        return HFS_STATUS_SUCCESS;
    }
    totalBytesRead += rc;

    if (0 == totalBytesRead) {
        goto client_closed;
    }

    // Pump it //
    status = hfsClientPumpBytesToConnSM((char *)&header,
                                        rc,
                                        clientfdIdx);
    if (!HFS_SUCCESS(status)) {
        goto client_closed;
    }

    status = HFS_STATUS_SUCCESS;
    goto done;

 drop_connection:
    HFS_LOG_INFO("OOPS FSHITGRR we are dropping the server now");
    DEACTIVATE_CLIENT_SIDE_SOCKET(pHerculesClient,clientfdIdx);
    RELEASE_CLIENT_SOCKET_IF_NO_PENDING(pHerculesClient,clientfdIdx);
    status = HFS_CONNECTION_DROPPED;
    goto done;

 client_closed:
    HFS_LOG_INFO("Client Closed the connection");
    DEACTIVATE_CLIENT_SIDE_SOCKET(pHerculesClient,clientfdIdx);
    RELEASE_CLIENT_SOCKET_IF_NO_PENDING(pHerculesClient,clientfdIdx);
    status = HFS_CLIENT_DISCONNECTED;

 done:
    return status;
}


void *
hfsClientInboundLoop(void *pVVHerculesClient)
{
    HFS_STATUS status=HFS_STATUS_SUCCESS;
    int numberOfFds;
    int i;
    PHERCULES_CLIENT pHerculesClient;

    pHerculesClient = (PHERCULES_CLIENT)pVVHerculesClient;
    HFS_ENTRY();
    do {
        //- Wait for all of the the clients sockets we have -//
    timed_out:
        numberOfFds = poll(pHerculesClient->kernel.pPollFds,
                           pHerculesClient->config.maxServersAllowed,
                           1); // Sadly we have to do this to support dynamic
        // client update to fs config change 
        if (!numberOfFds) {
            goto timed_out;
        }

        //- Number of fds -//
        if (numberOfFds < 0) {
            if (errno == EINTR) {
                continue;
            }

            HFS_LOG_ERROR("Poll on client side returned %d %d",numberOfFds,errno);
            status = HFS_INTERNAL_ERROR;
            HFS_UNUSED(status);
            goto cleanup;
        }

        //-- For all ids do cool stuff --//
        //-- TODO Optimize this a hell lot --//
        for (i=0 ; i < pHerculesClient->config.maxServersAllowed ; i++) {
            // Client has closed the connection //
            if (pHerculesClient->kernel.pPollFds[i].revents & POLLHUP) {
                DEACTIVATE_CLIENT_SIDE_SOCKET(pHerculesClient,i);
                RELEASE_CLIENT_SOCKET_IF_NO_PENDING(pHerculesClient,i);
            }

            if ((pHerculesClient->kernel.pPollFds[i].revents & POLLNVAL) &&
                pHerculesClient->kernel.pPollFds[i].fd != -1) {
                DEACTIVATE_CLIENT_SIDE_SOCKET(pHerculesClient,i);
                RELEASE_CLIENT_SOCKET_IF_NO_PENDING(pHerculesClient,i);
            }

            if (pHerculesClient->kernel.pPollFds[i].revents & POLLERR) {
                DEACTIVATE_CLIENT_SIDE_SOCKET(pHerculesClient,i);
                RELEASE_CLIENT_SOCKET_IF_NO_PENDING(pHerculesClient,i);
            }

            if (pHerculesClient->kernel.pPollFds[i].revents & (POLLERR | POLLNVAL | POLLHUP)) {
                continue;
            }

            //-- Ok now handle interesting events --//
            if (pHerculesClient->kernel.pPollFds[i].revents & POLLIN) {
                hfsClientSocketProcessCommands(pHerculesClient,i);
            }

        } //-- End of all client connections --//

    }while(pHerculesClient->kernel.clientRunning);

    HFS_LOG_INFO("Bye bye Client inbound");
    status = HFS_STATUS_SUCCESS;
    goto leave;

 cleanup:
 leave:
    HFS_LEAVE();
    return NULL;
}
