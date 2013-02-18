/*
 * utils_common.h
 *
 *     Common utility functions
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

#ifndef __UTILS_COMMON_H_
#define __UTILS_COMMON_H_

#include <pthread.h>
#include <list.h>
#include <proto_common.h>
#include <semaphore.h>


//-- Threading and Mutex and etc for the system --//
typedef pthread_mutex_t         HFS_MUTEX,*PHFS_MUTEX;
HFS_STATUS hfsMutexInit(PHFS_MUTEX);
HFS_STATUS hfsMutexDestroy(PHFS_MUTEX);
HFS_STATUS hfsMutexLock(PHFS_MUTEX);
HFS_STATUS hfsMutexUnlock(PHFS_MUTEX);

typedef sem_t       HFS_SEMAPHORE,*PHFS_SEMAPHORE;
int hfsSemInit(PHFS_SEMAPHORE pSemaPhore, int pshared, unsigned int value);
int hfsSemWait(PHFS_SEMAPHORE pSemaPhore);
int hfsSempost(PHFS_SEMAPHORE pSemaPhore);
int hfsSemGetValue(PHFS_SEMAPHORE pSemaPhore, int *sval);
int hfsSemTryWait(PHFS_SEMAPHORE pSemaPhore);
int hfsSemDestroy(PHFS_SEMAPHORE pSemaPhore);

void * hfsCalloc(size_t size);
void   hfsFree(void *);


//-- Queuing function --//
typedef enum __QUEUE_STATE {
    QUEUE_STATE_UNKNOWN,
    QUEUE_STATE_STARTED,
    QUEUE_STATE_PAUSED,
    QUEUE_STATE_STOPPED
}QUEUE_STATE;

typedef struct _HFS_QUEUE_ITEM {
    struct list_head    listHead;
    HFS_SEMAPHORE       completionSem;
    int                 totalMemory;
    int                 clientIdx;
}HFS_QUEUE_ITEM,*PHFS_QUEUE_ITEM;

typedef HFS_STATUS (*QPPROCESSING_FN)(PHFS_QUEUE_ITEM,HFS_STATUS);
#define MAX_THREADS_PER_QUEUE   10
typedef struct __HFS_QUEUE_PROCESSOR {
    //-- QueState --//
    QUEUE_STATE     state;
    //-- semaphore for threads to wait on --//
    HFS_SEMAPHORE   semPendingItems;
    //-- Lock for insertion and deletion --//
    HFS_MUTEX       queueLock; 
    //-- List head                       --//
    struct list_head lstAnchorPendingCmds;
    //-- Processing Function             --//
    QPPROCESSING_FN process;
    //-- int thread count               --//
    int             thread_count;
    pthread_t       thread_ids[MAX_THREADS_PER_QUEUE];
} HFS_QUEUE_PROCESSOR, *PHFS_QUEUE_PROCESSOR;

HFS_STATUS hfsQueueAlloc(PHFS_QUEUE_PROCESSOR *ppqprocessor);
HFS_STATUS hfsQueuehfsFree(PHFS_QUEUE_PROCESSOR   pqueueprocessor);
HFS_STATUS hfsQueueInit(PHFS_QUEUE_PROCESSOR   pqueueprocessor,QPPROCESSING_FN,int);
HFS_STATUS hfsQueueDrain(PHFS_QUEUE_PROCESSOR   pqueueprocessor);
HFS_STATUS hfsQueueSetState(PHFS_QUEUE_PROCESSOR   pqueueprocessor,QUEUE_STATE);
QUEUE_STATE hfsQueueGetState(PHFS_QUEUE_PROCESSOR   pqueueprocessor);
HFS_STATUS hfsQueueItem(PHFS_QUEUE_PROCESSOR,PHFS_QUEUE_ITEM);


#define MAX_FILE_SYSTEM_NAME 255
typedef struct __HFS_FS_CTX {
    char                fsName[MAX_FILE_SYSTEM_NAME];
    //- Sever name MDS and DS -//
    int                 totalServer;
    int                 stripeSize;
    int                 mdsCount;
    PHFS_SERVER_INFO    pMdsServerInfo;
    int                 dsCount;
    PHFS_SERVER_INFO    pDsServerInfo;

    // Variables length server list// MDS Servers followed by DS servers
    HFS_SERVER_INFO     server[1];
    HFS_SERVER_INFO         mdserver[HFS_MAX_MDS_CNT];
    HFS_SERVER_INFO         dserver[HFS_MAX_DS_CNT];

}HFS_FS_CTX,*PHFS_FS_CTX;


#define SERVER_CONFIG_ROLE_MDS      0
#define SERVER_CONFIG_ROLE_DS       1
#define SERVER_CONFIG_ROLE_MAX      3
HFS_STATUS          hfsBuildFSContext(char *configFileName,PHFS_FS_CTX *ppFsCtx);
HFS_STATUS          hfsBuildFSContext_MDS(char *rootMDSAddr, char *rootMDSPort, PHFS_FS_CTX *ppFsCtx);
PHFS_SERVER_INFO    hfsGetMDSFromServIdx(PHFS_FS_CTX,int);
PHFS_SERVER_INFO    hfsGetDSFromServIdx (PHFS_FS_CTX,int);
PHFS_SERVER_INFO    hfsGetServerFromName(PHFS_FS_CTX,char *name,
                                         int *serverRole);

//-- Miscellany --//
#define ISNULL(X)   ((NULL)==X)
#define MAX(A,B)    ((A)>(B)?(A):(B))
#define MIN(A,B)    ((A)<(B)?(A):(B))
#endif //__UTILS_COMMON_H_
