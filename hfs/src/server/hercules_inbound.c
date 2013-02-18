/*
 * hercules_inbound.c
 *
 * Hercules server inbound packet engine
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

#define MAX_CONNECT_BACKLOG 50
#define SOCKET_EVENT_MASK (POLLIN)


HFS_STATUS
hfsSetupConnListener(PHERCULES_SERVER pHerculesServer)
{
    HFS_STATUS status;
    int set = 1;
    int ret;

    //-Create Socket-/
    pHerculesServer->kernel.listeningFD = socket(AF_INET, SOCK_STREAM, 0);
    if (pHerculesServer->kernel.listeningFD < 0) {
        HFS_LOG_ERROR("Cannot create socket for listening");
        status = HFS_INTERNAL_ERROR;
        goto cleanup;
    }

    //-Setup Desired Socket options -//
    ret = setsockopt(pHerculesServer->kernel.listeningFD,
                     SOL_SOCKET,
                     SO_REUSEADDR,
                     (char *)&set, sizeof(set));
    if (ret < 0) {
        HFS_LOG_ERROR("Cannot Set Options for listening socket");
        status = HFS_INTERNAL_ERROR;
        goto cleanup;
    }

    //- Yes !! we are cool enuf to deal with non-blocking Sockets --//
    ret = ioctl(pHerculesServer->kernel.listeningFD, FIONBIO, (char *)&set);
    if (ret < 0) {
        HFS_LOG_ERROR("Cannot Set non-blocking option for socket");
        status = HFS_INTERNAL_ERROR;
        goto cleanup;
    }

    //-- Bind the socket --//
    memset(&pHerculesServer->kernel.addr, 0, sizeof(pHerculesServer->kernel.addr));
    pHerculesServer->kernel.addr.sin_family = AF_INET;
    pHerculesServer->kernel.addr.sin_addr.s_addr = htonl(INADDR_ANY);
    pHerculesServer->kernel.addr.sin_port =
        htons(pHerculesServer->kernel.listeningPort);
    ret = bind(pHerculesServer->kernel.listeningFD,
               (struct sockaddr *)&pHerculesServer->kernel.addr,
               sizeof(pHerculesServer->kernel.addr));
    if (ret < 0) {
        HFS_LOG_ERROR("Cannot bind listening Socket to interface %d", errno);
        status = HFS_INTERNAL_ERROR;
        goto cleanup;
    }

    //-- Setup the connect backlog --//
    ret = listen(pHerculesServer->kernel.listeningFD, MAX_CONNECT_BACKLOG);
    if (ret < 0) {
        HFS_LOG_ERROR("Cannot listen on Socket");
        status = HFS_INTERNAL_ERROR;
        goto cleanup;
    }

    status = HFS_STATUS_SUCCESS;
    goto leave;

 cleanup:
 leave:
    return status;
}


// This is a single thread function please don't let two threads call it //
// There can be only one listener thread per server which does to complete //
// connection setup        //
int
hfsGetFreeClientConnection(PHERCULES_SERVER pHerculesServer)
{
    int i;
    HFS_ENTRY(hfsGetFreeClientConnection);
    for (i = 0 ; i < pHerculesServer->config.maxClientsAllowed ; i++) {
        if (!pHerculesServer->kernel.pClientConnCtx[i].used) {
            pHerculesServer->kernel.pClientConnCtx[i].used = 1;
            pHerculesServer->kernel.pClientConnCtx[i].PendingCommands = 0;

            // Sanity checks//
            if (pHerculesServer->kernel.pPollFds[i].events) {
                HFS_LOG_ERROR("Server will sleep for ever soon1");
            }

            if (pHerculesServer->kernel.pPollFds[i].revents) {
                HFS_LOG_ERROR("Server will sleep for ever soon2");
            }

            if (-1 != pHerculesServer->kernel.pPollFds[i].fd) {
                HFS_LOG_ERROR("Server will sleep for ever soon3");
            }

            // Redundant deactivate
            DEACTIVATE_SOCKET(pHerculesServer, i);
            HFS_LEAVE(hfsGetFreeClientConnection);
            return i;
        }
    }
    HFS_LOG_INFO("Cannot allocate Client Connection");
    HFS_LEAVE(hfsGetFreeClientConnection);
    return -1;
}

HFS_STATUS
hfsBuildPollPool(PHERCULES_SERVER pHerculesServer)
{
    HFS_STATUS status=HFS_STATUS_SUCCESS;
    int i;
    HFS_ENTRY(hfsBuildPollPool);
    pHerculesServer->kernel.pClientConnCtx = NULL;
    pHerculesServer->kernel.pPollFds = NULL;

    pHerculesServer->kernel.pClientConnCtx = (PHFS_CLIENT_CONN_CTX) \
        hfsCalloc(sizeof(*(pHerculesServer->kernel.pClientConnCtx)) * pHerculesServer->config.maxClientsAllowed);
    if (!pHerculesServer->kernel.pClientConnCtx) {
        HFS_LOG_INFO("Cannot Create Client Connection list");
        status = HFS_INTERNAL_ERROR;
        goto cleanup;
    }
    HFS_LOG_INFO("got p connection idx %p", pHerculesServer->kernel.pClientConnCtx);

    pHerculesServer->kernel.pPollFds = (struct pollfd *) \
        hfsCalloc(sizeof(*(pHerculesServer->kernel.pPollFds)) * pHerculesServer->config.maxClientsAllowed);
    if (!pHerculesServer->kernel.pPollFds) {
        HFS_LOG_INFO("Cannot Create Client Connection list");
        status = HFS_INTERNAL_ERROR;
        goto cleanup;
    }
    HFS_LOG_INFO("got pPollFds %p", pHerculesServer->kernel.pPollFds);

    for (i = 0 ; i < pHerculesServer->config.maxClientsAllowed ; i++) {
        // pool fd initialization //
        // De activated and free //
        (pHerculesServer)->kernel.pPollFds[i].fd = -1;
        pHerculesServer->kernel.pClientConnCtx[i].inStage = REQ_COMM_STAGE_MAX;

        //- Initialize the security context -//
        if (getHFSServer()->config.role == SERVER_CONFIG_ROLE_MDS &&
            (getHFSServer()->config.serverInfo.serverIdx == 0 ||
             getHFSServer()->config.serverInfo.serverIdx == 1)) {
            pHerculesServer->kernel.pClientConnCtx[i].authenticated = 0;
            pHerculesServer->kernel.pClientConnCtx[i].randomNonce = (__u64)random();
        }else {
            pHerculesServer->kernel.pClientConnCtx[i].authenticated = 1;
        }
    }

    status = HFS_STATUS_SUCCESS;
    goto leave;
 cleanup:
    if (pHerculesServer->kernel.pPollFds) {
        hfsFree(pHerculesServer->kernel.pPollFds);
    }

    if (pHerculesServer->kernel.pClientConnCtx) {
        hfsFree(pHerculesServer->kernel.pClientConnCtx);
    }
 leave:
    HFS_LEAVE(hfsBuildPollPool);
    return status;
}


// This should not ever return a fatal error - But must log things //
HFS_STATUS
hfsSocketAcceptConnection(PHERCULES_SERVER pHerculesServer)
{
    int newConnectionIdx=-1;
    int newConnectionfd = -1;
    HFS_STATUS status;
    HFS_ENTRY(hfsSocketAcceptConnection);
    newConnectionfd = accept(pHerculesServer->kernel.listeningFD, NULL, NULL);
    if (-1==newConnectionfd) {
        HFS_LOG_INFO("Accept on server socket has failed");
        HFS_ENTRY(hfsSocketAcceptConnection);
        return HFS_STATUS_SUCCESS;
    }

    // for this connection make a connection slot
    newConnectionIdx = hfsGetFreeClientConnection(pHerculesServer);
    if (newConnectionIdx == -1) {
        HFS_LOG_ERROR("[Connection Dropped] Cannot allocate context for incomming");
        HFS_UNUSED(status);
        close(newConnectionfd);
        HFS_ENTRY(hfsSocketAcceptConnection);
        return HFS_STATUS_SUCCESS;
    }

    // Install the new connection fd //
    ACTIVATE_SOCKET(pHerculesServer, newConnectionIdx, newConnectionfd);
    HFS_ENTRY(hfsSocketAcceptConnection);
    return HFS_STATUS_SUCCESS;
}


// Pumps bytes into the connection state machine //
// REQ_COM_STAGE_WAITING -> REQ_COM_STAGE_HEADER -> REQ_COM_STAGE_COMMAND -> REQ_COM_STAGE_COMMAND
// ^ ^       ^   ^
// | |______________________________________________________|   |
// |_______________________________________________________________________________|
//

#define RESET_CONNECTION_STATE_MACHINE(pClientConnCtx) do {     \
        (pClientConnCtx)->inStage = REQ_COM_STAGE_WAITING;      \
        (pClientConnCtx)->bytesReadOff = 0;                     \
        (pClientConnCtx)->bounceHdr.command = 0;                \
        (pClientConnCtx)->pQueueItem = 0;                       \
        (pClientConnCtx)->commandSize = 0;                      \
    }while (0);

int IsValidReqHeader(HFS_PROTO_HEADER *p) {
    return 1;
}

HFS_STATUS
hfsPumpBytesToConnSM(char   *buffer,
                     int   size,
                     int   clientfdIdx,
                     PHFS_QUEUE_PROCESSOR targetQueue)
{
    int   i;
    HFS_STATUS  status=HFS_STATUS_SUCCESS;
    PHFS_CLIENT_CONN_CTX pClientConnCtx;

    pClientConnCtx = &getHFSServer()->kernel.pClientConnCtx[clientfdIdx];

    for (i = 0 ; i < size ; i++) {
        // Adjust the current stage //
        switch(pClientConnCtx->inStage) {
        case REQ_COM_STAGE_WAITING:
            if (pClientConnCtx->bytesReadOff) {
                HFS_LOG_ERROR("Command stream ClientIDX out of sync");
                return HFS_PROTOCOL_ERROR;
            }
            // Transit to next stage //
            pClientConnCtx->inStage++; // Null state go ahead //

        case REQ_COM_STAGE_HEADER:
            if (pClientConnCtx->bytesReadOff < sizeof(HFS_PROTO_HEADER)) {
                ((char *)(&pClientConnCtx->bounceHdr))[pClientConnCtx->bytesReadOff] = buffer[i];
                pClientConnCtx->bytesReadOff++;
            }

            if (pClientConnCtx->bytesReadOff != sizeof(HFS_PROTO_HEADER))
                continue;

            // TRANSIT TO NEXT STAGE //
            hfsDecodeProtoHeader(&pClientConnCtx->bounceHdr);

            // Sanity //
            if (!IsValidReqHeader(&pClientConnCtx->bounceHdr)) {
                HFS_LOG_ERROR("Decoded wrong header");
                return HFS_PROTOCOL_ERROR;
            }

            if (pClientConnCtx->pQueueItem) {
                HFS_LOG_INFO("Possible memory leak");
            }

            // Allocation for next stage //
            pClientConnCtx->pQueueItem =
                hfsAllocateProtoBuff(&pClientConnCtx->bounceHdr, getHFSFSCtx()->stripeSize);
            if (!pClientConnCtx->pQueueItem) {
                HFS_LOG_ERROR("Server low on memory or bad header");
                return HFS_PROTOCOL_ERROR;
            }

            pClientConnCtx->commandSize = hfsGetCommandSize(&pClientConnCtx->bounceHdr);
            if (-1 == pClientConnCtx->commandSize) {
                HFS_LOG_ERROR("ClientIDX out of sync2");
                return HFS_PROTOCOL_ERROR;
            }

            pClientConnCtx->inStage++;
            continue;

        case REQ_COM_STAGE_COMMAND:
            if (pClientConnCtx->bytesReadOff < sizeof(HFS_PROTO_HEADER)||
                !pClientConnCtx->commandSize || -1==pClientConnCtx->commandSize) {
                HFS_LOG_ERROR("HHGG Press panic");
                return HFS_PROTOCOL_ERROR;
            }

            if (pClientConnCtx->bytesReadOff <
                sizeof(HFS_PROTO_HEADER) +
                pClientConnCtx->commandSize) {
                ((char *)(pClientConnCtx->pQueueItem + 1))[pClientConnCtx->bytesReadOff] = buffer[i];
                pClientConnCtx->bytesReadOff++;
            }

            if (pClientConnCtx->bytesReadOff !=
                (sizeof(HFS_PROTO_HEADER) + pClientConnCtx->commandSize)) {
                continue;
            }

            // TRANSIT TO NEXT STAGE //
            status = hfsDecodeCommandBuffer(
                                            (char *)(pClientConnCtx->pQueueItem + 1) +
                                            sizeof(HFS_PROTO_HEADER),
                                            pClientConnCtx->bounceHdr.command);
            if (!HFS_SUCCESS(status)) {
                HFS_LOG_ERROR("cannot decode command buffer");
                return HFS_PROTOCOL_ERROR;
            }

            // If we have payload go to state extent else attach and reset //
            if (pduHasPayLoad(pClientConnCtx->bounceHdr.command) && pClientConnCtx->bounceHdr.pduExtentSize) {
                pClientConnCtx->inStage++;
                continue;
            } else {
                // Queue the item & reset state machine.
                pClientConnCtx->pQueueItem->clientIdx = clientfdIdx;
                status = hfsQueueItem(targetQueue, pClientConnCtx->pQueueItem);
                if (!HFS_SUCCESS(status)) {
                    HFS_LOG_ERROR("Cannot put the packet onto target queue");
                    return HFS_PROTOCOL_ERROR;
                }
                pClientConnCtx->PendingCommands++;
                RESET_CONNECTION_STATE_MACHINE(pClientConnCtx);
                continue;
            }

        case REQ_COMM_STAGE_EXTENT:
            if (pClientConnCtx->bytesReadOff < sizeof(HFS_PROTO_HEADER) ||
                !pClientConnCtx->commandSize || -1==pClientConnCtx->commandSize) {
                HFS_LOG_ERROR("HHGG Press panic2");
                return HFS_PROTOCOL_ERROR;
            }



            if (pClientConnCtx->bytesReadOff <
                sizeof(HFS_PROTO_HEADER) +
                pClientConnCtx->commandSize +
                pClientConnCtx->bounceHdr.pduExtentSize) {
                ((char *)(pClientConnCtx->pQueueItem + 1))[pClientConnCtx->bytesReadOff] = buffer[i];
                pClientConnCtx->bytesReadOff++;
            }

            if (pClientConnCtx->bytesReadOff !=
                sizeof(HFS_PROTO_HEADER) +
                pClientConnCtx->commandSize +
                pClientConnCtx->bounceHdr.pduExtentSize)
                continue;

            // Queue the item & reset state machine.
            pClientConnCtx->pQueueItem->clientIdx = clientfdIdx;
            status = hfsQueueItem(targetQueue, pClientConnCtx->pQueueItem);
            if (!HFS_SUCCESS(status)) {
                HFS_LOG_ERROR("Cannot put the packet onto target queue %ld", status);
                return HFS_PROTOCOL_ERROR;
            }
            pClientConnCtx->PendingCommands++;
            RESET_CONNECTION_STATE_MACHINE(pClientConnCtx);
            continue;

        default:
            HFS_LOG_ERROR("Invalid state for Protocol Reader State machine");
        } // End switch
    }// End for
    status = HFS_STATUS_SUCCESS;
    return HFS_STATUS_SUCCESS;
}


HFS_STATUS
hfsSocketProcessCommands(PHERCULES_SERVER pHerculesServer,
                         int  clientfdIdx)
{
    HFS_STATUS  status=HFS_STATUS_SUCCESS;
    int   rc;
    HFS_PROTO_HEADER header;

    // Read Protocol //
    {
        int totalBytesRead = 0;
        rc = recv(pHerculesServer->kernel.pPollFds[clientfdIdx].fd,
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
        status = hfsPumpBytesToConnSM((char *)&header,
                                      rc,
                                      clientfdIdx,
                                      pHerculesServer->kernel.pQpendingCommands);
        if (!HFS_SUCCESS(status)) {
            goto client_closed;
        }
    }

    status = HFS_STATUS_SUCCESS;
    goto done;

 drop_connection:
    HFS_LOG_INFO("we are dropping the client now");
    DEACTIVATE_SOCKET(pHerculesServer, clientfdIdx);
    RELEASE_SOCKET_IF_NO_PENDING(pHerculesServer, clientfdIdx);
    status = HFS_CONNECTION_DROPPED;
    goto done;

 client_closed:
    HFS_LOG_INFO("Client Closed the connection");
    DEACTIVATE_SOCKET(pHerculesServer, clientfdIdx);
    RELEASE_SOCKET_IF_NO_PENDING(pHerculesServer, clientfdIdx);
    status = HFS_CLIENT_DISCONNECTED;
 done:
    return status;
}


HFS_STATUS
hfsInboundLoop(PHERCULES_SERVER pHerculesServer)
{
    HFS_STATUS status=HFS_STATUS_SUCCESS;
    int numberOfFds;
    int listenerIdx;
    int i;


    HFS_ENTRY(hfsInboundLoop);

    status = hfsBuildPollPool(pHerculesServer);
    if (!HFS_SUCCESS(status)) {
        goto cleanup;
    }

    status = hfsSetupConnListener(pHerculesServer);
    if (!HFS_SUCCESS(status)) {
        goto cleanup;
    }

    listenerIdx = hfsGetFreeClientConnection(pHerculesServer);
    if (listenerIdx == -1) {
        HFS_LOG_ERROR("Cannot allocate connection context for listener socket");
        status = HFS_INTERNAL_ERROR;
        goto cleanup;
    }

    ACTIVATE_SOCKET(pHerculesServer, listenerIdx, pHerculesServer->kernel.listeningFD);
    do {
        //- Wait for all of the the clients sockets we have -//
        numberOfFds = poll(pHerculesServer->kernel.pPollFds,
                           pHerculesServer->config.maxClientsAllowed,
                           -1);

        //- Number of fds -//
        if (numberOfFds<0) {
            if (errno == EINTR) continue;
            HFS_LOG_ERROR("Poll on server returned %d %d", numberOfFds, errno);
            status = HFS_INTERNAL_ERROR;
            goto cleanup;
        }

        //-- For all ids do cool stuff --//
        //-- TODO Optimize this a hell lot --//
        for (i = 0 ; i < pHerculesServer->config.maxClientsAllowed ; i++) {

            // Client has closed the connection //
            if (pHerculesServer->kernel.pPollFds[i].revents & POLLHUP) {
                DEACTIVATE_SOCKET(pHerculesServer, i);
                RELEASE_SOCKET_IF_NO_PENDING(pHerculesServer, i);
            }

            if ((pHerculesServer->kernel.pPollFds[i].revents & POLLNVAL) &&
                pHerculesServer->kernel.pPollFds[i].fd != -1) {
                DEACTIVATE_SOCKET(pHerculesServer, i);
                RELEASE_SOCKET_IF_NO_PENDING(pHerculesServer, i);
            }

            if (pHerculesServer->kernel.pPollFds[i].revents & POLLERR) {
                DEACTIVATE_SOCKET(pHerculesServer, i);
                RELEASE_SOCKET_IF_NO_PENDING(pHerculesServer, i);
            }

            if (pHerculesServer->kernel.pPollFds[i].revents & (POLLERR | POLLNVAL | POLLHUP)) {
                continue;
            }

            //-- Ok now handle interesting events --//
            if (pHerculesServer->kernel.pPollFds[i].revents & POLLIN) {

                //-- Is it an incoming connection --//
                if (pHerculesServer->kernel.pPollFds[i].fd ==
                    pHerculesServer->kernel.listeningFD) {
                    hfsMutexLock(&pHerculesServer->kernel.connectAcceptMutex);
                    hfsSocketAcceptConnection(pHerculesServer);
                    hfsMutexUnlock(&pHerculesServer->kernel.connectAcceptMutex);
                }else {
                    //-- Is it data   --//
                    hfsSocketProcessCommands(pHerculesServer, i);
                }
            }
        } //-- End of all client connections --//
    } while (pHerculesServer->kernel.serverRunning);

    status = HFS_STATUS_SUCCESS;
    goto leave;

 cleanup:
 leave:
    HFS_LEAVE(hfsInboundLoop);
    return status;
}
