/*
 * hfs_proto_buff.c
 *
 * Generic request response protocol buffers
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
#include <netinet/in.h>

#define GET_BUFF_SIZE(CMD) \
    else if ((unsigned)CMD_REQ##CMD == (unsigned)header->command || (unsigned)CMD_RESP##CMD == header->command) { \
        bufferSize += MAX(sizeof(REQ##CMD),sizeof(RESP##CMD));          \
        if(pduHasPayLoad(CMD_REQ##CMD) || pduHasPayLoad(CMD_RESP##CMD)) \
            bufferSize += stripeSize;                                   \
    }


#define GET_COMMAND_SIZE(CMD)                                           \
    else if ((unsigned)CMD_REQ##CMD == (unsigned)header->command) {     \
        bufferSize = sizeof(REQ##CMD);                                  \
    }else if((unsigned)CMD_RESP##CMD == (unsigned)header->command) {    \
        bufferSize = sizeof(RESP##CMD);                                 \
    }





int
hfsGetCommandSize(HFS_PROTO_HEADER *header)
{
    int bufferSize=-1;
    if(!header->command) {
        HFS_LOG_ERROR("hfsGetCommandSize Invalid Command 0x%x",header->command);
        bufferSize = -1;
    }
    GET_COMMAND_SIZE(_DS_PING)
    GET_COMMAND_SIZE(_DS_ALLOC_HANDLE)
    GET_COMMAND_SIZE(_DS_FREE_HANDLE)
    GET_COMMAND_SIZE(_DS_READ_STRIPE)
    GET_COMMAND_SIZE(_DS_WRITE_STRIPE)
    GET_COMMAND_SIZE(_DS_READ_STRIPE_V)
    GET_COMMAND_SIZE(_DS_WRITE_STRIPE_V)
    GET_COMMAND_SIZE(_DS_GET_LENGTH)
    GET_COMMAND_SIZE(_DS_GET_DISK_USAGE)
    GET_COMMAND_SIZE(_DS_GET_STATS)
    GET_COMMAND_SIZE(_MDS_PING)
    GET_COMMAND_SIZE(_MDS_ALLOC_HANDLE)
    GET_COMMAND_SIZE(_MDS_FREE_HANDLE)
    GET_COMMAND_SIZE(_MDS_READDIR)
    GET_COMMAND_SIZE(_MDS_CREATE_DIRENT)
    GET_COMMAND_SIZE(_MDS_DELETE_DIRENT)
    GET_COMMAND_SIZE(_MDS_LOOK_UP)
    GET_COMMAND_SIZE(_MDS_GET_ATTR)
    GET_COMMAND_SIZE(_MDS_SET_ATTR)
    GET_COMMAND_SIZE(_MDS_GET_EXTENT_SIZE)
    GET_COMMAND_SIZE(_MDS_GET_CONFIG)
    GET_COMMAND_SIZE(_MDS_FS_CONF_CHANGED)
    GET_COMMAND_SIZE(_MDS_LOGIN_PHASE1)
    GET_COMMAND_SIZE(_MDS_LOGIN_PHASE2)
    else {
            HFS_LOG_ERROR("hfsGetCommandSize Invalid Command 0x%x",header->command);
            bufferSize = -1;
    }
return bufferSize;
}


int GetMaximumMemSize(HFS_PROTO_HEADER *header,int stripeSize)
{
    int bufferSize=sizeof(HFS_PROTO_HEADER) + sizeof(HFS_QUEUE_ITEM);
    if(!header->command) {
        HFS_LOG_ERROR("hfsGetCommandSize Invalid Command 0x%x",header->command);
        bufferSize = -1;
    }
    GET_BUFF_SIZE(_DS_PING)
    GET_BUFF_SIZE(_DS_ALLOC_HANDLE)
    GET_BUFF_SIZE(_DS_FREE_HANDLE)
    GET_BUFF_SIZE(_DS_READ_STRIPE)
    GET_BUFF_SIZE(_DS_WRITE_STRIPE)
    GET_BUFF_SIZE(_DS_READ_STRIPE_V)
    GET_BUFF_SIZE(_DS_WRITE_STRIPE_V)
    GET_BUFF_SIZE(_DS_GET_LENGTH)
    GET_BUFF_SIZE(_DS_GET_DISK_USAGE)
    GET_BUFF_SIZE(_DS_GET_STATS)
    GET_BUFF_SIZE(_MDS_PING)
    GET_BUFF_SIZE(_MDS_ALLOC_HANDLE)
    GET_BUFF_SIZE(_MDS_FREE_HANDLE)
    GET_BUFF_SIZE(_MDS_READDIR)
    GET_BUFF_SIZE(_MDS_CREATE_DIRENT)
    GET_BUFF_SIZE(_MDS_DELETE_DIRENT)
    GET_BUFF_SIZE(_MDS_LOOK_UP)
    GET_BUFF_SIZE(_MDS_GET_ATTR)
    GET_BUFF_SIZE(_MDS_SET_ATTR)
    GET_BUFF_SIZE(_MDS_GET_EXTENT_SIZE)
    GET_BUFF_SIZE(_MDS_GET_CONFIG)
    GET_BUFF_SIZE(_MDS_FS_CONF_CHANGED)
    GET_BUFF_SIZE(_MDS_LOGIN_PHASE1)
    GET_BUFF_SIZE(_MDS_LOGIN_PHASE2)
    else {
            HFS_LOG_ERROR("hfsGetCommandSize Invalid Command 0x%x",header->command);
            bufferSize = -1;
    }
    return bufferSize;
}

int pduHasPayLoad(__u32 command)
{
    if(command == CMD_RESP_DS_READ_STRIPE) {
        return 1;
    }
    if(command == CMD_REQ_DS_WRITE_STRIPE) {
        return 1;
    }
    if(command == CMD_RESP_DS_WRITE_STRIPE_V) {
        return 1;
    }
    if(command == CMD_REQ_DS_WRITE_STRIPE_V) {
        return 1;
    }
    if(command == CMD_RESP_MDS_READDIR) {
        return 1;
    }
    if(command == CMD_RESP_MDS_GET_CONFIG) {
        return 1;
    }

    return 0;
}

__u32
toggleReqResp(__u32 command) {
    if(command & (0x1UL)) {
        command += 0x1UL;
    } else {
        command -= 0x1UL;
    }
    return command;
}

void
hfsWaitQItem(PHFS_QUEUE_ITEM pQueueItem)
{
    HFS_ENTRY();
    hfsSemWait(&pQueueItem->completionSem);
    HFS_LEAVE();
}

void
hfsSignalQItem(PHFS_QUEUE_ITEM pQueueItem)
{
    HFS_ENTRY();
    hfsSempost(&pQueueItem->completionSem);
    HFS_LEAVE();
}

PHFS_QUEUE_ITEM
hfsAllocateProtoBuff(HFS_PROTO_HEADER *pHeader,
                     int stripeSize)
{
    char *buff;
    int bufferSize = GetMaximumMemSize(pHeader,stripeSize);
    if(-1 == bufferSize)
    return NULL;
    buff = calloc(bufferSize,sizeof(char));
    if(!buff) {
        return ((PHFS_QUEUE_ITEM) buff);
    }

    // Very important step code assumes that anything that is not touched is zero
    // Extremely essential for the RAID 5 Logic

    // book keeping for the list Q item //
    INIT_LIST_HEAD(&((PHFS_QUEUE_ITEM)buff)->listHead);
    hfsSemInit(&((PHFS_QUEUE_ITEM)buff)->completionSem,1,0);
    ((PHFS_QUEUE_ITEM)buff)->totalMemory = bufferSize;
    memcpy(buff+sizeof(HFS_QUEUE_ITEM),pHeader,sizeof(*pHeader));
    return ((PHFS_QUEUE_ITEM) buff);
}

void
hfsFreeProtoBuff(PHFS_QUEUE_ITEM protobuff)
{
    if(protobuff) {
        hfsFree(protobuff);
        protobuff = NULL;
    }
}



// Encoding and Decoding functions
void
hfsEncodeProtoHeader(HFS_PROTO_HEADER *hdr){
    int i=0;
    __u32 *ptr = (__u32*) hdr;

    for(i=0;i<sizeof(*hdr)/sizeof(*ptr);i++) {
        ptr[i] = htonl(ptr[i]);
    }
}

void
hfsDecodeProtoHeader(HFS_PROTO_HEADER *hdr)
{
    int i=0;
    __u32 *ptr = (__u32*) hdr;
    for(i=0;i<sizeof(*hdr)/sizeof(*ptr);i++) {
        ptr[i] = ntohl(ptr[i]);
    }
}




#define ENC_CMD_BUFF(CMD)                                               \
    case CMD_REQ##CMD:                                                  \
    for(i=0;i<sizeof(REQ##CMD)/sizeof(*cmdbuffer);i++)                  \
        cmdbuffer[i] = htonl(cmdbuffer[i]);                             \
    break;                                                              \
    case CMD_RESP##CMD:                                                 \
    for(i=0;i<sizeof(RESP##CMD)/sizeof(*cmdbuffer);i++)                 \
         cmdbuffer[i] = htonl(cmdbuffer[i]);                            \
    break;                                                              \


#define DEC_CMD_BUFF(CMD)                                               \
    case CMD_REQ##CMD:                                                  \
    for(i=0;i<sizeof(REQ##CMD)/sizeof(*cmdbuffer);i++)                  \
        cmdbuffer[i] = ntohl(cmdbuffer[i]);                             \
    break;                                                              \
    case CMD_RESP##CMD:                                                 \
    for(i=0;i<sizeof(RESP##CMD)/sizeof(*cmdbuffer);i++)                 \
        cmdbuffer[i] = ntohl(cmdbuffer[i]);                             \
    break;                                                              \


HFS_STATUS
hfsEncodeCommandBuffer(char *buffer,__u32 command)
{
    HFS_STATUS  status = HFS_STATUS_SUCCESS;
    __u32       *cmdbuffer = (__u32 *) buffer;
    int         i;

    switch(command) {
        ENC_CMD_BUFF(_DS_PING)
        ENC_CMD_BUFF(_DS_ALLOC_HANDLE)
        ENC_CMD_BUFF(_DS_FREE_HANDLE)
        ENC_CMD_BUFF(_DS_READ_STRIPE)
        ENC_CMD_BUFF(_DS_WRITE_STRIPE)
        ENC_CMD_BUFF(_DS_READ_STRIPE_V)
        ENC_CMD_BUFF(_DS_WRITE_STRIPE_V)
        ENC_CMD_BUFF(_DS_GET_LENGTH)
        ENC_CMD_BUFF(_DS_GET_DISK_USAGE)
        ENC_CMD_BUFF(_DS_GET_STATS)
        ENC_CMD_BUFF(_MDS_PING)
        ENC_CMD_BUFF(_MDS_ALLOC_HANDLE)
        ENC_CMD_BUFF(_MDS_FREE_HANDLE)
        ENC_CMD_BUFF(_MDS_READDIR)
        ENC_CMD_BUFF(_MDS_CREATE_DIRENT)
        ENC_CMD_BUFF(_MDS_DELETE_DIRENT)
        ENC_CMD_BUFF(_MDS_LOOK_UP)
        ENC_CMD_BUFF(_MDS_GET_ATTR)
        ENC_CMD_BUFF(_MDS_SET_ATTR)
        ENC_CMD_BUFF(_MDS_GET_EXTENT_SIZE)
        ENC_CMD_BUFF(_MDS_GET_CONFIG)
        ENC_CMD_BUFF(_MDS_FS_CONF_CHANGED);
        ENC_CMD_BUFF(_MDS_LOGIN_PHASE1);
        ENC_CMD_BUFF(_MDS_LOGIN_PHASE2);
     default:
         HFS_LOG_ERROR("GetMaximumMemSize Invalid Command 0x%x",command);
         status = HFS_PROTOCOL_ERROR;
    }
    return status;
}

HFS_STATUS
hfsDecodeCommandBuffer(char *buffer,__u32 command)
{
    HFS_STATUS  status = HFS_STATUS_SUCCESS;
    __u32       *cmdbuffer = (__u32 *) buffer;
    int         i;

    switch(command) {
        DEC_CMD_BUFF(_DS_PING)
        DEC_CMD_BUFF(_DS_ALLOC_HANDLE)
        DEC_CMD_BUFF(_DS_FREE_HANDLE)
        DEC_CMD_BUFF(_DS_READ_STRIPE)
        DEC_CMD_BUFF(_DS_WRITE_STRIPE)
        DEC_CMD_BUFF(_DS_READ_STRIPE_V)
        DEC_CMD_BUFF(_DS_WRITE_STRIPE_V)
        DEC_CMD_BUFF(_DS_GET_LENGTH)
        DEC_CMD_BUFF(_DS_GET_DISK_USAGE)
        DEC_CMD_BUFF(_DS_GET_STATS)
        DEC_CMD_BUFF(_MDS_PING)
        DEC_CMD_BUFF(_MDS_ALLOC_HANDLE)
        DEC_CMD_BUFF(_MDS_FREE_HANDLE)
        DEC_CMD_BUFF(_MDS_READDIR)
        DEC_CMD_BUFF(_MDS_CREATE_DIRENT)
        DEC_CMD_BUFF(_MDS_DELETE_DIRENT)
        DEC_CMD_BUFF(_MDS_LOOK_UP)
        DEC_CMD_BUFF(_MDS_GET_ATTR)
        DEC_CMD_BUFF(_MDS_SET_ATTR)
        DEC_CMD_BUFF(_MDS_GET_EXTENT_SIZE)
        DEC_CMD_BUFF(_MDS_GET_CONFIG)
        DEC_CMD_BUFF(_MDS_FS_CONF_CHANGED);
        DEC_CMD_BUFF(_MDS_LOGIN_PHASE1);
        DEC_CMD_BUFF(_MDS_LOGIN_PHASE2);
     default:
         HFS_LOG_ERROR("GetMaximumMemSize Invalid Command 0x%x",command);
         status = HFS_PROTOCOL_ERROR;
    }
    return status;
}

char *
getExtentPointer(PHFS_QUEUE_ITEM pQueueItem)
{
    char * retp;
    HFS_PROTO_HEADER *pHeader; // Bad interface choice
    HFS_ENTRY();
    pHeader = (HFS_PROTO_HEADER *) (pQueueItem + 1);

    if(-1 == hfsGetCommandSize(pHeader)) {
        HFS_LOG_ERROR("hfsGetCommandSize failed please check code");
    }

    retp = (char *)pQueueItem  +
        sizeof(*pQueueItem) +
        sizeof(*pHeader)    +
        hfsGetCommandSize(pHeader);

    HFS_LEAVE();
    return retp;
}
