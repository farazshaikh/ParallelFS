/*
 * utils_config.c
 *
 * Simple text config parser for filesystem configuration
 *
 * utility functions
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
#include "hercules_common.h"


#define NON_SERVER_LINES 4
#define FS_NAME "Hercules_FS"
#define STRIPE_SIZE 8192
#define MDS_FIRST_IDX 0
#define DS_FIRST_IDX 0

#define NAME_EQUALS(name1, name2) (!strcmp(name1, name2))
#define IS_MDS(line) ( !strcmp(line, "<MDS>\n") )
#define IS_DS(line) ( !strcmp(line, "<DS>\n") )

#define SET_HEADER(pHeader, cmd) do {                   \
        memset((pHeader), 0, sizeof(HFS_PROTO_HEADER)); \
        (pHeader)->command = (cmd);                     \
        (pHeader)->protoMagic = HFS_PROTO_MAGIC;        \
        (pHeader)->status = HFS_STATUS_SUCCESS;         \
    }while(0)



static HFS_STATUS getServerCount(FILE * configFile, int * serverCnt);
static HFS_STATUS readMDS(FILE * configFile, PHFS_FS_CTX pFsCtx);
static HFS_STATUS readDS(FILE * configFile, PHFS_FS_CTX pFsCtx);


HFS_STATUS
hfsBuildFSContext(char *configFileName,
                  PHFS_FS_CTX *ppFsCtx)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    FILE * configFile = NULL;
    int serverCnt = 0;
    int cnt = 0;
    PHFS_FS_CTX pFsCtx = NULL;
    size_t ctxSize = 0;
    int retval = 0;
    char *line=NULL;
    size_t len=0;


    do {
        //-- Open the file in read mode --//
        configFile = fopen (configFileName, "r");
        if (configFile == NULL) {
            HFS_LOG_ERROR("Cannot open config file %s", configFileName);
            status = HFS_HOST_OS_ERROR;
            break;
        }

        //-- Allocate a context structure after calculating its size --//
        status = getServerCount (configFile, &serverCnt);
        if (!HFS_SUCCESS(status)) {
            HFS_LOG_ERROR("Cannot parse config file %s", configFileName);
            break;
        }

        if (serverCnt > 0) {
            cnt = serverCnt - 1;
        }

        ctxSize = sizeof(HFS_FS_CTX) + cnt * sizeof(HFS_SERVER_INFO);
        pFsCtx = hfsCalloc (ctxSize);
        if (pFsCtx == NULL) {
            status = HFS_STATUS_OUT_OF_MEMORY;
            break;
        }

        //-- Seek to start of file --//
        retval = fseek (configFile, 0, SEEK_SET);
        if (retval < 0) {
            status = HFS_HOST_OS_ERROR;
            break;
        }

        //-- Read the config file and fill in the structure --//
        strncpy (pFsCtx->fsName, FS_NAME, MAX_FILE_SYSTEM_NAME);
        pFsCtx->stripeSize = STRIPE_SIZE;
        pFsCtx->totalServer = serverCnt;
        retval = getline (&line, &len, configFile);
        if (retval == -1 || !IS_MDS(line)) {
            status = HFS_STATUS_CONFIG_ERROR;
            break;
        }
        status = readMDS (configFile, pFsCtx);
        if (!HFS_SUCCESS(status))
            break;
        retval = getline (&line, &len, configFile);
        if (retval == -1 || !IS_DS(line)) {
            status = HFS_STATUS_CONFIG_ERROR;
            break;
        }
        status = readDS (configFile, pFsCtx);
        if (!HFS_SUCCESS(status)) {
            break;
        }

        //-- Successfully done reading the config file --//
        *ppFsCtx = pFsCtx;
        status = HFS_STATUS_SUCCESS;
    } while (0);

    if (!HFS_SUCCESS(status) && pFsCtx != NULL) {
        free (pFsCtx);
    }

    return status;
}

PHFS_SERVER_INFO
hfsGetMDSFromServIdx(PHFS_FS_CTX pFsCtx,
                     int idx)
{
    PHFS_SERVER_INFO pServInfo = NULL;

#ifdef PARAM_CHECKING
    if (pFsCtx == NULL) {
        // LOG HFS_STATUS_INVALID_PARAM_1
        return NULL;
    }
    if (idx == 0) {
        // LOG HFS_STATUS_INVALID_PARAM_2
        return NULL;
    }
#endif

    do {
        //-- Check whether the required mds exists --//
        if (idx > pFsCtx->mdsCount) {
            pServInfo = NULL;
            break;
        }

        //-- Get the info for the required MDS server --//
        pServInfo = pFsCtx->pMdsServerInfo + (idx - MDS_FIRST_IDX);
    } while (0);

    return pServInfo;
}

PHFS_SERVER_INFO
hfsGetDSFromServIdx(PHFS_FS_CTX pFsCtx,
                    int idx)
{
    PHFS_SERVER_INFO pServInfo = NULL;
#ifdef PARAM_CHECKING
    if (pFsCtx == NULL) {
        // LOG HFS_STATUS_INVALID_PARAM_1
        return NULL;
    }
    if (idx == 0) {
        // LOG HFS_STATUS_INVALID_PARAM_2
        return NULL;
    }
#endif

    do {
        //-- Check whether the required ds exists --//
        if (idx > pFsCtx->dsCount) {
            pServInfo = NULL;
            break;
        }

        //-- Get the info for the required DS server --//
        pServInfo = pFsCtx->pDsServerInfo + (idx - DS_FIRST_IDX);
    } while(0);

    return pServInfo;
}

PHFS_SERVER_INFO
hfsGetServerFromName(PHFS_FS_CTX pFsCtx,
                     char *name,
                     int *serverRole)
{
    PHFS_SERVER_INFO pServInfo = NULL;
    int i = 0;

#ifdef PARAM_CHECKING
    if (pFsCtx == NULL) {
        // LOG HFS_STATUS_INVALID_PARAM_1
        return NULL;
    }
    if (name == NULL) {
        // LOG HFS_STATUS_INVALID_PARAM_2
        return NULL;
    }
    if (serverRole == NULL) {
        // LOG HFS_STATUS_INVALID_PARAM_3
        return NULL;
    }
#endif

    do {
        //-- Scan for the server in the MDS list --//
        pServInfo = pFsCtx->pMdsServerInfo;
        *serverRole = SERVER_CONFIG_ROLE_MDS;
        for (i = MDS_FIRST_IDX; i <= (pFsCtx->mdsCount - (1 - MDS_FIRST_IDX)); i++) {
            if (NAME_EQUALS(name, pServInfo->hostName)) {
                break;
            }
            pServInfo++;
        }
        if (NAME_EQUALS(name, pServInfo->hostName)) {
            break;
        }

        //-- Scan for the server in the DS list --//
        pServInfo = pFsCtx->pDsServerInfo;
        *serverRole = SERVER_CONFIG_ROLE_DS;
        for (i = DS_FIRST_IDX; i <= (pFsCtx->dsCount - (1 - DS_FIRST_IDX)); i++) {
            if (NAME_EQUALS(name, pServInfo->hostName)) {
                break;
            }
            pServInfo++;
        }
        if (NAME_EQUALS(name, pServInfo->hostName)) {
            break;
        }

        //-- Server not found --//
        *serverRole = SERVER_CONFIG_ROLE_MAX;
        pServInfo = NULL;
    } while(0);

    return pServInfo;
}


static HFS_STATUS
getServerCount(FILE * configFile,
               int * serverCnt)
{
    char * line = NULL;
    size_t len = 0;
    int retval = 0;
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    int lineCnt = 0;


#ifdef PARAM_CHECKING
    if (configFile == NULL) {
        return HFS_STATUS_INVALID_PARAM_1;
    }
    if (serverCnt == NULL) {
        return HFS_STATUS_INVALID_PARAM_2;
    }
#endif

    do {
        //-- Seek to start of file --//
        retval = fseek (configFile, 0, SEEK_SET);
        if (retval < 0) {
            status = HFS_HOST_OS_ERROR;
        }

        //-- Count the number of lines in the file --//
        lineCnt = 0;
        retval = getline (&line, &len, configFile);
        while (retval != -1) {
            lineCnt++;
            retval = getline (&line, &len, configFile);
        }

        //-- Subtract non-server lines to get server count --//
        *serverCnt = lineCnt - NON_SERVER_LINES;
        status = HFS_STATUS_SUCCESS;
    } while (0);

    if (line) {
        hfsFree(line);
    }

    return status;
}

static HFS_STATUS
readMDS(FILE * configFile,
        PHFS_FS_CTX pFsCtx)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    int retval = 0;
    int i = 0;
    PHFS_SERVER_INFO pServInfo = NULL;
    char *line=NULL;
    size_t len=0;

#ifdef PARAM_CHECKING
    if (configFile == NULL) {
        return HFS_STATUS_INVALID_PARAM_1;
    }
    if (pFsCtx == NULL) {
        return HFS_STATUS_INVALID_PARAM_2;
    }
#endif

    do {
        //-- Read the mds servers count --//
        retval = getline (&line, &len, configFile);
        if (retval < 0) {
            status = HFS_STATUS_CONFIG_ERROR;
            break;
        }
        retval = sscanf (line, "mdscount=%d", &pFsCtx->mdsCount);
        if (retval != 1) {
            status = HFS_STATUS_CONFIG_ERROR;
            break;
        }

        //-- Read all the servers info --//
        pServInfo = &pFsCtx->server[0];
        pFsCtx->pMdsServerInfo = pServInfo;
        for (i=0; i<pFsCtx->mdsCount; i++) {
            retval = getline (&line, &len, configFile);
            if (retval < 0) {
                status = HFS_STATUS_CONFIG_ERROR;
                break;
            }
            retval = sscanf (line, "%s %s %s %s %s %s", pServInfo->hostName,
                             pServInfo->ipAddr, pServInfo->port, pServInfo->proto,
                             pServInfo->dataStorePath, pServInfo->logFile);
            if (retval != 6) {
                status = HFS_STATUS_CONFIG_ERROR;
                break;
            }
            pServInfo->role = SERVER_CONFIG_ROLE_MDS;
            pServInfo->serverIdx = i + MDS_FIRST_IDX;
            pServInfo++;
        }

        if (!HFS_SUCCESS(status)) {
            break;
        }

        status = HFS_STATUS_SUCCESS;
    } while (0);

    if (line) {
        hfsFree(line);
    }

    return status;
}

static HFS_STATUS
readDS(FILE * configFile, PHFS_FS_CTX pFsCtx)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    int retval = 0;
    int i = 0;
    PHFS_SERVER_INFO pServInfo = NULL;
    char *line=NULL;
    size_t len=0;

#ifdef PARAM_CHECKING
    if (configFile == NULL) {
        return HFS_STATUS_INVALID_PARAM_1;
    }

    if (pFsCtx == NULL) {
        return HFS_STATUS_INVALID_PARAM_2;
    }
#endif

    do {
        //-- Read the ds servers count --//
        retval = getline (&line, &len, configFile);
        if (retval < 0) {
            status = HFS_STATUS_CONFIG_ERROR;
            break;
        }
        retval = sscanf (line, "dscount=%d", &pFsCtx->dsCount);
        if (retval != 1) {
            status = HFS_STATUS_CONFIG_ERROR;
            break;
        }

        //-- Read all the servers info --//
        pServInfo = &pFsCtx->server[0];
        pServInfo += pFsCtx->mdsCount;
        pFsCtx->pDsServerInfo = pServInfo;
        for (i=0; i<pFsCtx->dsCount; i++) {
            retval = getline (&line, &len, configFile);
            if (retval < 0) {
                status = HFS_STATUS_CONFIG_ERROR;
                break;
            }
            retval = sscanf (line, "%s %s %s %s %s %s", pServInfo->hostName,
                             pServInfo->ipAddr, pServInfo->port, pServInfo->proto,
                             pServInfo->dataStorePath, pServInfo->logFile);
            if (retval != 6) {
                status = HFS_STATUS_CONFIG_ERROR;
                break;
            }
            pServInfo->role = SERVER_CONFIG_ROLE_DS;
            pServInfo->serverIdx = i + DS_FIRST_IDX;
            pServInfo++;
        }

        if (!HFS_SUCCESS(status)) {
            break;
        }

        status = HFS_STATUS_SUCCESS;
    } while (0);

    if (line) {
        hfsFree(line);
    }

    return status;
}

PHFS_FS_CTX
hfsReadFSConfigFromServer(int sockfd,
                          int extentSize)
{
    PHFS_SERVER_REQ pServReq;
    PHFS_SERVER_RESP pServResp;
    PHFS_SERVER_INFO pServInfo;
    PHFS_FS_CTX pFsCtx;
    int returnedents, sendSize;
    int i;
    HFS_STATUS status;
    PHFS_QUEUE_ITEM pQueueItem;
    HFS_PROTO_HEADER hdr;
    int bytesRecieved=0;
    int ret;
    char *extentPtr;

    //- Allocate the Req -//
    SET_HEADER(&hdr, CMD_REQ_MDS_GET_CONFIG);
    pQueueItem = hfsAllocateProtoBuff(&hdr, extentSize);
    if (!pQueueItem) {
        HFS_LOG_ERROR("Out of memory");
        goto cleanup;
    }
    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);
    pServResp = (PHFS_SERVER_RESP)(pQueueItem + 1);


    //-- Allocate a context structure after calculating its size --//
    pFsCtx = hfsCalloc(sizeof(HFS_FS_CTX));
    if (!pFsCtx) {
        status = HFS_STATUS_OUT_OF_MEMORY;
        goto cleanup;
    }

    //-- Read the config entries from MDS and fill in the structure --//
    strncpy(pFsCtx->fsName, FS_NAME, MAX_FILE_SYSTEM_NAME);
    pFsCtx->stripeSize = extentSize;
    pFsCtx->pMdsServerInfo = pFsCtx->mdserver;
    pFsCtx->pDsServerInfo = pFsCtx->dserver;

    do
        {
            // Make the packet //
            SET_HEADER(&pServReq->hdr, CMD_REQ_MDS_GET_CONFIG);
            sendSize = sizeof(HFS_PROTO_HEADER);
            sendSize += hfsGetCommandSize(&pServReq->hdr);

            pServReq->req.reqMDSGetConfig.srvrentOffset = pFsCtx->mdsCount + pFsCtx->dsCount;
            status = hfsEncodeCommandBuffer((char *)&pServReq->req, pServReq->hdr.command);
            if (!HFS_SUCCESS(status))
                break;
            hfsEncodeProtoHeader(&pServReq->hdr);

            //- Send the packet -//
            if (-1 == send(sockfd, pServReq, sendSize , 0))
                {
                    HFS_LOG_ERROR("Send Failed with errno %d\n", errno);
                    goto cleanup;
                }

            // Receive the header //
            ret = recv(sockfd, (char*)&(pServResp->hdr), sizeof(HFS_PROTO_HEADER), 0);
            if (-1 == ret)
                {
                    HFS_LOG_ERROR("Recv failed wth errno %d\n", errno);
                    goto cleanup;
                }

            // Receive the command //
            ret = recv(sockfd, (char*)&(pServResp->resp.respMDSFsConfChanged), sizeof(RESP_MDS_FS_CONF_CHANGED), 0);
            if (-1 == ret)
                {
                    HFS_LOG_ERROR("Recv failed wth errno %d\n", errno);
                    goto cleanup;
                }

            hfsDecodeProtoHeader(&pServResp->hdr);
            if (!HFS_SUCCESS( pServResp->hdr.status))
                {
                    HFS_LOG_ERROR("Invalid HFS Header received by client\n");
                    goto cleanup;
                }

            status = hfsDecodeCommandBuffer((char *)&pServResp->resp, pServResp->hdr.command);
            if (!HFS_SUCCESS(status))
                goto cleanup;


            // Receive the entries only as much specified in hdr->pduExtentSize//
            extentPtr = getExtentPointer(pQueueItem);
            bytesRecieved = 0;
            do {
                if (!pServResp->hdr.pduExtentSize) {
                    break;
                }

                ret = recv(sockfd, extentPtr + bytesRecieved, pServResp->hdr.pduExtentSize - bytesRecieved, 0);
                if (-1 == ret) {
                    HFS_LOG_ERROR("Recv failed wth errno %d\n", errno);
                    goto cleanup;
                }
                bytesRecieved += ret;
            }while(bytesRecieved < pServResp->hdr.pduExtentSize);


            returnedents = pServResp->resp.respMDSGetConfig.srvrentCount;
            // Copy in the stuff from the PDU into the local FSCTX
            pServInfo = (PHFS_SERVER_INFO) getExtentPointer(pQueueItem);
            for(i = 0; i < returnedents; i++) {
                pServInfo[i].serverIdx = ntohl(pServInfo[i].serverIdx);
                pServInfo[i].role = ntohl(pServInfo[i].role);

                if (pServInfo[i].role == SERVER_CONFIG_ROLE_MDS) {
                    memcpy(pFsCtx->mdserver + pFsCtx->mdsCount++, pServInfo+i, sizeof(HFS_SERVER_INFO));
                } else if (pServInfo[i].role == SERVER_CONFIG_ROLE_DS) {
                        memcpy(pFsCtx->dserver + pFsCtx->dsCount++, pServInfo+i, sizeof(HFS_SERVER_INFO));
                } else {
                    HFS_LOG_ERROR("Incorrect Server Role received from Root MDS");
                    goto cleanup;
                }
            }
        }while((returnedents == extentSize / sizeof(HFS_SERVER_INFO)) && returnedents);
    pFsCtx->totalServer = pFsCtx->mdsCount + pFsCtx->dsCount;
    goto doneOk;


 cleanup:
    if (pFsCtx) {
        hfsFree(pFsCtx);
        pFsCtx = NULL;
    }

 doneOk:
    hfsFreeProtoBuff(pQueueItem);
    return pFsCtx;
}

int
hfsGetFSExtentSize(int sockfd)
{
    PHFS_SERVER_REQ pServReq;
    PHFS_SERVER_RESP pServResp;
    int ret;
    HFS_STATUS status;
    int extentSize=0;
    int bytesRecieved = 0;

    // - Get the extent size ---//
    //- Build the packet ---//
    pServResp =
        (PHFS_SERVER_RESP)hfsCalloc(sizeof(HFS_PROTO_HEADER) + MAX(sizeof(RESP_MDS_GET_EXTENT_SIZE), sizeof(REQ_MDS_GET_EXTENT_SIZE)) );
    pServReq = (PHFS_SERVER_REQ) pServResp;
    if (!pServReq)
        {
            pServResp = NULL;
            pServReq = NULL;
            HFS_LOG_ERROR("Out of memory");
            goto cleanup;
        }

    SET_HEADER(&pServReq->hdr, CMD_REQ_MDS_GET_EXTENT_SIZE);
    hfsEncodeProtoHeader(&pServReq->hdr);
    //- Send the packet out -//
    ret = send(sockfd, pServReq, sizeof(HFS_PROTO_HEADER) + sizeof(REQ_MDS_GET_EXTENT_SIZE), 0);
    if (ret != sizeof(HFS_PROTO_HEADER) + sizeof(REQ_MDS_GET_EXTENT_SIZE))
        {
            HFS_LOG_ERROR("Send Failed with errno %d\n", errno);
            goto cleanup;
        }


    //- Get the response -//
    bytesRecieved = 0;
    do {
        ret = recv(sockfd, (char *)pServResp + bytesRecieved, sizeof(HFS_PROTO_HEADER) + sizeof(RESP_MDS_GET_EXTENT_SIZE) - bytesRecieved, 0);
        if (-1 == ret)
            {
                HFS_LOG_ERROR("Recv failed wth errno %d\n", errno);
                goto cleanup;
            }
        bytesRecieved += ret;
    }while(bytesRecieved < (sizeof(HFS_PROTO_HEADER) + sizeof(RESP_MDS_GET_EXTENT_SIZE)));

    hfsDecodeProtoHeader(&pServResp->hdr);
    if (HFS_PROTO_MAGIC != pServResp->hdr.protoMagic || !HFS_SUCCESS(pServResp->hdr.status))
        {
            HFS_LOG_ERROR("Invalid HFS Header received by client\n");
            goto cleanup;
        }

    status = hfsDecodeCommandBuffer((char *)&pServResp->resp, pServResp->hdr.command);
    if (!HFS_SUCCESS(status))
        {
            HFS_LOG_ERROR("Invalid HFS Header received by client\n");
            goto cleanup;
        }

    extentSize = pServResp->resp.respMDSGetExtentSize.extent_size;

 cleanup:
    if (pServReq) {
        hfsFree(pServReq);
    }

    return extentSize;
}

// Build FS Context from MDS
HFS_STATUS
hfsBuildFSContext_MDS(char *rootMDSAddr,
                      char *rootMDSPort,
                      PHFS_FS_CTX *ppFsCtx)
{
    HFS_STATUS status = HFS_STATUS_CONFIG_ERROR;
    int extentSize = 0;
    int sockfd = -1;
    int ret;
    struct sockaddr_in server_addr;

    if (-1 == (sockfd = socket(AF_INET, SOCK_STREAM, 6))) {
        HFS_LOG_ERROR("Socket Creation Failed\n");
        return status;
    }

    //- Connect to the server ----------------------//
    HFS_LOG_INFO("Connecting to Root MDS %s[%s]\n", rootMDSAddr, rootMDSPort);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(rootMDSPort));
    inet_aton(rootMDSAddr, &server_addr.sin_addr);

    ret = connect(sockfd,
                  (struct sockaddr *)&server_addr,
                  sizeof(struct sockaddr));
    if (-1 == ret) {
        HFS_LOG_ERROR("Connect to Root MDS Server %s failed with errno %d", rootMDSAddr, errno);
        goto cleanup;
    }
    HFS_LOG_INFO("Root MDS Server %s Connection Successful\n", rootMDSAddr);

    extentSize = hfsGetFSExtentSize(sockfd);
    if (!extentSize) {
        goto cleanup;
    }

    // Get the FS configuration //
    *ppFsCtx = hfsReadFSConfigFromServer(sockfd, extentSize);
    if (*ppFsCtx) {
        status = HFS_STATUS_SUCCESS;
    }

    HFS_LOG_INFO("got fsctx %p", *ppFsCtx);

 cleanup:
    close(sockfd);
    return status;
}
