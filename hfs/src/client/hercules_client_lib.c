/*
 * hercules_client_lib.c
 *
 *   library functions for the hfs client
 *
 *   fshaikh@cs.cmu.edu
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
#include <hercules_client.h>
#include <hercules_client_lib.h>

#define INIT_HEADER(pHeader,cmd) do {                   \
        memset((pHeader),0,sizeof(HFS_PROTO_HEADER));   \
        (pHeader)->command = (cmd);                     \
        (pHeader)->protoMagic = HFS_PROTO_MAGIC;        \
        (pHeader)->status = HFS_STATUS_SUCCESS;         \
    }while(0)

__u32
POSIXTOHFSMODE(mode_t m)
{
    if (S_ISREG(m)) {
        return HFS_S_IFREG;
    } else if (S_ISDIR(m)) {
        return HFS_S_IFDIR;
    } else if (S_ISCHR(m)) {
        return HFS_S_IFCHR;
    } else if (S_ISBLK(m)) {
        return HFS_S_IFBLK;
    } else if (S_ISFIFO(m)) {
        return HFS_S_IFIFO;
    } else if (S_ISLNK(m)) {
        return HFS_S_IFLNK;
    } else if (S_ISSOCK(m)) {
        return HFS_S_IFSOCK;
    }

    return HFS_S_IFUNKNOWN;
}

mode_t
HFSMODETOPOSIX(__u32 hfs_mode)
{
    if (hfs_mode == HFS_S_IFDIR) {
        return (S_IFDIR | 0755);
    } else if (hfs_mode == HFS_S_IFREG) {
        return (S_IFREG | 0744);
    } else if (hfs_mode == HFS_S_IFCHR) {
        return (S_IFCHR | 0744);
    } else if (hfs_mode == HFS_S_IFBLK) {
        return (S_IFBLK | 0744);
    } else if (hfs_mode == HFS_S_IFIFO) {
        return (S_IFIFO | 0744);
    } else if (hfs_mode == HFS_S_IFLNK) {
        return (S_IFLNK | 0744);
    } else if (hfs_mode ==HFS_S_IFSOCK) {
        return (S_IFSOCK | 0744);
    }

    return (S_IFREG | 0744);
}

HFS_STATUS
hfs_getattr(const char *ppath,
            struct stat *stat_buff)
{
    HFS_CLIENT_FILE_HDL clientFileHandle;
    HFS_STATUS   status = HFS_STATUS_SUCCESS;
    HFS_IATTR   iAttr;

    // lookup the path
    status = hfs_namei((char *)ppath,&clientFileHandle);
    if (!HFS_SUCCESS(status)) {
        return status;
    }

    // get the attributes for file //
    status = hfs_getIAttr(&clientFileHandle,&iAttr);
    if (!HFS_SUCCESS(status)) {
        return status;
    }

    // Setup the stat buf //
    stat_buff->st_dev = (dev_t)iAttr.rdev_t;
    stat_buff->st_ino = (ino_t)iAttr.selfMetaDataHandle;
    stat_buff->st_mode  = (mode_t)iAttr.iMode;
    stat_buff->st_nlink = (nlink_t) iAttr.linkCount;

    stat_buff->st_uid = iAttr.owner;
    stat_buff->st_gid = iAttr.group;

    stat_buff->st_rdev = (dev_t)iAttr.rdev_t;
    stat_buff->st_size = (off_t)iAttr.size;
    stat_buff->st_blksize = getHFSFSCtx()->stripeSize;
    stat_buff->st_blocks = (blkcnt_t)(iAttr.size/getHFSFSCtx()->stripeSize) + 1;

    stat_buff->st_atime = (time_t) iAttr.a_time;
    stat_buff->st_mtime = (time_t) iAttr.m_time;
    stat_buff->st_ctime = (time_t) iAttr.c_time;

    // Special processing for the mode now //
    stat_buff->st_mode = HFSMODETOPOSIX(iAttr.iMode);
    return HFS_STATUS_SUCCESS;
}

HFS_STATUS
hfs_readlink(const char *ppath,
             char *buff,
             size_t size)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS
hfs_mknod(const char *ppath,
          mode_t  mode,
          dev_t  dev)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS
hfs_mkdir(const char *ppath,
          mode_t  mode)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS
hfs_unlink(const char *ppath)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS
hfs_rmdir(const char *ppath)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS
hfs_symlink(const char *ppath,
            const char *ppathLink)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS
hfs_rename(const char *oldname,
           const char *newname)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS
hfs_link(const char *from,
         const char *to)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS
hfs_chmod(const char *ppath,
          mode_t  mode)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS
hfs_chown(const char *ppath,
          uid_t  uid,
          gid_t  gid)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS
hfs_truncate(const char *ppath,
             off_t  size)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS
hfs_utime(const char *ppath,
          struct utimbuf *timbuf)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS
hfs_read(PHFS_CLIENT_FILE_HDL pClientFileHdl,
         char    *buffer,
         off_t    offset,
         IN OUT size_t  *size)
{
    PHFS_IATTR pIAttr;
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    size_t bytesServiced;
    int i;
    int stripesNR;
    int failedReadCount;
    int firstLogicalStripe;
    int lastLogicalStripe;
    PHFS_QUEUE_ITEM *ppReadExtentItems;
    HFS_PROTO_HEADER header;
    PHFS_SERVER_REQ  pServReq;
    PHFS_SERVER_RESP pServResp;
    char    *pCurrentWriteBuff,*pCurrentReadBuff;

    HFS_ENTRY();
    if (*size <=0) {
        return *size;
    }

    pIAttr = &pClientFileHdl->iAttr;

    INIT_HEADER(&header,CMD_REQ_DS_READ_STRIPE);
    // Get the where abouts of the data //

    // Number of extents to read //
    firstLogicalStripe = offset / getHFSFSCtx()->stripeSize;
    lastLogicalStripe = (offset + *size - 1) / getHFSFSCtx()->stripeSize;
    stripesNR   = lastLogicalStripe - firstLogicalStripe + 1;
    ppReadExtentItems = hfsCalloc(stripesNR * sizeof(PHFS_QUEUE_ITEM));
    if (!(ppReadExtentItems)) {
        status = HFS_LOW_RESOURCES;
        goto cleanup;
    }

    // Allocate the protobuff for each of the stripe //
    for(i=0 ; i < stripesNR ; i++) {
        HFS_RAID5_BLK_MAP   hfsRaid5BlkMap;
        memset(&hfsRaid5BlkMap,0,sizeof(hfsRaid5BlkMap));

        // Get the server mappings //
        status = hfsRaid5GetBlockMapping(firstLogicalStripe + i,pIAttr,&hfsRaid5BlkMap);
        if (!HFS_SUCCESS(status)) {
            status = HFS_STATUS_CONFIG_ERROR;
            goto cleanup;
        }

        ppReadExtentItems[i] = hfsAllocateProtoBuff(&header,getHFSFSCtx()->stripeSize);
        if (!ppReadExtentItems[i]) {
            status = HFS_LOW_RESOURCES;
            goto cleanup;
        }
        pServReq = (PHFS_SERVER_REQ) (ppReadExtentItems[i] + 1);
        pServResp = (PHFS_SERVER_RESP)(ppReadExtentItems[i] + 1);

        pServReq->req.reqDSReadStripe.serverId = pIAttr->dataHandlesSrvs[hfsRaid5BlkMap.dataServerIdx];
        pServReq->req.reqDSReadStripe.dataHandle = pIAttr->dataHandles[hfsRaid5BlkMap.dataServerIdx];
        pServReq->req.reqDSReadStripe.stripeNum = hfsRaid5BlkMap.dataStripeOffset;
        pServReq->req.reqDSReadStripe.stripeCnt = 1;
        pServReq->req.reqDSReadStripe.readSize = getHFSFSCtx()->stripeSize;

        //- Now we have to save the index into the inode for which this read was issued -//
        //- So the degraded reader can use this index + stripcount to get the backup extents --//
        // _ ORIG __ []
        // _ ORIG __ [] <-- = "save this into the cookie".
        // _ ORIG __ [] |
        // _ ORIG __ [] + stripecnt
        // _ BCKUP __[] |
        // _ BCKUP __[] <--
        // _ BCKUP __[]
        // _ BCKUP __[]
        pServReq->hdr.cookies[COOKIE_DATA_SERVER_IDX] = hfsRaid5BlkMap.dataServerIdx;


        // go to the right ds //
        ppReadExtentItems[i]->clientIdx =
            GET_DS_SERVERS_CLIENT_IDX(pServReq->req.reqDSReadStripe.serverId);
    }


    // Send out Reads //
    for(i=0 ; i < stripesNR ; i++) {
        status = hfsQueueItem(getHFSClient()->kernel.pQOutBound,ppReadExtentItems[i]);
        if (!HFS_SUCCESS(status)) {
            HFS_LOG_ERROR("FOUCH Memory was wasted as incomplete reads were queue");
            return HFS_PROTOCOL_ERROR;
        }
    }

    // Wait for reads to complete //
    for(i = 0 ; i < stripesNR ; i++) {
        hfsWaitQItem(ppReadExtentItems[i]);
    }

    // RAID 5 Reconstruction is required         //
    // Count the number of failures           //
    // We can tolerate 1 Read failure and serve        //
    // using our degraded read operation and hope that the server comes up //
    failedReadCount = 0;
    for (i = 0 ; i < stripesNR ; i++) {
        pServResp = (PHFS_SERVER_RESP)(ppReadExtentItems[i] + 1);
        if (HFS_SUCCESS(pServResp->hdr.status)) {
            continue;
        }

        HFS_LOG_ERROR("Read Failed to read %d stripes", ++failedReadCount);
        HFS_UNUSED(failedReadCount);
        //- Try doing a degraded read
        // This will surely succeed if only 1 server is down
        // If more than 1 servers are down it will return a zero filled buffer
        // We log such an event but do server 0 bytes data to the client.
        status = hfsDegradedDSRequest(ppReadExtentItems[i],
                                      pIAttr);
        if (!HFS_SUCCESS(status)) {
            HFS_LOG_ERROR("Cannot get the reconstruction done for LCN %d",firstLogicalStripe+i);
            HFS_LOG_ERROR("Client Read will be short");
            status = HFS_STATUS_SUCCESS;
        }
    }



    // copy the stuff back into the buffer //
    pCurrentWriteBuff = buffer;
    bytesServiced  = 0;
    for(i = 0 ; i < stripesNR ; i++) {
        int     extent_offset_to_copy;
        int     extent_size_to_copy;

        /* to the header */
        pServResp = (PHFS_SERVER_RESP)(ppReadExtentItems[i] + 1);
        /* to the extent */
        pCurrentReadBuff = getExtentPointer(ppReadExtentItems[i]);

        if (0==i) {
            /* Special case head */
            extent_offset_to_copy = (offset % getHFSFSCtx()->stripeSize);
            extent_size_to_copy = MIN(getHFSFSCtx()->stripeSize - extent_offset_to_copy,*size);
        }else if (stripesNR-1==i) {
            /*Special case tail */
            extent_offset_to_copy = 0;
            extent_size_to_copy = (offset + *size) % getHFSFSCtx()->stripeSize;
            /* Special Special case perfect boundary read */
            if (0 == ((offset + *size) % getHFSFSCtx()->stripeSize) )
                extent_size_to_copy = getHFSFSCtx()->stripeSize;
        }else {
            /* normal case of copying entire extents */
            extent_offset_to_copy = 0;
            extent_size_to_copy = getHFSFSCtx()->stripeSize;
        }


        if (HFS_SUCCESS(pServResp->hdr.status)) {

            /* Actual copy Only if the command was successful*/
            if (pCurrentWriteBuff + extent_size_to_copy <= buffer + *size) {
                memcpy(pCurrentWriteBuff,
                       pCurrentReadBuff + extent_offset_to_copy,
                       extent_size_to_copy);
            } else {
                HFS_LOG_ERROR("Read logic bug");
                if (pCurrentWriteBuff < buffer + *size){
                    extent_size_to_copy = (buffer + *size) - pCurrentWriteBuff;
                    memcpy(pCurrentWriteBuff,
                           pCurrentReadBuff + extent_offset_to_copy,
                           extent_size_to_copy);
                }
            }

        }
        else
            HFS_LOG_ERROR("One of the extents failed to be read %x",(unsigned int)pServResp->hdr.status);

        // Move ahead anyways //
        pCurrentWriteBuff += extent_size_to_copy;
        bytesServiced  += extent_size_to_copy;
    }
    *size = bytesServiced;
    status = HFS_STATUS_SUCCESS;

 cleanup:
    // free all the protobuffs //
    for(i = 0 ; i < stripesNR ; i++) {
        hfsFreeProtoBuff(ppReadExtentItems[i]);
    }

    // Free the map to the protobuffs //
    if (ppReadExtentItems) {
        hfsFree(ppReadExtentItems);
    }

    HFS_LEAVE();
    return status;
}

//-- This is exactly as read Every bug in write is a possible bug in read --//
//-- Just that X2 blocks are now send out int db block backup block   --//
HFS_STATUS
hfs_write(PHFS_CLIENT_FILE_HDL pClientFileHdl,
          const char   *buffer,
          off_t    offset,
          IN OUT size_t  *size)
{
    PHFS_IATTR   pIAttr;
    HFS_STATUS   status = HFS_STATUS_SUCCESS;
    size_t    bytesServiced;
    int     i,stripesNR;
    int     firstLogicalStripe,lastLogicalStripe;
    PHFS_QUEUE_ITEM *ppWriteExtentItems;
    HFS_PROTO_HEADER header;
    PHFS_SERVER_REQ  pServReq;
    PHFS_SERVER_RESP pServResp;
    char    *pCurrentWriteBuff,*pCurrentReadBuff,*pCurrentWriteBuff2;

    HFS_ENTRY();
    if (*size <=0) {
        return *size;
    }

    pIAttr = &pClientFileHdl->iAttr;
    INIT_HEADER(&header,CMD_REQ_DS_WRITE_STRIPE);

    // Number of extents to read //
    firstLogicalStripe = offset / getHFSFSCtx()->stripeSize;
    lastLogicalStripe = (offset + *size - 1) / getHFSFSCtx()->stripeSize;
    stripesNR   = lastLogicalStripe - firstLogicalStripe + 1;
    ppWriteExtentItems = hfsCalloc(stripesNR * sizeof(PHFS_QUEUE_ITEM) * TOTAL_COPIES);
    if (!(ppWriteExtentItems)) {
        status = HFS_LOW_RESOURCES;
        goto cleanup;
    }

    // Allocate the protobuff for each of the stripe //
    for( i = 0; i < stripesNR * TOTAL_COPIES ; i = i + TOTAL_COPIES){
        HFS_RAID5_BLK_MAP   hfsRaid5BlkMap;
        memset(&hfsRaid5BlkMap,0,sizeof(hfsRaid5BlkMap));

        // Get the server mappings //
        status = hfsRaid5GetBlockMapping(firstLogicalStripe + (i / TOTAL_COPIES),pIAttr,&hfsRaid5BlkMap);
        if (!HFS_SUCCESS(status)) {
            status = HFS_STATUS_CONFIG_ERROR;
            goto cleanup;
        }

        //--------------------------------------------------------------------//
        //- Prepare the original write block ---------------------------------//
        ppWriteExtentItems[i] = hfsAllocateProtoBuff(&header,getHFSFSCtx()->stripeSize);
        if (!ppWriteExtentItems[i]) {
            status = HFS_LOW_RESOURCES;
            goto cleanup;
        }

        // Create the write of the primary datablock //
        pServReq = (PHFS_SERVER_REQ) (ppWriteExtentItems[i] + 1);
        pServResp = (PHFS_SERVER_RESP)(ppWriteExtentItems[i] + 1);
        HFS_UNUSED(pServResp);

        pServReq->req.reqDSWriteStripe.serverId = pIAttr->dataHandlesSrvs[hfsRaid5BlkMap.dataServerIdx];
        pServReq->req.reqDSWriteStripe.dataHandle = pIAttr->dataHandles[hfsRaid5BlkMap.dataServerIdx];
        pServReq->req.reqDSWriteStripe.stripeNum = hfsRaid5BlkMap.dataStripeOffset;
        pServReq->req.reqDSWriteStripe.stripeCnt = 1;
        pServReq->req.reqDSWriteStripe.writeSize = getHFSFSCtx()->stripeSize;
        pServReq->hdr.cookies[COOKIE_DATA_SERVER_IDX] = hfsRaid5BlkMap.dataServerIdx;

        // go to the right ds //
        ppWriteExtentItems[i]->clientIdx =
            GET_DS_SERVERS_CLIENT_IDX(pServReq->req.reqDSReadStripe.serverId);

        //--------------------------------------------------------------------//
        //- Prepare the backup write block ---------------------------------//
        ppWriteExtentItems[i+1] = hfsAllocateProtoBuff(&header,getHFSFSCtx()->stripeSize);
        if (!ppWriteExtentItems[i+1]) {
            status = HFS_LOW_RESOURCES;
            goto cleanup;
        }

        // Create the write of the primary datablock //
        pServReq = (PHFS_SERVER_REQ) (ppWriteExtentItems[i+1] + 1);
        pServResp = (PHFS_SERVER_RESP)(ppWriteExtentItems[i+1] + 1);

        pServReq->req.reqDSWriteStripe.serverId = pIAttr->dataHandlesSrvs[hfsRaid5BlkMap.dataServerIdx + pIAttr->stripe_cnt];
        pServReq->req.reqDSWriteStripe.dataHandle = pIAttr->dataHandles[hfsRaid5BlkMap.dataServerIdx + pIAttr->stripe_cnt];
        pServReq->req.reqDSWriteStripe.stripeNum = hfsRaid5BlkMap.dataStripeOffset;
        pServReq->req.reqDSWriteStripe.stripeCnt = 1;
        pServReq->req.reqDSWriteStripe.writeSize = getHFSFSCtx()->stripeSize;
        pServReq->hdr.cookies[COOKIE_DATA_SERVER_IDX] = hfsRaid5BlkMap.dataServerIdx;

        // go to the right ds //
        ppWriteExtentItems[i+1]->clientIdx =
            GET_DS_SERVERS_CLIENT_IDX(pServReq->req.reqDSWriteStripe.serverId);
    }


    // copy the stuff from the buffer into the proto header //
    pCurrentReadBuff = (char *)buffer;
    bytesServiced  = 0;
    for(i = 0 ; i < stripesNR * TOTAL_COPIES; i += 2) {
        int     extent_offset_to_copy; // How much into the extent buffer
        int     extent_size_to_copy; // How much size to copy

        /* to the header */
        pServResp = (PHFS_SERVER_RESP)(ppWriteExtentItems[i] + 1);
        /* to the extent */
        pCurrentWriteBuff = getExtentPointer(ppWriteExtentItems[i]);
        pCurrentWriteBuff2 = getExtentPointer(ppWriteExtentItems[i+1]);

        if (0==i) {
            /* Special case head */
            extent_offset_to_copy = (offset % getHFSFSCtx()->stripeSize);
            extent_size_to_copy = MIN(getHFSFSCtx()->stripeSize - extent_offset_to_copy,*size);
        }else if ((stripesNR * TOTAL_COPIES) - TOTAL_COPIES==i) {
            /*Special case tail */
            extent_offset_to_copy = 0;
            extent_size_to_copy = (offset + *size) % getHFSFSCtx()->stripeSize;
            /* Special Special case perfect boundary read */
            if (0 ==((offset + *size) % getHFSFSCtx()->stripeSize) )
                extent_size_to_copy = getHFSFSCtx()->stripeSize;
        } else {
            /* normal case of copying entire extents */
            extent_offset_to_copy = 0;
            extent_size_to_copy = getHFSFSCtx()->stripeSize;
        }


        /* copy stuff into primary data & secondary data server */
        if (pCurrentWriteBuff + extent_size_to_copy <= buffer + *size) {
            memcpy(pCurrentWriteBuff, // Primary
                   pCurrentReadBuff,
                   extent_size_to_copy);
            memcpy(pCurrentWriteBuff2,// Backup
                   pCurrentReadBuff,
                   extent_size_to_copy);
        } else {
            HFS_LOG_ERROR("Read logic bug");
            if (pCurrentWriteBuff < buffer + *size){
                extent_size_to_copy = (buffer + *size) - pCurrentWriteBuff;
                memcpy(pCurrentWriteBuff + extent_offset_to_copy, // Primary
                       pCurrentReadBuff,
                       extent_size_to_copy);
                memcpy(pCurrentWriteBuff2 + extent_offset_to_copy, // Backup
                       pCurrentReadBuff,
                       extent_size_to_copy);
            }
        }

        // Set the right offsets for the server //
        // Create the write of the primary datablock //
        // Primary
        pServReq = (PHFS_SERVER_REQ) (ppWriteExtentItems[i] + 1);
        pServResp = (PHFS_SERVER_RESP)(ppWriteExtentItems[i] + 1);
        pServReq->req.reqDSWriteStripe.writeSize = extent_size_to_copy;
        pServReq->req.reqDSWriteStripe.writeInStripeOffset = extent_offset_to_copy;
        pServReq->hdr.pduExtentSize = extent_size_to_copy;

        // Primary
        pServReq = (PHFS_SERVER_REQ) (ppWriteExtentItems[i+1] + 1);
        pServResp = (PHFS_SERVER_RESP)(ppWriteExtentItems[i+1] + 1);
        pServReq->req.reqDSWriteStripe.writeSize =  extent_size_to_copy;
        pServReq->req.reqDSWriteStripe.writeInStripeOffset = extent_offset_to_copy;
        pServReq->hdr.pduExtentSize = extent_size_to_copy;


        // Move ahead anyways //
        pCurrentReadBuff += extent_size_to_copy;
        bytesServiced  += extent_size_to_copy;
    }

    // Send out Writes //
    for(i=0;i<stripesNR * TOTAL_COPIES;i++){
        status = hfsQueueItem(getHFSClient()->kernel.pQOutBound,ppWriteExtentItems[i]);
        if (!HFS_SUCCESS(status)) {
            HFS_LOG_ERROR("FOUCH Memory was wasted as incomplete reads were queue");
            return HFS_PROTOCOL_ERROR;
        }
    }

    // Wait for writes to complete //
    for(i=0;i<stripesNR * TOTAL_COPIES;i++)
        hfsWaitQItem(ppWriteExtentItems[i]);


    *size = bytesServiced;
    status = HFS_STATUS_SUCCESS;

 cleanup:
    // free all the protobuffs //
    for(i = 0 ; i < stripesNR * TOTAL_COPIES ; i++) {
        hfsFreeProtoBuff(ppWriteExtentItems[i]);
    }

    // Free the map to the protobuffs //
    if (ppWriteExtentItems) {
        hfsFree(ppWriteExtentItems);
    }

    HFS_LEAVE();
    return status;
}

HFS_STATUS
hfs_statfs(const char  *ppath,
           struct statfs *statvfs)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS
hfs_flush(const char *ppath,
          PHFS_CLIENT_FILE_HDL pClientFileHdl)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS
hfs_release(const char   *ppath,
            PHFS_CLIENT_FILE_HDL pClientFileHdl)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS
hfs_fsync(const char *ppath,
          PHFS_CLIENT_FILE_HDL pClientFileHdl)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

#ifdef HAVE_SETXATTR
HFS_STATUS
hfs_setxattr(const char *ppath,
             const char *attrname,
             const char *buuf,
             size_t, int)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS
hfs_getxattr(const char *, const char *, char *, size_t)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS
hfs_listxattr(const char *, char *, size_t)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS
hfs_removexattr(const char *, const char *)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}
#endif

HFS_STATUS
hfs_opendir(const char    *ppath,
            PHFS_CLIENT_FILE_HDL pClientFileHdl)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

// Here we don't need the inode attributes for the directory  //
// So attribute caching is not done here       //
// What this means is pClientFileHdl->iAttr is BOGUS don't touch it //
HFS_STATUS
hfs_readdir(PHFS_CLIENT_FILE_HDL pClientFileHdl,
            off_t    offset,
            PHFS_QUEUE_ITEM  *ppRetQueueItem)
{
    HFS_STATUS  status = HFS_STATUS_SUCCESS;
    HFS_PROTO_HEADER header;
    PHFS_QUEUE_ITEM pQueueItem;
    PHFS_SERVER_REQ pServReq;
    PHFS_SERVER_RESP pServResp;

    INIT_HEADER(&header,CMD_REQ_MDS_READDIR);

    // Create the packet and setup the correct pointers//
    pQueueItem =
        hfsAllocateProtoBuff(&header,getHFSFSCtx()->stripeSize);
    if (!pQueueItem) {
        HFS_LOG_ERROR("Server low on memory or bad header");
        return HFS_PROTOCOL_ERROR;
    }
    pServResp = (PHFS_SERVER_RESP)(pQueueItem + 1);
    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);

    pServReq->req.reqMDSReaddir.direntOffset = offset;
    pServReq->req.reqMDSReaddir.metaDataHandle = pClientFileHdl->metaDataHandle;

    // Send out the packet //
    pQueueItem->clientIdx = GET_MDS_SERVERS_CLIENT_IDX(pClientFileHdl->serverId);
    status = hfsQueueItem(getHFSClient()->kernel.pQOutBound,pQueueItem);
    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR("Cannot queue item on out bound");
        return HFS_PROTOCOL_ERROR;
    }
    // Wait for the response //
    hfsWaitQItem(pQueueItem);

    // Retry the stuff if stuff has failed on the MDS //
    if (!HFS_SUCCESS(pServResp->hdr.status)) {
        //-Sadly rebuild the packet-//
        INIT_HEADER(&pServReq->hdr,CMD_REQ_MDS_READDIR);
        pServReq->req.reqMDSReaddir.direntOffset = offset;
        pServReq->req.reqMDSReaddir.metaDataHandle = pClientFileHdl->metaDataHandle;


        status = hfsDegradedMDSRequest(pQueueItem);
        if (!HFS_SUCCESS(status)) {
            HFS_LOG_ERROR("Failed retry from backups");
            status = HFS_STATUS_SUCCESS;
        }
    }

    *ppRetQueueItem = (pQueueItem); // Will be free by the calling function //
    return HFS_STATUS_SUCCESS;
}

HFS_STATUS hfs_releasedir(const char   *ppath,
                          PHFS_CLIENT_FILE_HDL pClientFileHdl)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS hfs_fsyncdir(const char *ppath,
                        PHFS_CLIENT_FILE_HDL pClientFileHdl)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

//void *(* init )(struct fuse_conn_info *conn)
//void(* destroy )(void *)
HFS_STATUS hfs_access(const char *ppath,
                      int   access)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}


HFS_STATUS hfs_ftruncate(const char *ppath,
                         off_t offset,
                         PHFS_CLIENT_FILE_HDL pClientFileHdl)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS hfs_fgetattr(const char *ppath,
                        struct stat *pStat,
                        PHFS_CLIENT_FILE_HDL pClientFileHdl)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS hfs_lock(const char *ppath,
                    PHFS_CLIENT_FILE_HDL pClientFileHdl,
                    HFS_STATUS cmd,
                    struct flock *flock)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS hfs_utimens(const char *ppath,
                       const struct timespec tv[2])
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}

HFS_STATUS hfs_bmap(const char *ppath,
                    size_t  blocksize,
                    uint64_t *idx)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    return status;
}


// some core API //
HFS_STATUS
hfs_add_dirent(PHFS_CLIENT_FILE_HDL parentFileHdl,
               PHFS_CLIENT_FILE_HDL childFileHdl,
               PHFS_DIRENT   pDirent)
{
    HFS_STATUS   status = HFS_STATUS_SUCCESS;
    HFS_PROTO_HEADER header;
    PHFS_QUEUE_ITEM  pQueueItem;
    PHFS_SERVER_REQ  pServReq;
    PHFS_SERVER_RESP pServResp;

    INIT_HEADER(&header,CMD_REQ_MDS_CREATE_DIRENT);

    // Create the packet and setup the correct pointers//
    pQueueItem = hfsAllocateProtoBuff(&header,getHFSFSCtx()->stripeSize);
    if (!pQueueItem) {
        HFS_LOG_ERROR("Server low on memory or bad header");
        return HFS_PROTOCOL_ERROR;
    }

    pServResp = (PHFS_SERVER_RESP)(pQueueItem + 1);
    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);

    pServReq->req.reqMDSCreateDirent.hfsDirent.dirEntMode    = pDirent->dirEntMode;
    strncpy((char *)pServReq->req.reqMDSCreateDirent.hfsDirent.dname,(char *) pDirent->dname,HFS_MAX_FILE_NAME);
    pServReq->req.reqMDSCreateDirent.hfsDirent.selfMetaDataHandle  = childFileHdl->metaDataHandle;
    pServReq->req.reqMDSCreateDirent.hfsDirent.serverId    = childFileHdl->serverId;
    pServReq->req.reqMDSCreateDirent.hfsDirent.parentMetaDataHandle = parentFileHdl->metaDataHandle;

    // Send out the packet //
    pQueueItem->clientIdx = GET_MDS_SERVERS_CLIENT_IDX(parentFileHdl->serverId);
    status = hfsQueueItem(getHFSClient()->kernel.pQOutBound,pQueueItem);
    if (!HFS_SUCCESS(status)) {
        hfsFreeProtoBuff(pQueueItem);
        HFS_LOG_ERROR("Cannot queue item on out bound");
        return HFS_PROTOCOL_ERROR;
    }

    // Wait for the response //
    hfsWaitQItem(pQueueItem);

    // Do the operation on the backup if things failed //
    if (!HFS_SUCCESS(pServResp->hdr.status)) {
        //-Sadly rebuild the packet-//
        INIT_HEADER(&pServReq->hdr,CMD_REQ_MDS_CREATE_DIRENT);
        pServReq->req.reqMDSCreateDirent.hfsDirent.dirEntMode    = pDirent->dirEntMode;
        strncpy((char *)pServReq->req.reqMDSCreateDirent.hfsDirent.dname,(char *) pDirent->dname,HFS_MAX_FILE_NAME);
        pServReq->req.reqMDSCreateDirent.hfsDirent.selfMetaDataHandle  = childFileHdl->metaDataHandle;
        pServReq->req.reqMDSCreateDirent.hfsDirent.serverId    = childFileHdl->serverId;
        pServReq->req.reqMDSCreateDirent.hfsDirent.parentMetaDataHandle = parentFileHdl->metaDataHandle;

        status = hfsDegradedMDSRequest(pQueueItem);
        if (!HFS_SUCCESS(status))
            HFS_LOG_ERROR("Failed retry from backups");
    }

    status = pServResp->hdr.status;
    hfsFreeProtoBuff(pQueueItem);
    return status;
}


HFS_STATUS
hfs_create_alloc_meta_data_handles(PHFS_IATTR    piAttr,
                                   PHFS_CLIENT_FILE_HDL pClientFileHdl)
{
    HFS_STATUS   status = HFS_STATUS_SUCCESS;
    HFS_PROTO_HEADER header;
    PHFS_QUEUE_ITEM  pQueueItem;
    PHFS_SERVER_REQ  pServReq;
    PHFS_SERVER_RESP pServResp;
    HFS_SERVER_ID  serverId;

    INIT_HEADER(&header,CMD_REQ_MDS_ALLOC_HANDLE);

    // Create the packet and setup the correct pointers//
    pQueueItem = hfsAllocateProtoBuff(&header,getHFSFSCtx()->stripeSize);
    if (!pQueueItem) {
        HFS_LOG_ERROR("Server low on memory or bad header");
        return HFS_PROTOCOL_ERROR;
    }

    pServResp = (PHFS_SERVER_RESP)(pQueueItem + 1);
    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);
    serverId = rand() % (getHFSFSCtx()->mdsCount / HFS_CLIENT_MDS_IDX_INTERLEAVE);

    pServReq->req.reqMDSAllocHandle.metaDataHandle = 0;
    pServReq->req.reqMDSAllocHandle.serverId  = serverId;
    // Send out the packet //
    pQueueItem->clientIdx =
        GET_MDS_SERVERS_CLIENT_IDX(pServReq->req.reqMDSAllocHandle.serverId);
    status = hfsQueueItem(getHFSClient()->kernel.pQOutBound,pQueueItem);
    if (!HFS_SUCCESS(status)) {
        hfsFreeProtoBuff(pQueueItem);
        HFS_LOG_ERROR("Cannot queue item on out bound");
        return HFS_PROTOCOL_ERROR;
    }

    // Wait for the response //
    hfsWaitQItem(pQueueItem);

    // Do the operation on the backup if things failed //
    if (!HFS_SUCCESS(pServResp->hdr.status)) {
        //-Sadly rebuild the packet-//
        INIT_HEADER(&pServReq->hdr,CMD_REQ_MDS_ALLOC_HANDLE);
        pServReq->req.reqMDSAllocHandle.metaDataHandle = 0;
        pServReq->req.reqMDSAllocHandle.serverId  = serverId;

        status = hfsDegradedMDSRequest(pQueueItem);
        if (!HFS_SUCCESS(status)) {
            HFS_LOG_ERROR("Failed retry from backups");
        }
    }

    if (HFS_SUCCESS(pServResp->hdr.status)) {
        pClientFileHdl->metaDataHandle = pServResp->resp.respMDSAllocHandle.metaDataHandle;
        pClientFileHdl->serverId  = pServResp->resp.respMDSAllocHandle.serverId;
        piAttr->selfMetaDataHandle  = pClientFileHdl->metaDataHandle;
    }

    status = pServResp->hdr.status;
    hfsFreeProtoBuff(pQueueItem);
    return status;
}


HFS_STATUS
hfs_create_alloc_data_handle(PHFS_IATTR piAttr)
{
    HFS_STATUS   status = HFS_STATUS_SUCCESS;
    int     i,numDataHandles;
    int     firstDataServer;
    int     firstDataServerFixed;
    int     copyIdx;
    PHFS_QUEUE_ITEM *ppAllocDataHandleItems;
    HFS_PROTO_HEADER header;
    PHFS_SERVER_REQ  pServReq;
    PHFS_SERVER_RESP pServResp;
    HFS_ENTRY();

    // Since we are operating raid 5 we cannot work with less than 3 data server//
    if (getHFSFSCtx()->dsCount < HFS_MIN_STRIPE_CNT) {
        return HFS_STATUS_CONFIG_ERROR;
    }

    INIT_HEADER(&header,CMD_REQ_DS_ALLOC_HANDLE);

    // Numbers of data handles to allocate to read //
    numDataHandles  = getHFSFSCtx()->dsCount;
    if (numDataHandles > HFS_MAX_STRIPE_CNT) {
        numDataHandles = HFS_MAX_STRIPE_CNT;
    }


    ppAllocDataHandleItems = hfsCalloc(numDataHandles * sizeof(PHFS_QUEUE_ITEM) * TOTAL_COPIES);
    if (!(ppAllocDataHandleItems)) {
        status = HFS_LOW_RESOURCES;
        goto cleanup;
    }

    // Allocate the protobuff for each of the original alloc handle //
    firstDataServer = rand() % getHFSFSCtx()->dsCount;
    firstDataServerFixed = firstDataServer;
    for(i = 0 ; i < numDataHandles * TOTAL_COPIES ; i++) {
        ppAllocDataHandleItems[i] = hfsAllocateProtoBuff(&header,getHFSFSCtx()->stripeSize);
        if (!ppAllocDataHandleItems[i]) {
            status = HFS_LOW_RESOURCES;
            goto cleanup;
        }
        pServReq = (PHFS_SERVER_REQ)(ppAllocDataHandleItems[i] + 1);
        pServResp = (PHFS_SERVER_RESP)(ppAllocDataHandleItems[i] + 1);

        //- Interleave if we have done calculating the data-blocks -//
        if (i == numDataHandles) {
            HFS_LOG_INFO("Interleaving at %d",numDataHandles);
            firstDataServer++;
        }

        // Make the group [0 1 2] [1 2 0] //
        pServReq->req.reqDSAllocHandle.dataHandle = 0;
        pServReq->req.reqDSAllocHandle.serverId = (firstDataServer + i) % numDataHandles;

        //- Now make group roll over entire DS-//
        pServReq->req.reqDSAllocHandle.serverId += firstDataServerFixed;
        pServReq->req.reqDSAllocHandle.serverId %= getHFSFSCtx()->dsCount;

        // go to the right ds //
        ppAllocDataHandleItems[i]->clientIdx =
            GET_DS_SERVERS_CLIENT_IDX(pServReq->req.reqDSAllocHandle.serverId);
    }

    // Send out the alloc handles //
    for ( i = 0 ; i < numDataHandles * TOTAL_COPIES ; i++) {
        status = hfsQueueItem(getHFSClient()->kernel.pQOutBound,ppAllocDataHandleItems[i]);
        if (!HFS_SUCCESS(status)) {
            HFS_LOG_ERROR("FOUCH Memory was wasted as incomplete reads were queue");
            return HFS_PROTOCOL_ERROR;
        }
    }

    // Wait for get data handles to complete //
    for(i=0 ; i < numDataHandles * TOTAL_COPIES ; i++) {
        hfsWaitQItem(ppAllocDataHandleItems[i]);
    }

    // copy the data handles back into the inode //
    copyIdx = 0;
    for( i = 0 ; i < numDataHandles * TOTAL_COPIES ; i++){
        pServResp = (PHFS_SERVER_RESP)(ppAllocDataHandleItems[i] + 1);
        /* Actual copy Only if the command was successful*/
        if (HFS_SUCCESS(pServResp->hdr.status)){
            piAttr->dataHandles[copyIdx]  = pServResp->resp.respDSAllocHandle.dataHandle;
            piAttr->dataHandlesSrvs[copyIdx++] = pServResp->resp.respDSAllocHandle.serverId;
        } else {
            HFS_LOG_ERROR("One of the extents failed to be read %x",(unsigned int)pServResp->hdr.status);
            status = pServResp->hdr.status;
            goto cleanup;
        }
    }
    piAttr->stripe_cnt = copyIdx / TOTAL_COPIES;
    piAttr->size  = 0;

    // Well all DS servers may be down //
    if (!piAttr->stripe_cnt) {
        HFS_LOG_ERROR("Looks like all data servers are down. Or else check logic !");
        status = HFS_LOW_RESOURCES;
        goto cleanup;
    }

    // All done
    status = HFS_STATUS_SUCCESS;

 cleanup:
    // free all the proto buffs //
    for( i = 0 ; i < numDataHandles ; i++) {
        hfsFreeProtoBuff(ppAllocDataHandleItems[i]);
    }

    // Free the map to the proto buffs //
    if (ppAllocDataHandleItems) {
        hfsFree(ppAllocDataHandleItems);
    }

    HFS_LEAVE();
    return status;
}

// NOTES: Call this function only if an lookup failed //
HFS_STATUS hfs_create(char    *ppath,
                      __u32    mode,    // This is mode as defined by UNIX
                      __u32    fs_dev_t,
                      PHFS_CLIENT_FILE_HDL pClientFileHdl)
{
    char    *fileName;
    char     firstFileChar;  //because we do some cool stuff here //
    HFS_CLIENT_FILE_HDL parentDirHandle;
    HFS_STATUS   status = HFS_STATUS_SUCCESS;
    int     i=0;
    HFS_IATTR   iAttr;
    HFS_DIRENT   childDirent;
    time_t    now;
    HFS_ENTRY();

    //-- Separate the path dirname + filename --//
    if (!ppath)
        return HFS_PROTO_ERROR_BAD_PATH;
    i=strlen(ppath)-1;
    if (i<=0)
        return HFS_PROTO_ERROR_BAD_PATH;
    for(;i>=0;i--) {
        if (ppath[i]=='/' || ppath[i]=='\\') {
            break;
        }
    }

    if (i<0) {
        return HFS_PROTO_ERROR_BAD_PATH;
    }

    fileName = ppath + i + 1;

    //-We were called only if the lookup for the file has failed -
    // Absolutely right to assume that the file does not exist //
    firstFileChar = fileName[0];
    fileName[0] = '\0';
    status = hfs_namei((char *)ppath,&parentDirHandle);
    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR("Cannot locate parent %s for file %s",ppath,fileName);
        fileName[0] = firstFileChar;
        return HFS_PROTO_ERROR_BAD_PATH;
    }
    fileName[0] = firstFileChar;

    memset(&iAttr,0,sizeof(iAttr));
    // Create the Data handles //
    if (HFS_S_IFDIR != POSIXTOHFSMODE(mode)) {
        status = hfs_create_alloc_data_handle(&iAttr);
        if (!HFS_SUCCESS(status) ) {
            HFS_LOG_ERROR("Cannot Create data handles for parent %s for file %s",ppath,fileName);
            return status;
        }
    }
    // Allocate Meta data handle //
    status = hfs_create_alloc_meta_data_handles(&iAttr,pClientFileHdl);
    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR("Cannot Create data handles for parent %s for file %s",ppath,fileName);
        return status;
    }


    //- Set the new i-node aka Meta data handle -//
    iAttr.a_time  = iAttr.m_time = iAttr.c_time = time(&now);
    iAttr.group   = 0;
    iAttr.owner   = 0;
    iAttr.iMode   = POSIXTOHFSMODE(mode);
    iAttr.linkCount  = 1;
    iAttr.size   = 0;
    iAttr.rdev_t  = fs_dev_t;
    if (HFS_S_IFDIR == mode) iAttr.linkCount++;   // Directories have 2 dirents //

    status = hfs_setIAttr(pClientFileHdl,&iAttr,MDS_ATTR_MASK_DH);
    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR("Cannot Create data handles for parent %s for file %s",ppath,fileName);
        return status;
    }

    //- Insert into the namespace of the parent -//
    childDirent.dirEntMode = POSIXTOHFSMODE(mode);
    strncpy((char *)childDirent.dname,fileName,HFS_MAX_FILE_NAME);
    childDirent.parentMetaDataHandle = parentDirHandle.metaDataHandle;
    childDirent.selfMetaDataHandle = pClientFileHdl->metaDataHandle;
    childDirent.serverId    = pClientFileHdl->serverId;

    status = hfs_add_dirent(&parentDirHandle,pClientFileHdl,&childDirent);
    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR("Cannot create dirent %s %s",ppath,fileName);
        return status;
    }

    HFS_LEAVE();
    return status;
}

HFS_STATUS
hfs_lookup(PHFS_CLIENT_FILE_HDL pHfsClientHdl,
           const char *dname,
           OUT PHFS_CLIENT_FILE_HDL pHfsChildHdl)
{
    HFS_STATUS   status = HFS_STATUS_SUCCESS;
    HFS_PROTO_HEADER header;
    PHFS_QUEUE_ITEM  pQueueItem;
    PHFS_SERVER_REQ  pServReq;
    PHFS_SERVER_RESP pServResp;

    INIT_HEADER(&header,CMD_REQ_MDS_LOOK_UP);

    // Create the packet and setup the correct pointers//
    pQueueItem = hfsAllocateProtoBuff(&header,getHFSFSCtx()->stripeSize);
    if (!pQueueItem) {
        HFS_LOG_ERROR("Server low on memory or bad header");
        return HFS_PROTOCOL_ERROR;
    }

    pServResp = (PHFS_SERVER_RESP)(pQueueItem + 1);
    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);


    pServReq->req.reqMDSLookUp.parentMetaDataHandle = pHfsClientHdl->metaDataHandle;
    pServReq->req.reqMDSLookUp.serverId    = pHfsClientHdl->serverId;
    strncpy((char *)pServReq->req.reqMDSLookUp.dname,dname,HFS_MAX_FILE_NAME);

    // Send out the packet //
    pQueueItem->clientIdx = GET_MDS_SERVERS_CLIENT_IDX(pHfsClientHdl->serverId);
    status = hfsQueueItem(getHFSClient()->kernel.pQOutBound,pQueueItem);
    if (!HFS_SUCCESS(status)) {
        hfsFreeProtoBuff(pQueueItem);
        HFS_LOG_ERROR("Cannot queue item on out bound");
        return HFS_PROTOCOL_ERROR;
    }

    // Wait for the response //
    hfsWaitQItem(pQueueItem);

    // Do the operation on the backup if things failed //
    if (!HFS_SUCCESS(pServResp->hdr.status)) {
        //-Sadly rebuild the packet-//
        INIT_HEADER(&pServReq->hdr,CMD_REQ_MDS_LOOK_UP);
        pServReq->req.reqMDSLookUp.parentMetaDataHandle = pHfsClientHdl->metaDataHandle;
        pServReq->req.reqMDSLookUp.serverId    = pHfsClientHdl->serverId;
        strncpy((char *)pServReq->req.reqMDSLookUp.dname,dname,HFS_MAX_FILE_NAME);

        status = hfsDegradedMDSRequest(pQueueItem);
        if (!HFS_SUCCESS(status))
            HFS_LOG_ERROR("Failed retry from backups");
    }


    if (HFS_SUCCESS(pServResp->hdr.status)) {
        pHfsChildHdl->metaDataHandle = pServResp->resp.respMDSLookUp.selfHandle;
        pHfsChildHdl->serverId  = pServResp->resp.respMDSLookUp.serverId;
    }
    status = pServResp->hdr.status;
    hfsFreeProtoBuff(pQueueItem);
    return status;
}

HFS_STATUS
hfs_getIAttr(PHFS_CLIENT_FILE_HDL pHfsClientHdl,
             PHFS_IATTR pHfsIAttr)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    HFS_PROTO_HEADER header;
    PHFS_QUEUE_ITEM pQueueItem;
    PHFS_SERVER_REQ pServReq;
    PHFS_SERVER_RESP pServResp;

    INIT_HEADER(&header,CMD_REQ_MDS_GET_ATTR);

    // Create the packet and setup the correct pointers//
    pQueueItem = hfsAllocateProtoBuff(&header,getHFSFSCtx()->stripeSize);
    if (!pQueueItem) {
        HFS_LOG_ERROR("Server low on memory or bad header");
        return HFS_PROTOCOL_ERROR;
    }

    pServResp = (PHFS_SERVER_RESP)(pQueueItem + 1);
    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);


    // Build the packet //
    pServReq->req.reqMDSGetAttr.selfMetaDataHandle = pHfsClientHdl->metaDataHandle;

    // Send out the packet //
    pQueueItem->clientIdx = GET_MDS_SERVERS_CLIENT_IDX(pHfsClientHdl->serverId);
    status = hfsQueueItem(getHFSClient()->kernel.pQOutBound,pQueueItem);
    if (!HFS_SUCCESS(status)) {
        hfsFreeProtoBuff(pQueueItem);
        HFS_LOG_ERROR("Cannot queue item on out bound");
        return HFS_PROTOCOL_ERROR;
    }

    // Wait for the response //
    hfsWaitQItem(pQueueItem);

    // Do the operation on the backup if things failed //
    if (!HFS_SUCCESS(pServResp->hdr.status)) {
        //-Sadly rebuild the packet-//
        INIT_HEADER(&pServReq->hdr,CMD_REQ_MDS_GET_ATTR);
        pServReq->req.reqMDSGetAttr.selfMetaDataHandle = pHfsClientHdl->metaDataHandle;

        status = hfsDegradedMDSRequest(pQueueItem);
        if (!HFS_SUCCESS(status))
            HFS_LOG_ERROR("Failed retry from backups");
    }

    if (HFS_SUCCESS(pServResp->hdr.status)) {
        status = hfs_getISize(&pServResp->resp.respMDSGetAttr.attr);

        if (!HFS_SUCCESS(status)) {
            HFS_LOG_ERROR("Could not get the size for the file correctly");
            return status;
        }

        //- copy the inode attributes -//
        *pHfsIAttr = pServResp->resp.respMDSGetAttr.attr;
    }
    status = pServResp->hdr.status;
    hfsFreeProtoBuff(pQueueItem);
    return status;
}


HFS_STATUS
hfs_getISize(PHFS_IATTR pHfsIAttr)
{
    HFS_STATUS   status = HFS_STATUS_SUCCESS;
    int     i;
    PHFS_QUEUE_ITEM *ppGetSizeItems;
    HFS_PROTO_HEADER header;
    PHFS_SERVER_REQ  pServReq;
    PHFS_SERVER_RESP pServResp;

    HFS_ENTRY();

    INIT_HEADER(&header,CMD_REQ_DS_GET_LENGTH);


    if (HFS_S_IFDIR == POSIXTOHFSMODE(pHfsIAttr->iMode)) {
        pHfsIAttr->size = 4096;
        return HFS_STATUS_SUCCESS;
    }

    // Number of extents to read //
    ppGetSizeItems = hfsCalloc(pHfsIAttr->stripe_cnt * sizeof(PHFS_QUEUE_ITEM));
    if (!(ppGetSizeItems)) {
        status = HFS_LOW_RESOURCES;
        goto cleanup;
    }

    // Allocate the protobuff for each of the stripe //
    for(i=0;i<pHfsIAttr->stripe_cnt;i++){

        ppGetSizeItems[i] = hfsAllocateProtoBuff(&header,getHFSFSCtx()->stripeSize);
        if (!ppGetSizeItems[i]) {
            status = HFS_LOW_RESOURCES;
            goto cleanup;
        }
        pServReq = (PHFS_SERVER_REQ) (ppGetSizeItems[i] + 1);
        pServResp = (PHFS_SERVER_RESP)(ppGetSizeItems[i] + 1);

        pServReq->req.reqDSGetLength.serverId = pHfsIAttr->dataHandlesSrvs[i];
        pServReq->req.reqDSGetLength.dataHandle = pHfsIAttr->dataHandles[i];
        pServReq->req.reqDSGetLength.extentSize = 0;


        //- Now we have to save the index into the inode for which this get length was issued -//
        //- So the degraded get length can use this index + stripcount to get the backup extents --//
        // _ ORIG __ []
        // _ ORIG __ [] <-- = "save this into the cookie".
        // _ ORIG __ [] |
        // _ ORIG __ [] + stripecnt
        // _ BCKUP __[] |
        // _ BCKUP __[] <--
        // _ BCKUP __[]
        // _ BCKUP __[]
        pServReq->hdr.cookies[COOKIE_DATA_SERVER_IDX] = i;


        // go to the right ds //
        ppGetSizeItems[i]->clientIdx =
            GET_DS_SERVERS_CLIENT_IDX(pServReq->req.reqDSReadStripe.serverId);

    }


    // Send out Reads //
    for(i=0;i<pHfsIAttr->stripe_cnt;i++){
        status = hfsQueueItem(getHFSClient()->kernel.pQOutBound,ppGetSizeItems[i]);
        if (!HFS_SUCCESS(status)) {
            HFS_LOG_ERROR("FOUCH Memory was wasted as incomplete reads were queue");
            return HFS_PROTOCOL_ERROR;
        }
    }

    // Wait for reads to complete //
    for(i=0;i<pHfsIAttr->stripe_cnt;i++)
        hfsWaitQItem(ppGetSizeItems[i]);


    // If we have failed to read the size of one file try to read from the backup//
    for(i=0;i<pHfsIAttr->stripe_cnt;i++){
        pServResp = (PHFS_SERVER_RESP)(ppGetSizeItems[i] + 1);
        if (HFS_SUCCESS(pServResp->hdr.status))
            continue;

        HFS_LOG_ERROR("Read Failed to read get length of an extent");
        //- Try doing a degraded read
        // This will surely succeed if only 1 server is down
        // If more than 1 servers are down it will return a zero filled buffer
        // We log such an event but do server 0 bytes data to the client.
        status = hfsDegradedDSRequest(
                                      ppGetSizeItems[i],
                                      pHfsIAttr);
        if (!HFS_SUCCESS(status)) {
            HFS_LOG_ERROR("Cannot get the reconstruced size for file %d",i);
            HFS_LOG_ERROR("Client file size will be short");
            status = HFS_STATUS_SUCCESS;
        }
    }



    // calculate the length now //
    pHfsIAttr->size = 0;
    for(i=0;i<pHfsIAttr->stripe_cnt;i++){
        /* to the header */
        pServResp = (PHFS_SERVER_RESP)(ppGetSizeItems[i] + 1);
        // Add to file length if we were successful //
        if (HFS_SUCCESS(pServResp->hdr.status)) {
            pServResp = (PHFS_SERVER_RESP)(ppGetSizeItems[i] + 1);
            pHfsIAttr->size += pServResp->resp.respDSGetLength.extentSize;
        }
        else {
            HFS_LOG_ERROR("One of the extents failed to be report size %x",(unsigned int)pServResp->hdr.status);
        }
    }

    status = HFS_STATUS_SUCCESS;
 cleanup:
    // free all the protobuffs //
    for(i=0;i<pHfsIAttr->stripe_cnt;i++)
        hfsFreeProtoBuff(ppGetSizeItems[i]);

    // Free the map to the protobuffs //
    if (ppGetSizeItems)
        hfsFree(ppGetSizeItems);

    HFS_LEAVE();
    return status;
}




HFS_STATUS
hfs_setIAttr(PHFS_CLIENT_FILE_HDL pHfsClientHdl,PHFS_IATTR pHfsAttr,__u32 attrMask)
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    HFS_PROTO_HEADER header;
    PHFS_QUEUE_ITEM pQueueItem;
    PHFS_SERVER_REQ pServReq;
    PHFS_SERVER_RESP pServResp;

    INIT_HEADER(&header,CMD_REQ_MDS_SET_ATTR);

    // Create the packet and setup the correct pointers//
    pQueueItem = hfsAllocateProtoBuff(&header,getHFSFSCtx()->stripeSize);
    if (!pQueueItem) {
        HFS_LOG_ERROR("Server low on memory or bad header");
        return HFS_PROTOCOL_ERROR;
    }

    pServResp = (PHFS_SERVER_RESP)(pQueueItem + 1);
    pServReq = (PHFS_SERVER_REQ) (pQueueItem + 1);


    // Build the packet //
    pServReq->req.reqMDSSetAttr.attr    = *pHfsAttr;
    pServReq->req.reqMDSSetAttr.attrMask   = attrMask;

    // Send out the packet //
    pQueueItem->clientIdx = GET_MDS_SERVERS_CLIENT_IDX(pHfsClientHdl->serverId);
    status = hfsQueueItem(getHFSClient()->kernel.pQOutBound,pQueueItem);
    if (!HFS_SUCCESS(status)) {
        hfsFreeProtoBuff(pQueueItem);
        HFS_LOG_ERROR("Cannot queue item on out bound");
        return HFS_PROTOCOL_ERROR;
    }

    // Wait for the response //
    hfsWaitQItem(pQueueItem);

    // Do the operation on the backup if things failed //
    if (!HFS_SUCCESS(pServResp->hdr.status)) {
        //-Sadly rebuild the packet-//
        INIT_HEADER(&pServReq->hdr,CMD_REQ_MDS_SET_ATTR);
        pServReq->req.reqMDSSetAttr.attr    = *pHfsAttr;
        pServReq->req.reqMDSSetAttr.attrMask   = attrMask;

        status = hfsDegradedMDSRequest(pQueueItem);
        if (!HFS_SUCCESS(status))
            HFS_LOG_ERROR("Failed retry from backups");
    }

    status = pServResp->hdr.status;
    hfsFreeProtoBuff(pQueueItem);
    return status;
}


// This does not fill int the pHfsClienthdl->iAttr field //
HFS_STATUS
hfs_namei(char    *name,
          PHFS_CLIENT_FILE_HDL pHfsClienthdl)
{
    char    *componentName;
    char    *prev;
    HFS_CLIENT_FILE_HDL hfsClientChildhdl;
    char    *tempName;

    HFS_STATUS status = HFS_STATUS_SUCCESS;

    pHfsClienthdl->metaDataHandle = HFS_ROOT_HANDLE;
    pHfsClienthdl->serverId  = HFS_ROOT_SERVER_IDX;
    if (strlen(name) == 1 && name[0] == '/') {
        return HFS_STATUS_SUCCESS;
    }

    //-- Make the filename temporary --//
    tempName = NULL;
    tempName = hfsCalloc(strlen(name)+1);
    if (!tempName)
        return HFS_LOW_RESOURCES;

    memcpy(tempName,name,strlen(name));
    // For each component do a name i //
    prev = tempName;
    while(1) {
        componentName = strtok_r(prev,"/",&prev);
        //-- Process Component Name --//
        status = hfs_lookup(pHfsClienthdl,componentName,&hfsClientChildhdl);
        if (!HFS_SUCCESS(status)) {
            HFS_LOG_ERROR("Lookup failed");
            if (tempName)
                hfsFree(tempName);
            return status;
        }
        pHfsClienthdl->metaDataHandle = hfsClientChildhdl.metaDataHandle;
        pHfsClienthdl->serverId = hfsClientChildhdl.serverId;
        if (*prev == '\0') break;
    }

    if (tempName)
        hfsFree(tempName);

    return status;
}



//- RAID 5 Interfaces -//
HFS_STATUS
hfsRaid5GetBlockMapping(int     LCN,
                        PHFS_IATTR   piAttr,
                        PHFS_RAID5_BLK_MAP pHfsRaid5blockMap)
{
    HFS_ENTRY();
    pHfsRaid5blockMap->dataServerIdx  = LCN % piAttr->stripe_cnt;
    pHfsRaid5blockMap->dataStripeOffset = LCN / piAttr->stripe_cnt;
    HFS_LOG_INFO("LCN MAPPING \n LCN = %d"
                 " \n DIDX = %d DOFF = %d",
                 LCN,
                 pHfsRaid5blockMap->dataServerIdx,
                 pHfsRaid5blockMap->dataStripeOffset);
    HFS_LEAVE();
    return HFS_STATUS_SUCCESS;
}

//-- Care was taken to keep the packet preserved in machine order for this
// function --//
HFS_STATUS
hfsDegradedDSRequest(PHFS_QUEUE_ITEM pFailedItems,
                     PHFS_IATTR  piAttr)
{
    HFS_STATUS   status;
    int     backupReadDataServerIdx;
    PHFS_SERVER_REQ  pServReq;
    HFS_ENTRY();

    pServReq = (PHFS_SERVER_REQ) (pFailedItems + 1);

    // _ ORIG __ []
    // _ ORIG __ [] <-- = "save this into the cookie".
    // _ ORIG __ [] |
    // _ ORIG __ [] + stripecnt
    // _ BCKUP __[] |
    // _ BCKUP __[] <--
    // _ BCKUP __[]
    // _ BCKUP __[]

    backupReadDataServerIdx =
        pServReq->hdr.cookies[COOKIE_DATA_SERVER_IDX]+piAttr->stripe_cnt;

    // Map to the backup here //
    pServReq->req.reqDSReadStripe.serverId = piAttr->dataHandlesSrvs[backupReadDataServerIdx];
    pServReq->req.reqDSReadStripe.dataHandle = piAttr->dataHandles[backupReadDataServerIdx];
    pFailedItems->clientIdx    = GET_DS_SERVERS_CLIENT_IDX(pServReq->req.reqDSReadStripe.serverId);

    // Send out Backup request //
    status = hfsQueueItem(getHFSClient()->kernel.pQOutBound,pFailedItems);
    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR("FOUCH Memory was wasted as incomplete reads were queue");
        return HFS_PROTOCOL_ERROR;
    }

    // Wait for reads to complete //
    hfsWaitQItem(pFailedItems);
    HFS_LEAVE();

    // Return the status for the packet that was read from the backup //
    return pServReq->hdr.status;
}


//-- Care was taken to keep the packet preserved in machine order for this
// function --//
#define BACK_UP_MDS_SKIP_OFFSET   1
HFS_STATUS
hfsDegradedMDSRequest(PHFS_QUEUE_ITEM pFailedItems)
{
    HFS_STATUS   status;
    PHFS_SERVER_REQ  pServReq;
    HFS_ENTRY();

    pServReq = (PHFS_SERVER_REQ) (pFailedItems + 1);

    // MDS backup layout
    // _ ORIG __ [] <- + 1
    // _ BCKUP __[] <- - +
    // _ ORIG __ []
    // _ BCKUP __[]
    // _ ORIG __ []
    // _ BCKUP __[]
    // _ ORIG __ []
    // _ BCKUP __[]

    pFailedItems->clientIdx += BACK_UP_MDS_SKIP_OFFSET;

    // Send out Backup request //
    status = hfsQueueItem(getHFSClient()->kernel.pQOutBound,pFailedItems);
    if (!HFS_SUCCESS(status)) {
        HFS_LOG_ERROR("FOUCH Memory was wasted as incomplete reads were queue");
        return HFS_PROTOCOL_ERROR;
    }

    // Wait for reads to complete //
    hfsWaitQItem(pFailedItems);
    HFS_LEAVE();

    // Return the status for the packet that was read from the backup //
    return pServReq->hdr.status;
}
