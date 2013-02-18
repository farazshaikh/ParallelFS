/*
 * utils_common.c
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

#include <hercules_common.h>

// - Threading and mutex --//
HFS_STATUS
hfsMutexInit(PHFS_MUTEX pHfsMutex)
{
    HFS_ENTRY(hfsMutexInit);
    HFS_LEAVE(hfsMutexInit);
    return ((HFS_STATUS)pthread_mutex_init(pHfsMutex, NULL));
}

HFS_STATUS
hfsMutexDestroy(PHFS_MUTEX pHfsMutex)
{
    HFS_ENTRY(hfsMutexDestroy);
    HFS_LEAVE(hfsMutexDestroy);
    return ((HFS_STATUS)pthread_mutex_destroy(pHfsMutex));
}

HFS_STATUS
hfsMutexLock(PHFS_MUTEX pHfsMutex)
{
    HFS_ENTRY(hfsMutexLock);
    HFS_LEAVE(hfsMutexLock);
    return ((HFS_STATUS)pthread_mutex_lock(pHfsMutex));
}

HFS_STATUS
hfsMutexUnlock(PHFS_MUTEX pHfsMutex)
{
    HFS_ENTRY(hfsMutexUnlock);
    HFS_LEAVE(hfsMutexUnlock);
    return ((HFS_STATUS)pthread_mutex_unlock(pHfsMutex));
}

//-- Semaphores --//
int
hfsSemInit(PHFS_SEMAPHORE pHfsSemaPhore, int pshared, unsigned int value)
{
    HFS_ENTRY(hfsSemInit);
    HFS_LEAVE(hfsSemInit);
    HFS_LOG_INFO("%d", sem_init(pHfsSemaPhore, pshared, value));
    HFS_LOG_INFO("%d", errno);
    return 0;
}

int
hfsSemWait(PHFS_SEMAPHORE pHfsSemaPhore)
{
    int ret;
    HFS_ENTRY(hfsSemWait);
 loop:
    ret = sem_wait(pHfsSemaPhore);
    if (-1 == ret && errno == EINTR) goto loop;
    HFS_LOG_INFO("%d", errno);
    HFS_LEAVE(hfsSemWait);
    return 0;
}

int
hfsSempost(PHFS_SEMAPHORE pHfsSemaPhore)
{
    HFS_ENTRY(hfsSempost);
    HFS_LEAVE(hfsSempost);
    return sem_post(pHfsSemaPhore);
}

int
hfsSemGetValue(PHFS_SEMAPHORE pHfsSemaPhore, int *sval)
{
    HFS_ENTRY(hfsSemGetValue);
    HFS_LEAVE(hfsSemGetValue);
    return sem_getvalue(pHfsSemaPhore, sval);
}

int
hfsSemTryWait(PHFS_SEMAPHORE pHfsSemaPhore)
{
    HFS_ENTRY(hfsSemTryWait);
    HFS_LEAVE(hfsSemTryWait);
    return hfsSemTryWait(pHfsSemaPhore);
}

int
hfsSemDestroy(PHFS_SEMAPHORE pHfsSemaPhore)
{
    HFS_ENTRY(hfsSemDestroy);
    HFS_LEAVE(hfsSemDestroy);
    return sem_destroy(pHfsSemaPhore);
}

//- Memory Allocation for HFS -//
void *
hfsCalloc(size_t size)
{
    HFS_ENTRY();
    return calloc(size, (size_t)1);
}

void
hfsFree(void *p)
{
    HFS_ENTRY();
    if (p) {
        free(p);
    }
    HFS_LEAVE();
}

//-- Queuing  --//
void *
CommonQueueDispatch(void *queueContext)
{
    HFS_STATUS  ret;
    PHFS_QUEUE_PROCESSOR pQProcessor;
    PHFS_QUEUE_ITEM  pDeQueuedItem;
    struct list_head *pDeQueuedItemlh;

    HFS_ENTRY(QueueDispatch);
    pQProcessor = (PHFS_QUEUE_PROCESSOR)queueContext;


    if (NULL == pQProcessor || NULL == pQProcessor->process) {
        HFS_LOG_ERROR("Queue thread cannot be started invalid context passed");
        ret = HFS_INTERNAL_ERROR;
        goto leave;
    }

    HFS_LOG_INFO("Starting dispatch thread for Q %p", pQProcessor);
    while (QUEUE_STATE_STOPPED!=pQProcessor->state)
        {
            hfsSemWait(&pQProcessor->semPendingItems);
            // Ok We should now have at least one item to process //

            // Lock the queue
            ret = hfsMutexLock(&pQProcessor->queueLock);
            if (!HFS_SUCCESS(ret)) {
                HFS_LOG_ERROR("Processing thread cannot acquire mutex for q");
                ret = HFS_STATUS_LOCKING_ERROR;
                goto leave;
            }

            // Remove the element
            if (list_empty(&pQProcessor->lstAnchorPendingCmds))
                {
                    HFS_LOG_ERROR("Processing Thread Smells Fish for %p or last nudge",
                                  pQProcessor);
                    ret = hfsMutexUnlock(&pQProcessor->queueLock);
                    if (!HFS_SUCCESS(ret)) {
                        HFS_LOG_ERROR("Processing thread cannot Release mutex for q");
                        ret = HFS_STATUS_LOCKING_ERROR;
                        goto leave;
                    }
                    continue;
                }else {
                pDeQueuedItemlh = NULL;
                pDeQueuedItemlh = pQProcessor->lstAnchorPendingCmds.next;
                list_del_init(pDeQueuedItemlh);
                pDeQueuedItem = list_entry(pDeQueuedItemlh, HFS_QUEUE_ITEM, listHead);
            }

            // Unlock the queue
            ret = hfsMutexUnlock(&pQProcessor->queueLock);
            if (!HFS_SUCCESS(ret)) {
                HFS_LOG_ERROR("Processing thread cannot acquire mutex for q");
                ret = HFS_STATUS_LOCKING_ERROR;
                goto leave;
            }

            // Process the element
            ret = pQProcessor->process(pDeQueuedItem, HFS_STATUS_SUCCESS);
            if (!HFS_SUCCESS(ret)) {
                HFS_LOG_ERROR("Processing has failed on queue item %p", pDeQueuedItem);
                HFS_LOG_ERROR("Some client will face music");
            }
        }

    ret = HFS_STATUS_SUCCESS;
 leave:
    HFS_LOG_INFO("Exiting dispatch thread for Q %p", pQProcessor);
    HFS_LEAVE(QueueDispatch);
    return ((void *) ret);
}




HFS_STATUS
hfsQueueAlloc(PHFS_QUEUE_PROCESSOR *ppQueueProcessor) {
    PHFS_QUEUE_PROCESSOR pHFSQue;
    *ppQueueProcessor = NULL;
    pHFSQue = (PHFS_QUEUE_PROCESSOR)hfsCalloc(sizeof(*pHFSQue));
    if (!pHFSQue) {
        HFS_LOG_ERROR("Cannot Allocate Queue");
        return HFS_STATUS_OUT_OF_MEMORY;
    }
    *ppQueueProcessor = pHFSQue;
    return HFS_STATUS_SUCCESS;
}

HFS_STATUS
hfsQueuehfsFree(PHFS_QUEUE_PROCESSOR pQueueProcessor) {
    hfsFree(pQueueProcessor);
    return HFS_STATUS_SUCCESS;
}

HFS_STATUS
hfsQueueInit(PHFS_QUEUE_PROCESSOR pQueueProcessor,
             QPPROCESSING_FN processingFn,
             int   threadCount)
{
    int i;
    int ret;

    pQueueProcessor->state = QUEUE_STATE_UNKNOWN;
    hfsSemInit(&pQueueProcessor->semPendingItems, 1, 0);
    hfsMutexInit(&pQueueProcessor->queueLock);
    INIT_LIST_HEAD(&pQueueProcessor->lstAnchorPendingCmds);
    pQueueProcessor->process = processingFn;

    for (i = 0 ; i < threadCount && i < MAX_THREADS_PER_QUEUE ; i++) {
        ret = pthread_create(&pQueueProcessor->thread_ids[i], NULL, CommonQueueDispatch, pQueueProcessor);
        if (ret) {
            return HFS_STATUS_QUEUE_NOT_STARTED;
        }
    }
    return HFS_STATUS_SUCCESS;
}


#define QUEUE_DRAIN_RETRY_TIMEOUT 500
HFS_STATUS
hfsQueueDrain(PHFS_QUEUE_PROCESSOR pQueueProcessor)
{
    HFS_STATUS ret;
    HFS_ENTRY(hfsQueueDrain);
    if (QUEUE_STATE_STARTED == hfsQueueGetState(pQueueProcessor)) {
        HFS_LOG_ERROR("Cannot drain the queue in Started State %p",
                      pQueueProcessor);
        ret = HFS_INTERNAL_ERROR;
        goto leave;
    }

    while (1) {
        int sval;
        ret = hfsSemGetValue(&pQueueProcessor->semPendingItems, &sval);
        if (ret) {
            HFS_LOG_ERROR("Cannot Get value for semaphore");
            ret = HFS_INTERNAL_ERROR;
            goto leave;
        }

        if (0 == sval) {
            ret = HFS_STATUS_SUCCESS;
            goto leave;
        }
        usleep(QUEUE_DRAIN_RETRY_TIMEOUT);
    }
 leave:
    HFS_LEAVE(hfsQueueDrain);
    return ret;
}

HFS_STATUS
hfsQueueSetState(PHFS_QUEUE_PROCESSOR pQueueProcessor, QUEUE_STATE p)
{
    HFS_STATUS ret=HFS_STATUS_SUCCESS;
    HFS_ENTRY(hfsQueueSetState);
    HFS_LEAVE(hfsQueueSetState);
    //-TODO error checking -//
    hfsMutexLock(&pQueueProcessor->queueLock);
    pQueueProcessor->state = p;
    hfsMutexUnlock(&pQueueProcessor->queueLock);
    return ret;
}

QUEUE_STATE
hfsQueueGetState(PHFS_QUEUE_PROCESSOR pQueueProcessor)
{
    HFS_ENTRY(hfsQueueGetState);
    HFS_LEAVE(hfsQueueGetState);
    return pQueueProcessor->state;
}


HFS_STATUS
hfsQueueItem(PHFS_QUEUE_PROCESSOR pQueueProcessor,
             PHFS_QUEUE_ITEM  pQueueItem)
{
    HFS_STATUS ret=HFS_STATUS_SUCCESS;

    HFS_ENTRY(hfsQueueItem);
    if (ISNULL(pQueueItem) || ISNULL(pQueueProcessor)) {
        ret = HFS_STATUS_PRE_CONDITION_FAILS;
        goto leave;
    }

    if (!list_empty(&pQueueItem->listHead)) {
        HFS_LOG_ERROR("Item %p is alread on some list",
                      pQueueItem);
        ret = HFS_STATUS_PRE_CONDITION_FAILS;
        goto leave;
    }

    if (QUEUE_STATE_STARTED!=hfsQueueGetState(pQueueProcessor)) {
        HFS_LOG_ERROR("Queue Not started cannot insert elements now %p",
                      pQueueProcessor);
        ret = HFS_STATUS_QUEUE_NOT_STARTED;
        goto leave;
    }

    //- Lock the Queue -//
    ret = hfsMutexLock(&pQueueProcessor->queueLock);
    if (!HFS_SUCCESS(ret)) {
        ret = HFS_STATUS_LOCKING_ERROR;
        goto leave;
    }
    //- Insert the element -//
    list_add_tail(&pQueueItem->listHead, &pQueueProcessor->lstAnchorPendingCmds);
    ret = hfsSempost(&pQueueProcessor->semPendingItems);
    if (ret) {
        HFS_LOG_ERROR("Queue threads cannot be woken up");
        ret = HFS_INTERNAL_ERROR;
        goto removeItem;
    }

    ret = HFS_STATUS_SUCCESS;
    goto unlock;


    //- Cleanups -//
 removeItem:
    list_del_init(&pQueueItem->listHead);
 unlock:
    //- Unlock the Queue -//
    hfsMutexUnlock(&pQueueProcessor->queueLock);
 leave:
    HFS_LEAVE(hfsQueueItem);
    return ret;
}
