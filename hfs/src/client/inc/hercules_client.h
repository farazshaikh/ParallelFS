/*
 * hercules_client.h
 *
 * client header
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

// Config Info
#define MAX_HOST_NAME       HFS_MAX_HOST_NAME
#define SOCKET_EVENT_MASK   (POLLIN | POLLHUP)

//- Well defined ways of using cookie -//
#define COOKIE_QITEM_IDX        0
#define COOKIE_DATA_SERVER_IDX  1

//- Paralleism info                 -//
#define HFS_OUTBOUND_THREAD_NR  16

//- Configuration information for this server --//
typedef struct __HERCULES_CLIENT_CONFIG {
    char  userName[HFS_MAX_USER_NAME+1];
    char  epassword[HFS_MAX_ENC_PASSWORD+1];    //- The encrypted password -//
    char  hostName[MAX_HOST_NAME];
    char *configFilePath;
    char *mntpnt;
    int   maxServersAllowed;
    int   maxMDSServers;
} HERCULES_CLIENT_CONFIG, *PHERCULES_CLIENT_CONFIG;

//-- Client connection context apart from the pollfd --/
typedef enum _RESP_COMM_STAGE {
    RESP_COM_STAGE_WAITING,
    RESP_COM_STAGE_HEADER,
    RESP_COM_STAGE_COMMAND,
    RESP_COMM_STAGE_EXTENT,
    RESP_COMM_STAGE_MAX
} REQ_COMM_STAGE;

typedef struct __HFS_SERVER_CONN_CTX {
    int                 used;
    int                 PendingCommands;        // Also serializes all send on this socket //

    // Reading things off the line in a sane way //
    HFS_PROTO_HEADER    bounceHdr;
    REQ_COMM_STAGE      inStage;
    int                 bytesReadOff;
    int                 commandSize;
    PHFS_QUEUE_ITEM     pQueueItem;

    // Current build up of the queued Item //
    HFS_MUTEX          mutexPendingCommandslst;
    struct list_head    lstAnchorPendingCommands;
} HFS_SERVER_CONN_CTX, *PHFS_SERVER_CONN_CTX;

//-- Core Structures for the running Hercules system --/
typedef struct __HERCULES_CLIENT_KERNEL {
    PHFS_QUEUE_PROCESSOR    pQOutBound;
    int                     clientRunning;
    pthread_t               tidInBoundProcessor;

    // Client Connection info //
    HFS_MUTEX               configUpdateMutex;
    HFS_SEMAPHORE           clientOutBoundCritSections;    // Lock before locking the connection mutex 
    int                     activeServers;
    struct pollfd          *pPollFds;
    PHFS_SERVER_CONN_CTX    pServerConnCtx;
} HERCULES_CLIENT_KERNEL, *PHERCULES_CLIENT_KERNEL;

//-- Running instance of an Hercules server --//
typedef struct __HERCULES_CLIENT {
    HERCULES_CLIENT_CONFIG   config;
    HERCULES_CLIENT_KERNEL   kernel;
} HERCULES_CLIENT, *PHERCULES_CLIENT;

HFS_STATUS hfsFuseInit(int argc, char *argv[]);

//- Global Struct -//
PHERCULES_CLIENT    getHFSClient();
PHFS_CLIENT_OPTIONS getHFSClientOptions();
PHFS_FS_CTX         getHFSFSCtx();

//- Inbound processing -//
void * hfsClientInboundLoop(void *pVVHerculesClient);


//- Outbound processing -//
HFS_STATUS clientOutBoundProcessing(
                                    PHFS_QUEUE_ITEM pQueueItem,
                                    HFS_STATUS      status);

void * hfsConfigChanged(void *pCurrentClient);
void hfsUnsolicitedCommand(PHFS_QUEUE_ITEM pQueueItem);
HFS_STATUS hfsAuthenticateROOTMDS(HERCULES_CLIENT *pHfsClient);

//-- Connections --//
// If all commands are sent out //
// and the connection was deactivated fees it for further use //
#define RELEASE_CLIENT_SOCKET_IF_NO_PENDING(pHerculesServer,clientfdIdx) do { \
        if(!(pHerculesServer)->kernel.pServerConnCtx[(clientfdIdx)].PendingCommands) \
            if(-1 == (pHerculesServer)->kernel.pPollFds[(clientfdIdx)].fd) { \
                (pHerculesServer)->kernel.pPollFds[(clientfdIdx)].fd = -1; \
                (pHerculesServer)->kernel.pPollFds[(clientfdIdx)].revents = 0; \
                (pHerculesServer)->kernel.pPollFds[(clientfdIdx)].events = 0; \
                (pHerculesServer)->kernel.pServerConnCtx[(clientfdIdx)].used  = 0; \
            }                                                           \
    } while(0)

// Close and stop further inclusion in poll - But is still used //
// until all commands on it go out //
#define DEACTIVATE_CLIENT_SIDE_SOCKET(pHerculesClient,clientfdIdx) do { \
        close((pHerculesClient)->kernel.pPollFds[(clientfdIdx)].fd);    \
        (pHerculesClient)->kernel.pPollFds[(clientfdIdx)].fd = -1;      \
        (pHerculesClient)->kernel.pPollFds[(clientfdIdx)].revents = 0;  \
        (pHerculesClient)->kernel.pPollFds[(clientfdIdx)].events = 0;   \
    } while(0)


#define ACTIVATE_CLIENT_SOCKET(pHerculesClient,Idx,fd2install) do {     \
        (pHerculesClient)->kernel.pServerConnCtx[(Idx)].used  = 1;      \
        (pHerculesClient)->kernel.pServerConnCtx[(Idx)].inStage = RESP_COM_STAGE_WAITING; \
        (pHerculesClient)->kernel.pPollFds[(Idx)].fd = (fd2install);    \
        (pHerculesClient)->kernel.pPollFds[(Idx)].revents = 0;          \
        (pHerculesClient)->kernel.pPollFds[(Idx)].events = SOCKET_EVENT_MASK; \
    }while(0)

#define GET_DS_SERVERS_CLIENT_IDX(serverId)                     \
    ((serverId) + (getHFSClient()->config.maxMDSServers))

#define HFS_CLIENT_MDS_IDX_INTERLEAVE     2
#define GET_MDS_SERVERS_CLIENT_IDX(serverId)            \
    ((serverId) * (HFS_CLIENT_MDS_IDX_INTERLEAVE))
