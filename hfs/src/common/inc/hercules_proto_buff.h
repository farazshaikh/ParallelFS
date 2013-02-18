/*
 * hercules_proto_buff.h
 *
 *     Request responce protocol buffers
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

int GetMaximumMemSize(HFS_PROTO_HEADER *header,int stripeSize);
int hfsGetCommandSize(HFS_PROTO_HEADER *header);
PHFS_QUEUE_ITEM hfsAllocateProtoBuff(HFS_PROTO_HEADER *header,int stripeSize);
void hfsFreeProtoBuff(PHFS_QUEUE_ITEM);

// Encoding and Decoding of headers //
void hfsEncodeProtoHeader(HFS_PROTO_HEADER *hdr);
void hfsDecodeProtoHeader(HFS_PROTO_HEADER *hdr);
HFS_STATUS hfsEncodeCommandBuffer(char *cmdbuffer,__u32 command);
HFS_STATUS hfsDecodeCommandBuffer(char *cmdbuffer,__u32 command);
char * getExtentPointer(PHFS_QUEUE_ITEM pQueueItem);

// Signal and Wait //
void hfsSignalQItem(PHFS_QUEUE_ITEM pQueueItem);
void hfsWaitQItem(PHFS_QUEUE_ITEM pQueueItem);


__u32 toggleReqResp(__u32 command);
int pduHasPayLoad(__u32 command);

#define INIT_HEADER(pHeader,cmd) do {                   \
        memset((pHeader),0,sizeof(HFS_PROTO_HEADER));   \
        (pHeader)->command = (cmd);                     \
        (pHeader)->protoMagic = HFS_PROTO_MAGIC;        \
        (pHeader)->status = HFS_STATUS_SUCCESS;         \
}while(0)
