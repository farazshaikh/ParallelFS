/*
 * hercules_client.c
 *
 *      hercules client that talks to meta data and data servers to
 *      provide filesystem services
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

//-- Get the running instance of the client --//
PHERCULES_CLIENT
getHFSClient()
{
    static HERCULES_CLIENT herculesClient;
    return &herculesClient;
}

//-- Command line options for starting the client.
PHFS_CLIENT_OPTIONS
getHFSClientOptions()
{
    static HFS_CLIENT_OPTIONS  hfsClientOptions;
    return &hfsClientOptions;
}

//-- Overall file system Context --//
PHFS_FS_CTX *
getPPHFSFSCtx() {
    static PHFS_FS_CTX pHfsFSCtx;
    return &pHfsFSCtx;
}

PHFS_FS_CTX
getHFSFSCtx()
{
    return (*getPPHFSFSCtx());
}

//-- Overall file system Context from MDS--//
PHFS_FS_CTX * getPPHFSFSCtx_MDS() {
    static PHFS_FS_CTX          pHfsFSCtx_MDS;
    return &pHfsFSCtx_MDS;
}

PHFS_FS_CTX getHFSFSCtx_MDS() {
    return (*getPPHFSFSCtx_MDS());
}


////////////////////////////////////////////////////////////////////////////////
void
sigPipeHandler(int signo)
{
    HFS_ENTRY();
    HFS_LEAVE();
    return;
}


//-- Signal handle bookeeping -//
HFS_STATUS
setSignals()
{
    struct sigaction action,  old_action;
    HFS_ENTRY();
    action.sa_handler = sigPipeHandler;
    sigemptyset(&action.sa_mask); //-- block sigs of type being handled --//
    action.sa_flags = SA_RESTART; //-- restart syscalls if possible --//

    if (sigaction(SIGPIPE, &action, &old_action) < 0) {
        perror("Signal error");
        return HFS_STATUS_CONFIG_ERROR;
    }

    HFS_LEAVE();
    return (HFS_STATUS_SUCCESS);
}


void printUsage() {
    printf("hercules_client mnt_point ROOT_MDS_IP ROOT_MDS_PORT client_log_file");
}

#define MAX_SERVERS_SUPPORTED       1024
#define MAX_MDS_SERVERS_SUPPORTED   128
#define MAX_DS_SERVERS_SUPPORTED    (MAX_SERVERS_SUPPORTED - MAX_MDS_SUPPORTED)
HFS_STATUS
parse_cmd_line_options(int argc,
                       char **argv,
                       PHFS_CLIENT_OPTIONS opts)
{
    HFS_STATUS ret=HFS_STATUS_SUCCESS;
    HFS_ENTRY(parse_cmd_line_options);

    if (argc != 5){
        HFS_LOG_ERROR("Cannot Parse Command Line");
        ret = HFS_INTERNAL_ERROR;
        goto leave;
    }
    //-- Get the config file name path --//
    opts->mountPoint        = argv[1];
    opts->fsConfigFilePath  = NULL;
    opts->rootMDSAddr       = argv[2];
    opts->rootMDSPort       = argv[3];
    opts->clientLogFile     = argv[4];

    opts->logMask                   = -1;
    opts->maxServersSupported       = MAX_SERVERS_SUPPORTED;
    opts->maxMdsServersSupported    = MAX_MDS_SERVERS_SUPPORTED;

 leave:
    if (!HFS_SUCCESS(ret)) {
        printUsage();
    }

    HFS_LEAVE(parse_cmd_line_options);
    return ret;
}


HFS_STATUS
hfsBuildServerPollPool(PHERCULES_CLIENT pHerculesClient)
{
    HFS_STATUS status=HFS_STATUS_SUCCESS;
    int i;

    HFS_ENTRY();
    pHerculesClient->kernel.pServerConnCtx = NULL;
    pHerculesClient->kernel.pPollFds       = NULL;

    pHerculesClient->kernel.pServerConnCtx = (PHFS_SERVER_CONN_CTX) \
        hfsCalloc(sizeof(*(pHerculesClient->kernel.pServerConnCtx)) * pHerculesClient->config.maxServersAllowed);
    if (!pHerculesClient->kernel.pServerConnCtx) {
        HFS_LOG_INFO("Cannot Create Server Connection list");
        status = HFS_INTERNAL_ERROR;
        goto cleanup;
    }
    HFS_LOG_INFO("got pservecnnctx %p", pHerculesClient->kernel.pServerConnCtx);

    pHerculesClient->kernel.pPollFds = (struct pollfd *)                \
        hfsCalloc(sizeof(*(pHerculesClient->kernel.pPollFds)) * pHerculesClient->config.maxServersAllowed);
    if (!pHerculesClient->kernel.pPollFds) {
        HFS_LOG_INFO("Cannot Create Server Connection list");
        status = HFS_INTERNAL_ERROR;
        goto cleanup;
    }
    HFS_LOG_INFO("got pPollFds %p",  pHerculesClient->kernel.pPollFds);

    for(i=0; i<pHerculesClient->config.maxServersAllowed; i++) {
        // pool fd initialization //
        // De activated and free  //
        pHerculesClient->kernel.pPollFds[i].fd = -1;
        pHerculesClient->kernel.pServerConnCtx[i].inStage = RESP_COMM_STAGE_MAX;

        // pending commands list //
        pHerculesClient->kernel.pServerConnCtx[i].pQueueItem = NULL;
        hfsMutexInit(&pHerculesClient->kernel.pServerConnCtx[i].mutexPendingCommandslst);
        INIT_LIST_HEAD(&pHerculesClient->kernel.pServerConnCtx[i].lstAnchorPendingCommands);
    }

    status = HFS_STATUS_SUCCESS;
    goto leave;

 cleanup:
    if (pHerculesClient->kernel.pPollFds) {
        hfsFree(pHerculesClient->kernel.pPollFds);
    }

    if (pHerculesClient->kernel.pServerConnCtx) {
        hfsFree(pHerculesClient->kernel.pServerConnCtx);
    }

 leave:
    HFS_LEAVE();
    return status;
}

void
hfsFreePollPool(HERCULES_CLIENT *pHerculesClient)
{
    if (pHerculesClient->kernel.pPollFds) {
        HFS_LOG_INFO("pollfds free %p", pHerculesClient->kernel.pPollFds);
        hfsFree(pHerculesClient->kernel.pPollFds);
    }

    if (pHerculesClient->kernel.pServerConnCtx) {
        HFS_LOG_INFO("pServerconct free %p", pHerculesClient->kernel.pServerConnCtx);
        hfsFree(pHerculesClient->kernel.pServerConnCtx);
    }
}

HFS_STATUS
setClientSockOpts(int fd)
{
    int set = 1;
    int ret;
    HFS_ENTRY();

    //-Setup Desired Socket options -//
    ret = setsockopt(fd,
                     SOL_SOCKET,
                     SO_REUSEADDR,
                     (char *)&set, sizeof(set));
    if (ret < 0) {
        HFS_LOG_ERROR("Cannot Set Options for socket");
        return HFS_INTERNAL_ERROR;
    }

    //- Yes !! we are cool enuf to deal with non-blocking Sockets --//
    ret = ioctl(fd,  FIONBIO,  (char *)&set);
    if (ret < 0) {
        HFS_LOG_ERROR("Cannot Set non-blocking option for socket");
        return HFS_INTERNAL_ERROR;
    }

    HFS_LEAVE();
    return HFS_STATUS_SUCCESS;
}


HFS_STATUS
resetClientSockOpts(int fd)
{
    int reset = 0;
    int ret;
    HFS_ENTRY();

    //-Setup Desired Socket options -//
    ret = setsockopt(fd,
                     SOL_SOCKET,
                     SO_REUSEADDR,
                     (char *)&reset,  sizeof(reset));
    if (ret < 0) {
        HFS_LOG_ERROR("Cannot Set Options for socket");
        return HFS_INTERNAL_ERROR;
    }

    //-- deal with non-blocking Sockets --//
    ret = ioctl(fd,  FIONBIO,  (char *)&reset);
    if (ret < 0) {
        HFS_LOG_ERROR("Cannot Set non-blocking option for socket");
        return HFS_INTERNAL_ERROR;
    }

    HFS_LEAVE();
    return HFS_STATUS_SUCCESS;
}

//- Connect to all servers in the FS context -//
HFS_STATUS hfsConnectServers(PHERCULES_CLIENT    pHerculesClient,
                             PHFS_FS_CTX         pHFSFSCtx)
{
    HFS_STATUS          status = HFS_STATUS_SUCCESS;
    int                 ret;
    __u32               mdscount = 0,  dscount = 0;
    PHFS_SERVER_INFO    pMDSServerInfo;
    PHFS_SERVER_INFO    pDSServerInfo;
    struct sockaddr_in  server_addr;

    HFS_ENTRY(hfsConnectServers);

    //- Sanity checks -//
    if (pHFSFSCtx->mdsCount > pHerculesClient->config.maxMDSServers) {
        HFS_LOG_ERROR("Cannot support these many servers");
        return HFS_STATUS_CONFIG_ERROR;
    }

    if (pHFSFSCtx->dsCount >
       (pHerculesClient->config.maxServersAllowed - pHerculesClient->config.maxMDSServers)){
        HFS_LOG_ERROR("Cannot support these many servers");
        return HFS_STATUS_CONFIG_ERROR;
    }

    //-- Build the pool server --//
    status = hfsBuildServerPollPool(pHerculesClient);
    if (!HFS_SUCCESS(status)) {
        return status;
    }

    //-- Start the MDS  --//
    while(mdscount < pHFSFSCtx->mdsCount &&
          mdscount < pHerculesClient->config.maxMDSServers) {
        int mdsIdx;
        pMDSServerInfo = hfsGetMDSFromServIdx(pHFSFSCtx,  mdscount);
        if (!pMDSServerInfo) {
            HFS_LOG_ERROR("Cannot get meta data server info for server %d", mdscount+1);
            return HFS_STATUS_CONFIG_ERROR;
        }

        mdsIdx = mdscount;      // Do nothing here this is straight ordering and has no interleave factors
        pHerculesClient->kernel.pPollFds[mdsIdx].fd = socket(AF_INET,  SOCK_STREAM, 0);
        if (-1 == pHerculesClient->kernel.pPollFds[mdsIdx].fd) {
            HFS_LOG_ERROR("Cannot get meta data server info for server %d", mdscount);
            return HFS_STATUS_CONFIG_ERROR;
        }

        HFS_LOG_INFO("Connecting to MDS %s[%s]\n", pMDSServerInfo->ipAddr, pMDSServerInfo->port);
        server_addr.sin_family = AF_INET;
        server_addr.sin_port   = htons(atoi(pMDSServerInfo->port));
        inet_aton(pMDSServerInfo->ipAddr, &server_addr.sin_addr);

        ret = connect(pHerculesClient->kernel.pPollFds[mdsIdx].fd,
                      (struct sockaddr *)&server_addr,
                      sizeof(struct sockaddr));
        if (-1 == ret) {
            HFS_LOG_ERROR("Connect to MDS Server failed %d errno %d", mdscount, errno);
            return HFS_STATUS_CONFIG_ERROR;
        }
        HFS_LOG_INFO("MDS Server ID %d Connection  Successful\n",  mdscount);

        // Set Async mode //
        status = setClientSockOpts(pHerculesClient->kernel.pPollFds[mdsIdx].fd);
        if (!HFS_SUCCESS(status)) {
            return status;
        }

        ACTIVATE_CLIENT_SOCKET(pHerculesClient, mdscount, pHerculesClient->kernel.pPollFds[mdscount].fd);
        mdscount++;
    }
    pMDSServerInfo = NULL;

    //-- Start the MDS  --//
    dscount = 0;
    while(dscount < pHFSFSCtx->dsCount &&
          dscount < pHerculesClient->config.maxServersAllowed - pHerculesClient->config.maxMDSServers) {
        int dsIdx = GET_DS_SERVERS_CLIENT_IDX(dscount);

        pDSServerInfo = hfsGetDSFromServIdx(pHFSFSCtx, dscount);
        if (!pDSServerInfo) {
            HFS_LOG_ERROR("Cannot get data server info for server %d", dscount);
            return HFS_STATUS_CONFIG_ERROR;
        }

        pHerculesClient->kernel.pPollFds[dsIdx].fd = socket(AF_INET,  SOCK_STREAM, 0);
        if (-1 == pHerculesClient->kernel.pPollFds[dsIdx].fd) {
            HFS_LOG_ERROR("Cannot get data server info for server %d", dscount);
            return HFS_STATUS_CONFIG_ERROR;
        }

        HFS_LOG_INFO("Connecting to DS %s[%s]\n", pDSServerInfo->ipAddr, pDSServerInfo->port);
        server_addr.sin_family = AF_INET;
        server_addr.sin_port   = htons(atoi(pDSServerInfo->port));
        inet_aton(pDSServerInfo->ipAddr, &server_addr.sin_addr);

        ret = connect(pHerculesClient->kernel.pPollFds[dsIdx].fd,
                      (struct sockaddr *)&server_addr,
                      sizeof(struct sockaddr));
        if (-1 == ret) {
            HFS_LOG_ERROR("Connect to DS Server failed %d errno %d", dscount, errno);
            return HFS_STATUS_CONFIG_ERROR;
        }
        HFS_LOG_INFO("DS Server ID %d Connection  Successful\n", dscount);

        // Set Async Mode //
        status = setClientSockOpts(pHerculesClient->kernel.pPollFds[dsIdx].fd);
        if (!HFS_SUCCESS(status)) {
            return status;
        }
        ACTIVATE_CLIENT_SOCKET(pHerculesClient, dsIdx, pHerculesClient->kernel.pPollFds[dsIdx].fd);
        dscount++;
    }

    pHerculesClient->kernel.activeServers = pHFSFSCtx->dsCount + pHFSFSCtx->mdsCount;
    return HFS_STATUS_SUCCESS;
}


//- Disconnect all servers to all servers in the FS context -//
void
hfsDisConnectServers(PHERCULES_CLIENT pHerculesClient)
{
    int i;
    HFS_ENTRY();
    for(i=0; i<pHerculesClient->config.maxServersAllowed; i++) {
        close(pHerculesClient->kernel.pPollFds[i].fd);
    }
    HFS_LEAVE();
}

//- Build up client configuration -//
HFS_STATUS
hfsBuildClient(PHFS_CLIENT_OPTIONS opts,
               PHFS_FS_CTX         phFSFsCtx,
               PHERCULES_CLIENT    pHerculesClient)
{
    HFS_STATUS          status=HFS_STATUS_SUCCESS;
    int                 ret;
    time_t              now;
    int                 i;

    HFS_ENTRY();
    srandom((unsigned int)time(&now));
    srand((unsigned int)time(&now));

        //-- Get the host name--//
    ret = gethostname(pHerculesClient->config.hostName, MAX_HOST_NAME);
    if (ret) {
        HFS_LOG_ERROR("Could not get host name\n");
    }

    //-- Ok now lets own the server Info from the configuration as own --//
    pHerculesClient->config.configFilePath = opts->fsConfigFilePath;
    pHerculesClient->config.mntpnt         = opts->mountPoint;
    pHerculesClient->config.maxServersAllowed   = opts->maxServersSupported;
    pHerculesClient->config.maxMDSServers       = opts->maxMdsServersSupported;

    //- Allocate a signal handler -//
    if (!HFS_SUCCESS(setSignals())) {
        HFS_LOG_ERROR("Signal Handler Not Installed...\n");
        HFS_LEAVE();
        return HFS_STATUS_CONFIG_ERROR;
    }

    //-- Connect Authenticate with the servers Section take out from build kernel-//
    //- Server List Information -//
    pHerculesClient->kernel.activeServers  = 0;
    pHerculesClient->kernel.pPollFds       = NULL;
    pHerculesClient->kernel.pServerConnCtx = NULL;
    pHerculesClient->kernel.pQOutBound     = NULL;

    //- Initialialize the number of outbound critical sections -//
    hfsSemInit(&pHerculesClient->kernel.clientOutBoundCritSections,
               1,
               0);
    status = hfsMutexInit(&pHerculesClient->kernel.configUpdateMutex);
    if (!HFS_SUCCESS(status)) {
        return status;
    }

    //-- Signal Available critical sections --//
    for(i=0 ; i<HFS_OUTBOUND_THREAD_NR ; i++) {
        hfsSempost(&pHerculesClient->kernel.clientOutBoundCritSections);
    }

    //- Connect to all servers -//
    status = hfsConnectServers(pHerculesClient, phFSFsCtx);
    if (!HFS_SUCCESS(status)) {
        printf("Failed Connecting to Servers [will not mounting]");
        return status;
    }

    status = hfsAuthenticateROOTMDS(pHerculesClient);
    if (status == HFS_STATUS_AUTH_FAILED) {
        printf("Authentication Failed ![will not mount]\n");
        exit(255);
    }

    if (!HFS_SUCCESS(status)) {
        printf("\n Encountered errors while mounting file System [will not mount]");
        return status;
    }

    status = HFS_STATUS_SUCCESS;
    HFS_LEAVE();
    return status;
}

//-- Client --//
HFS_STATUS
hfsBuildClientKernel(PHERCULES_CLIENT    pHerculesClient,
                     PHFS_FS_CTX         phFSFsCtx)
{
    HFS_STATUS  status = HFS_STATUS_SUCCESS;
    int         ret;

    //- Allocate the client side outbound queue -//
    status = hfsQueueAlloc(&pHerculesClient->kernel.pQOutBound);
    if (!HFS_SUCCESS(status)) {
        pHerculesClient->kernel.pQOutBound = NULL;
        goto cleanup;
    }

    // TODO Increase threads and debug //
    // Crank up client side inbound //
    status = hfsQueueInit(pHerculesClient->kernel.pQOutBound,
                          clientOutBoundProcessing,
                          HFS_OUTBOUND_THREAD_NR);
    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR("Cannot start the out bound Queue for client");
        goto cleanup;
    }
    hfsQueueSetState(pHerculesClient->kernel.pQOutBound, QUEUE_STATE_STARTED);

    // Set up the incoming loop //
    pHerculesClient->kernel.clientRunning = 1;
    ret = pthread_create(&pHerculesClient->kernel.tidInBoundProcessor, NULL, hfsClientInboundLoop, pHerculesClient);
    if (ret) {
        HFS_LOG_ERROR("Client Inbound Queing setup failed %d", errno);
        return HFS_STATUS_QUEUE_NOT_STARTED;
    }

    status = HFS_STATUS_SUCCESS;
    goto done;

 cleanup:
    if (pHerculesClient->kernel.pQOutBound) {
        hfsQueuehfsFree(pHerculesClient->kernel.pQOutBound);
    }

 done:
    return status;
}

//-- Dynamically updating the FS configuration without FS remount --//
//-- FS Callback Config Changed --//

void
hfsUnsolicitedCommand(PHFS_QUEUE_ITEM pQueueItem)
{
    PHFS_SERVER_RESP pServResp;
    pthread_t        th;
    int              ret;

    pServResp = (PHFS_SERVER_RESP)(pQueueItem + 1);
    switch(pServResp->hdr.command) {
    case CMD_RESP_MDS_FS_CONF_CHANGED:
        // Nothing interesting is the packet as such for this command //
        ret = pthread_create(&th, NULL, hfsConfigChanged, getHFSClient());
        if (ret) {
            HFS_LOG_ERROR("Cannot start thread for unsolicited command processing %d", errno);
            break;
        }
        pthread_detach(th);
        break;
    default:
        HFS_LOG_ERROR("Unknown unsolicited command Dropping it");
    }
    hfsFreeProtoBuff(pQueueItem);
}


HFS_STATUS
hfsHoldClient(PHERCULES_CLIENT pHerculesClient)
{
    int i;
    HFS_ENTRY();
    // Acquire all the critical sections semaphores for the outgoing Q's//
    for(i = 0 ; i < HFS_OUTBOUND_THREAD_NR ; i++) {
        hfsSemWait(&pHerculesClient->kernel.clientOutBoundCritSections);
    }

    HFS_LEAVE();
    return HFS_STATUS_SUCCESS;
}

HFS_STATUS
hfsReleaseClient(PHERCULES_CLIENT pHerculesClient)
{
    int i;
    HFS_ENTRY();
    // Acquire all the critical sections semaphores for the outgoing Q's//
    for(i = 0 ; i < HFS_OUTBOUND_THREAD_NR ; i++) {
        hfsSempost(&pHerculesClient->kernel.clientOutBoundCritSections);
    }

    HFS_LEAVE();
    return HFS_STATUS_SUCCESS;
}


//- Wait for completion of the in flight commands -//
//- Call this only after the client has been frozen after hfsHoldClient --//
#define WAIT_RETRIES            4
#define WAIT_DELAY              3000
HFS_STATUS
hfsWaitForInFlight(PHERCULES_CLIENT pHerculesClient)
{
    int i, j;
    HFS_ENTRY();
    for(i=0; i<WAIT_RETRIES; i++) {

        for(j = 0; j < pHerculesClient->config.maxServersAllowed; j++) {
            if (0 == pHerculesClient->kernel.pServerConnCtx[j].PendingCommands) {
                continue;
            } else {
                break;
            }
        }

        if (j >= pHerculesClient->config.maxServersAllowed) {
            HFS_LOG_INFO("All inflight commands completed");
            HFS_LEAVE();
            return HFS_STATUS_SUCCESS;
        }
        usleep(WAIT_DELAY);
    }
    HFS_LEAVE();
    return HFS_STATUS_LOCKING_ERROR;
}

//-- Will be called in the context of an independant thread --//
void *
hfsConfigChanged(void *pCurrentClient)
{
    HFS_STATUS              status = HFS_STATUS_SUCCESS;
    HFS_STATUS              cstat  = HFS_STATUS_SUCCESS;
    PHFS_FS_CTX             pHfsFSCtx_MDSlocal=NULL;
    HERCULES_CLIENT         hfsClientLocal, *pHfsCurrentClient=NULL;
    PHFS_SERVER_INFO        pRootBackupServerInfo = NULL;

    //--Required for swapping--//
    PHFS_FS_CTX             ptempFSCtx;
    HERCULES_CLIENT_KERNEL  tempKernel;
    //--Stages of the code --/
    int                     serversConnected=0;
    int                     clientHeld=0;
    HFS_ENTRY(hfsConfigChanged);

    pHfsCurrentClient = (HERCULES_CLIENT *)pCurrentClient;
    memset(&hfsClientLocal, 0, sizeof(hfsClientLocal));

    status = hfsMutexLock(&pHfsCurrentClient->kernel.configUpdateMutex);
    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR("CRITICAL:Failed to acquire client configuration update mutex");
        return NULL;
    }

    // -- Get the server info of the backup root MDS --//
    pRootBackupServerInfo = hfsGetMDSFromServIdx(getHFSFSCtx(), HFS_ROOT_SERVER_IDX + 1);
    if (NULL == pRootBackupServerInfo) {
        HFS_LOG_ERROR("CRITICAL:Failed to acquire Backup Root MDS. Failed to Proceed");
        return NULL;
    }

    do {
        //-- Get the new context of the entire file system from Root MDS --//
        status = hfsBuildFSContext_MDS(getHFSClientOptions()->rootMDSAddr,
                                       getHFSClientOptions()->rootMDSPort,
                                       &pHfsFSCtx_MDSlocal);
        if (!HFS_SUCCESS(status)) {

            //-- Probably Root MDS has MySqlServer Has Gone away error. Try Backup Root MDS --//
            status = hfsBuildFSContext_MDS(pRootBackupServerInfo->ipAddr,
                                           pRootBackupServerInfo->port,
                                           &pHfsFSCtx_MDSlocal);
            if (!HFS_SUCCESS(status)) {
                HFS_LOG_ERROR("Failed to build FS Config");
                break;
            }
        }

        memcpy(&hfsClientLocal,  &getHFSClient()->config,  sizeof(HERCULES_CLIENT_CONFIG));

        //-- Connect to local copy of the client to the servers --//
        status = hfsConnectServers(&hfsClientLocal, pHfsFSCtx_MDSlocal);
        if (!HFS_SUCCESS(status)) {
            HFS_LOG_ERROR("Failed to connect to updated list of servers, Retaining old");
            break;
        }
        serversConnected  = 1;

        status = hfsHoldClient(pHfsCurrentClient);
        if (!HFS_SUCCESS(status)) {
            HFS_LOG_ERROR("Client hold failed");
            break;
        }
        clientHeld = 1;

        status = hfsWaitForInFlight(pHfsCurrentClient);
        if (!HFS_SUCCESS(status)) {
            HFS_LOG_ERROR("Client flush failed");
            break;
        }

        //-- From this point down its success or exit 0 --//
        //-- Swap the Client to New config --//
        tempKernel = hfsClientLocal.kernel;
        hfsClientLocal.kernel =  pHfsCurrentClient->kernel;

        //-- Copy the new kernel connection info --//
        pHfsCurrentClient->kernel.activeServers = tempKernel.activeServers;
        pHfsCurrentClient->kernel.pPollFds      = tempKernel.pPollFds;
        pHfsCurrentClient->kernel.pServerConnCtx= tempKernel.pServerConnCtx;

        //-- Swap the Current copy of the FS context with the new one --//
        ptempFSCtx          = pHfsFSCtx_MDSlocal;
        pHfsFSCtx_MDSlocal  = getHFSFSCtx();
        *getPPHFSFSCtx()     = ptempFSCtx;

        //-- Release the client and now cross your fingers --//
        status = hfsReleaseClient(pHfsCurrentClient);
        if (!HFS_SUCCESS(status)) {
            HFS_LOG_ERROR("CRITICAL:Failed to release client bailing out mount");
            exit(0);
        }

        clientHeld = 0;
        status = HFS_STATUS_SUCCESS;
    }while(0);

    goto cleanup;
 cleanup:
    //- Release the client if held -//
    if (clientHeld) {
        cstat = hfsReleaseClient(pHfsCurrentClient);
        if (!HFS_SUCCESS(cstat)) {
            HFS_LOG_ERROR("CRITICAL:Failed to release client bailing out mount");
            exit(0);
        }
    }

    //- Free a copy of the FS configuration -//
    if (pHfsFSCtx_MDSlocal) {          // Could be any old or new
        HFS_LOG_INFO("free %p", pHfsFSCtx_MDSlocal);
        hfsFree(pHfsFSCtx_MDSlocal);
    }

    //- Disconnect a set of servers -//
    if (serversConnected) {             // Could be any old or new
        hfsDisConnectServers(&hfsClientLocal);
        //hfsFreePollPool(&hfsClientLocal);
    }

    cstat = hfsMutexUnlock(&pHfsCurrentClient->kernel.configUpdateMutex);
    if (!HFS_SUCCESS(cstat)){
        HFS_LOG_ERROR("CRITICAL:Failed to release client configuration update mutex");
        exit(0);
    }

    HFS_LEAVE(hfsConfigChanged);
    return NULL;
}


//-- CHAP Logon --//
//-- The client must currently authenticate itself with the root MDS --//
//-- Called from hfsConnectServer before starting pooling on the socket --//
//-- So the protobuff infrastructure is not be used here --//
//-- There are some open issues here that we currently donot handle --//
//-- AKA authenticating to the backup etc --//
HFS_STATUS
hfsAuthenticateROOTMDS(HERCULES_CLIENT *pHfsClient)
{
    HFS_STATUS              status;
    int                     i;
    HFS_PROTO_HEADER        hdr;
    REQ_MDS_LOGIN_PHASE1    reqLoginPhase1;
    RESP_MDS_LOGIN_PHASE1   respLoginPhase1;
    REQ_MDS_LOGIN_PHASE2    reqLoginPhase2;
    RESP_MDS_LOGIN_PHASE2   respLoginPhase2;
    int                     serverAuthenticated[2] = { 0,  0 };

    for(i = HFS_ROOT_SERVER_IDX ; i <= HFS_ROOT_SERVER_IDX+1 ; i++) {
        int socketfd;
        int ret;
        int decrpytion_status;

        memset(&hdr, 0, sizeof(hdr));
        memset(&reqLoginPhase1, 0, sizeof(reqLoginPhase1));
        memset(&respLoginPhase1, 0, sizeof(respLoginPhase1));
        memset(&reqLoginPhase2, 0, sizeof(reqLoginPhase2));
        memset(&respLoginPhase2, 0, sizeof(respLoginPhase2));


        socketfd = pHfsClient->kernel.pPollFds[i].fd;
        if (socketfd == -1) {
            continue;
        }

        status = resetClientSockOpts(socketfd);
        if (!HFS_SUCCESS(status)) {
            continue;
        }

        // Send out the login phase 1 commands //
        INIT_HEADER(&hdr, CMD_REQ_MDS_LOGIN_PHASE1);
        memcpy(reqLoginPhase1.user, pHfsClient->config.userName, HFS_MAX_USER_NAME);

        hfsEncodeProtoHeader(&hdr);
        hfsEncodeCommandBuffer((char *)&reqLoginPhase1, CMD_REQ_MDS_LOGIN_PHASE1);

        ret = send(socketfd, &hdr, sizeof(hdr), 0);
        if (ret != sizeof(hdr)) {
            continue;
        }

        ret = send(socketfd, &reqLoginPhase1, sizeof(reqLoginPhase1), 0);
        if (ret != sizeof(reqLoginPhase1)) {
            continue;
        }

        // Receive phase 1 challenge //
        ret = recv(socketfd, &hdr, sizeof(hdr), 0);
        if (ret != sizeof(hdr)) {
            continue;
        }

        ret = recv(socketfd, &respLoginPhase1, sizeof(respLoginPhase1), 0);
        if (ret != sizeof(respLoginPhase1)) {
            continue;
        }

        hfsDecodeProtoHeader(&hdr);
        hfsDecodeCommandBuffer((char *)&respLoginPhase1, CMD_REQ_MDS_LOGIN_PHASE1);

        if (!HFS_SUCCESS(hdr.status)) {
            continue;
        }

        // Send out the login phase 2 commands //
        INIT_HEADER(&hdr, CMD_REQ_MDS_LOGIN_PHASE2);
        memcpy(reqLoginPhase2.user, pHfsClient->config.userName, HFS_MAX_USER_NAME);
        reqLoginPhase2.u.dencryptedNonce = respLoginPhase1.u.encryptedNonce;
        decrpytion_status = ecb_crypt(getHFSClient()->config.epassword,
                                      (char *)&reqLoginPhase2.u.dencryptedNonce,
                                      sizeof(reqLoginPhase2.u.dencryptedNonce),
                                      DES_DECRYPT | DES_SW);
        if (DES_FAILED(decrpytion_status)) {
            continue;
        }

        hfsEncodeProtoHeader(&hdr);
        hfsEncodeCommandBuffer((char *)&reqLoginPhase2, CMD_REQ_MDS_LOGIN_PHASE2);

        ret = send(socketfd, &hdr, sizeof(hdr), 0);
        if (ret != sizeof(hdr)) {
            continue;
        }

        ret = send(socketfd, &reqLoginPhase2, sizeof(reqLoginPhase2), 0);
        if (ret != sizeof(reqLoginPhase2)) {
            continue;
        }

        // Receive phase 2 challenge //
        ret = recv(socketfd, &hdr, sizeof(hdr), 0);
        if (ret != sizeof(hdr)) {
            continue;
        }

        ret = recv(socketfd, &respLoginPhase2, sizeof(respLoginPhase2), 0);
        if (ret != sizeof(respLoginPhase2)) {
            continue;
        }

        hfsDecodeProtoHeader(&hdr);
        hfsDecodeCommandBuffer((char *)&respLoginPhase2, CMD_REQ_MDS_LOGIN_PHASE2);

        // were we authenticated //
        if (!HFS_SUCCESS(hdr.status)) {
            continue;
        }

        // Done with this socket set it back to async mode //
        status = setClientSockOpts(socketfd);
        if (!HFS_SUCCESS(status)) {
            continue;
        }

        // Ok successully authenticated/
        serverAuthenticated[i]++;
    }

    // Did we authenticate to both the primary and backup if not close sockets//
    if (!serverAuthenticated[0]) {
        DEACTIVATE_CLIENT_SIDE_SOCKET(pHfsClient, 0);
    }

    // Ingore if we cannot authenticate with the backup MDS //
    if (!serverAuthenticated[1]) {
        HFS_LOG_ERROR("Couldn't verify login with backup mds ignoring for now !");
        serverAuthenticated[1]++;
    }

    if (!serverAuthenticated[1]) {
        DEACTIVATE_CLIENT_SIDE_SOCKET(pHfsClient, 1);
    }

    if (!serverAuthenticated[0] || !serverAuthenticated[1]) {
        return HFS_STATUS_AUTH_FAILED;
    }

    return HFS_STATUS_SUCCESS;
}


void
hfsSetClientCredentials(HERCULES_CLIENT *pHfsClient,
                        char            *userName,
                        char            *encrpytedPassword)
{
    memset(pHfsClient->config.userName, 0, HFS_MAX_USER_NAME+1);
    memset(pHfsClient->config.epassword, 0, HFS_MAX_ENC_PASSWORD+1);
    memcpy(pHfsClient->config.userName, userName, HFS_MAX_USER_NAME);
    memcpy(pHfsClient->config.epassword, encrpytedPassword, HFS_MAX_ENC_PASSWORD);
}

HFS_STATUS hfsTermGetCredentials(char *userName,
                                 char *encrpytedPassword)
{
    char *pass;
    fflush(stdin);
    fflush(stdout);

    memset(userName, 0, HFS_MAX_USER_NAME+1);
    memset(encrpytedPassword, 0, HFS_MAX_ENC_PASSWORD+1);
    printf("\nHercules Parallel File System (Version 1.0.0)");
    printf("\n\nAuthenticating with Root MetaData Server %s", getHFSClientOptions()->rootMDSAddr);
    printf("\nAuthentication Method [Hercules CHAP Authentication]");
    printf("\nUser Name:");
    scanf("%8s", userName);

    pass = getpass("Password:");
    memcpy(encrpytedPassword, pass, HFS_MAX_ENC_PASSWORD);
    pass = crypt(encrpytedPassword, HFS_CRYPT_SALT);
    memcpy(encrpytedPassword, pass, HFS_MAX_ENC_PASSWORD);

    memset(pass, 0, HFS_MAX_ENC_PASSWORD+1);
    return HFS_STATUS_SUCCESS;
}

//-- Client main --//
int
main(int argc, char **argv)
{
    HFS_STATUS  status=HFS_STATUS_SUCCESS;
    PHFS_FS_CTX pfilesystemContext;
    char        userName[HFS_MAX_USER_NAME+1];
    char        epassword[HFS_MAX_ENC_PASSWORD+1];
    HFS_ENTRY(main);

    //-- Parse the command line //
    status = parse_cmd_line_options(argc,
                                    argv,
                                    getHFSClientOptions());
    if (!HFS_SUCCESS(status)) {
        goto leave;
    }

    //-- Get the credentials from the server --//
    status = hfsTermGetCredentials(userName, epassword);
    if (!HFS_SUCCESS(status)) {
        goto leave;
    }

    hfsSetClientCredentials(getHFSClient(), userName, epassword);

    //-- Initialize the logging subsystem --//
    status = loggerInit(getHFSClientOptions()->clientLogFile,
                        getHFSClientOptions()->logMask);
    if (!HFS_SUCCESS(status)) {
        goto leave;
    }

    //-- Get the context for the entire file system --//
    pfilesystemContext = getHFSFSCtx();
    HFS_UNUSED(pfilesystemContext);

    //-- Get the context of the entire file system from Root MDS --//
    status = hfsBuildFSContext_MDS(getHFSClientOptions()->rootMDSAddr,
                                   getHFSClientOptions()->rootMDSPort,
                                   getPPHFSFSCtx());
    if (!HFS_SUCCESS(status)) {
        goto leave;
    }


    //- Build up the role for this client --//
    status = hfsBuildClient(getHFSClientOptions(),
                            getHFSFSCtx(),
                            getHFSClient());
    if (!HFS_SUCCESS(status)) {
        goto leave;
    }

    //-- Fuse Init --//
    status = hfsFuseInit(2, argv);
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
