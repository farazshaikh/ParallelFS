/*
 * hercules_server.h
 *
 *      Server headers
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


// Config Info
#define MAX_HOST_NAME                     HFS_MAX_HOST_NAME
#define HFS_SERVER_INBOUND_THREAD_NR      16
//- Configuration information for this server --//
typedef struct __HERCULES_SERVER_CONFIG {
    int                 role; 
    char                hostName[MAX_HOST_NAME];
    int                 maxClientsAllowed;
    char               *configFilePath;

    //-- Copy of our identity from FS configuration file --//
    HFS_SERVER_INFO     serverInfo;
} HERCULES_SERVER_CONFIG, *PHERCULES_SERVER_CONFIG;

//-- Clien connection context apart from the pollfd --/
typedef enum _REQ_COMM_STAGE {
    REQ_COM_STAGE_WAITING,
    REQ_COM_STAGE_HEADER,
    REQ_COM_STAGE_COMMAND,
    REQ_COMM_STAGE_EXTENT,
    REQ_COMM_STAGE_MAX
} REQ_COMM_STAGE;

typedef struct __HFS_CLIENT_CONN_CTX {
    int                 used;
    int                 PendingCommands;

    //- Security context -//
    int                 authenticated;
    __u64               randomNonce;
    char                userName[HFS_MAX_USER_NAME];


    // Reading things off the line in a sane way //
    HFS_PROTO_HEADER    bounceHdr;
    REQ_COMM_STAGE      inStage;
    int                 bytesReadOff;
    int                 commandSize;

    // Building up the queued Item //
    PHFS_QUEUE_ITEM     pQueueItem;
} HFS_CLIENT_CONN_CTX, *PHFS_CLIENT_CONN_CTX;

//-- Core Structures for the running Hercules system --/
typedef struct __HERCULES_SRV_KERNEL {
    PHFS_QUEUE_PROCESSOR    pQpendingCommands;
    PHFS_QUEUE_PROCESSOR    pQOutBound;

    // Wait for FS config updates //
    pthread_t               thrFsUpdate;
    int                     FsUpdateSem;

    // Db Context //
    HFS_DB_CTX          hfsDbCtx;

    //- Meta data update lock -//
    HFS_MUTEX               serverMetaDataUpdateLock;

    // Client Connection info //
    __u32                   lastOperation;
    HFS_MUTEX               connectAcceptMutex;
    int                     activeClients;
    struct pollfd          *pPollFds;
    PHFS_CLIENT_CONN_CTX    pClientConnCtx;

    //- Server TCP connection information -//
    struct sockaddr_in   addr;
    int                  timeout;
    int                  listeningPort;
    int                  listeningFD;
    int                  serverRunning;
} HERCULES_SERVER_KERNEL, *PHERCULES_SERVER_KERNEL;

//-- Running instance of an Hercules server --//
typedef struct __HERCULES_SERVER {
    HERCULES_SERVER_CONFIG   config;
    HERCULES_SERVER_KERNEL   kernel;
} HERCULES_SERVER, *PHERCULES_SERVER;


//- Global Struct -//
PHERCULES_SERVER    getHFSServer();
PHFS_SERVER_OPTIONS getHFSServerOptions();
PHFS_FS_CTX         getHFSFSCtx();

//- Processing function -//
HFS_STATUS outBoundProcessing(PHFS_QUEUE_ITEM,HFS_STATUS);
HFS_STATUS mdsPendingCommandProcessing(PHFS_QUEUE_ITEM,HFS_STATUS);
HFS_STATUS dsPendingCommandProcessing(PHFS_QUEUE_ITEM,HFS_STATUS);

//- Process the incoming connections for this server -//
HFS_STATUS hfsInboundLoop(PHERCULES_SERVER);

//- Marking of the last operation on the server -//
void hfsServerMarkLastOperation(PHERCULES_SERVER pHerculesServer,__u32 command);

//-- Connections --//
// If all commands are sent out //
// and the connection was deactivated fees it for further use //
#define RELEASE_SOCKET_IF_NO_PENDING(pHerculesServer,clientfdIdx) do {  \
        if(!(pHerculesServer)->kernel.pClientConnCtx[(clientfdIdx)].PendingCommands) \
            if(-1 == (pHerculesServer)->kernel.pPollFds[(clientfdIdx)].fd) { \
                (pHerculesServer)->kernel.pPollFds[(clientfdIdx)].fd = -1; \
                (pHerculesServer)->kernel.pPollFds[(clientfdIdx)].revents = 0; \
                (pHerculesServer)->kernel.pPollFds[(clientfdIdx)].events = 0; \
                (pHerculesServer)->kernel.pClientConnCtx[(clientfdIdx)].used  = 0; \
            }                                                           \
    }while(0)

// Close and stop further inclusion in poll - But is still used //
// until all commands on it go out //
#define DEACTIVATE_SOCKET(pHerculesServer,clientfdIdx) do {             \
        (pHerculesServer)->kernel.pClientConnCtx[(clientfdIdx)].bytesReadOff = 0; \
        close((pHerculesServer)->kernel.pPollFds[(clientfdIdx)].fd);    \
        (pHerculesServer)->kernel.pPollFds[(clientfdIdx)].fd = -1;      \
        (pHerculesServer)->kernel.pPollFds[(clientfdIdx)].revents = 0;  \
        (pHerculesServer)->kernel.pPollFds[(clientfdIdx)].events = 0;   \
        memset((pHerculesServer)->kernel.pClientConnCtx[(clientfdIdx)].userName,0,HFS_MAX_USER_NAME); \
        if((pHerculesServer)->config.role == SERVER_CONFIG_ROLE_MDS &&  \
           ((pHerculesServer)->config.serverInfo.serverIdx == 0 ||      \
            (pHerculesServer)->config.serverInfo.serverIdx == 1)) {     \
            pHerculesServer->kernel.pClientConnCtx[(clientfdIdx)].authenticated = 0; \
            pHerculesServer->kernel.pClientConnCtx[(clientfdIdx)].randomNonce   = (__u64)random(); \
        }else {                                                         \
            pHerculesServer->kernel.pClientConnCtx[(clientfdIdx)].authenticated = 1; \
        }                                                               \
    }while(0)


#define ACTIVATE_SOCKET(pHerculesServer,listenerIdx,fd2install) do {    \
        (pHerculesServer)->kernel.pClientConnCtx[(listenerIdx)].bytesReadOff = 0; \
        (pHerculesServer)->kernel.pClientConnCtx[(listenerIdx)].used  = 1; \
        (pHerculesServer)->kernel.pClientConnCtx[(listenerIdx)].inStage = REQ_COM_STAGE_WAITING; \
        (pHerculesServer)->kernel.pPollFds[(listenerIdx)].fd = (fd2install); \
        (pHerculesServer)->kernel.pPollFds[(listenerIdx)].revents = 0;  \
        (pHerculesServer)->kernel.pPollFds[(listenerIdx)].events = SOCKET_EVENT_MASK; \
    }while(0)
