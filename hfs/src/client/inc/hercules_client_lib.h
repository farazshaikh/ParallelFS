/*
 * hercules_client_lib.h
 *
 * Hercules client library header file
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

#pragma pack(push,1)
typedef struct __HFS_CLIENT_FILE_HDL {
    HFS_SERVER_ID serverId;
    HFS_META_DATA_HANDLE metaDataHandle;
    HFS_IATTR iAttr;
}HFS_CLIENT_FILE_HDL,*PHFS_CLIENT_FILE_HDL;
#pragma pack(pop)

__u32 POSIXTOHFSMODE(mode_t m);
mode_t HFSMODETOPOSIX(__u32 hfs_mode);

//- data -path ops -//
HFS_STATUS hfs_getattr(const char *ppath,
                       struct stat *stat_buff);

HFS_STATUS hfs_read(PHFS_CLIENT_FILE_HDL pClientFileHdl,
                    char *buffer,
                    off_t offset,
                    IN OUT size_t *size);

HFS_STATUS hfs_write(PHFS_CLIENT_FILE_HDL pClientFileHdl,
                     const char *buffer,
                     off_t offset,
                     IN OUT size_t *size);


// Name space ops //
HFS_STATUS hfs_namei(char *name,
                     PHFS_CLIENT_FILE_HDL pHfsClienthdl);

HFS_STATUS hfs_getIAttr(PHFS_CLIENT_FILE_HDL pHfsClientHdl,
                        PHFS_IATTR pHfsIAttr);

HFS_STATUS hfs_getISize(PHFS_IATTR pHfsIAttr);

HFS_STATUS hfs_setIAttr(PHFS_CLIENT_FILE_HDL pHfsClientHdl,
                        PHFS_IATTR pHfsAttr,
                        __u32 attrMask);

HFS_STATUS hfs_readdir(PHFS_CLIENT_FILE_HDL pClientFileHdl,
                       off_t offset,
                       PHFS_QUEUE_ITEM *ppRetQueueItem);

// - Creation ops -//
HFS_STATUS hfs_create(char *ppath,
                      mode_t mode,
                      __u32 fs_dev_t,
                      PHFS_CLIENT_FILE_HDL pClientFileHdl);

HFS_STATUS hfs_create_alloc_data_handle(PHFS_IATTR piAttr);

HFS_STATUS hfs_create_alloc_meta_data_handles(PHFS_IATTR piAttr,
                                              PHFS_CLIENT_FILE_HDL pClientFileHdl);

//- This is trickier don't touch it Then namespace is distributed here -//
HFS_STATUS hfs_add_dirent(PHFS_CLIENT_FILE_HDL parentFileHdl,
                          PHFS_CLIENT_FILE_HDL childFileHdl,
                          PHFS_DIRENT pDirent);

//- Client operations -//
HFS_STATUS hfsBuildClientKernel(PHERCULES_CLIENT pHerculesClient,
                                PHFS_FS_CTX phFSFsCtx);

//-- Client RAID 5 Interfaces --//
typedef struct __hfsRaid5blkMap {
    HFS_SERVER_ID dataServerIdx;
    __u32 dataStripeOffset;
    HFS_SERVER_ID parityServerIdx;
    __u32 parityStripeOffset;
} HFS_RAID5_BLK_MAP, *PHFS_RAID5_BLK_MAP;


HFS_STATUS hfsDegradedDSRequest(PHFS_QUEUE_ITEM pFailedItems,
                                PHFS_IATTR piAttr);

HFS_STATUS hfsDegradedMDSRequest(PHFS_QUEUE_ITEM pFailedItems);


HFS_STATUS hfsRaid5GetBlockMapping(int LCN,
                                   PHFS_IATTR piAttr,
                                   PHFS_RAID5_BLK_MAP pHfsRaid5blockMap);

HFS_STATUS hfsRaid5XORBuffers(PHFS_QUEUE_ITEM *ppReadExtentItems,
                              int extentCount,
                              int extentSize);

HFS_STATUS hfsRaid5DegradedRead(PHFS_QUEUE_ITEM pReadFailedItems,
                                PHFS_IATTR piAttr,
                                int extentSize);
