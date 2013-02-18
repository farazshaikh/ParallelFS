/*
 * hercules_types.h
 *
 *      Hercules wire protocol
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

#ifndef __PROTO_COMMON_H__
#define __PROTO_COMMON_H__

#pragma pack(push,1)

//-- Field Size constants --//
#define HFS_MAX_FILE_NAME       255
#define HFS_MIN_MDS_CNT         1
#define HFS_MAX_MDS_CNT         16
#define HFS_MIN_STRIPE_CNT      3
#define HFS_MAX_STRIPE_CNT      16
#define HFS_MIN_DS_CNT          HFS_MIN_STRIPE_CNT
#define HFS_MAX_DS_CNT          32
#define HFS_MIN_STRIPE_SIZE     1024
#define HFS_MAX_STRIPE_SIZE     32768
#define HFS_INVALID_HANDLE      0x00

#define BACK_INTERLEAVE_FACTOR  1
#define TOTAL_COPIES            2


//-- Root Boot Strapping --//
#define HFS_INVALID_HANDLE      0x00
#define HFS_ROOT_HANDLE         0x10
#define HFS_ROOT_SERVER_IDX     0x00

//- Boot Strapping Info for the data handle -//
#define HFS_FIRST_DATA_HANDLE               0x100
#define HFS_DATA_SERVER_META_FILE_HANDLE    0x000

//-- File System Configuration --//
#define HFS_MAX_HOST_NAME       90
#define HFS_MAX_IP_ADDRESS      16
#define HFS_MAX_PORT_STR        10
#define HFS_MAX_PROTO_STR       4
#define HFS_MAX_DATA_STORE_PATH 255
#define HFS_MAX_LOG_FILE_PATH   255
#define HFS_MAX_FILE_SYSTEM_NAME 255


//- Dirent Stuff Smells and runs like EXT2       --//
#define HFS_S_IFMT                  0xF0000000  //format mask
#define HFS_S_IFSOCK                0xC0000000  //socket
#define HFS_S_IFLNK                 0xA0000000  //symbolic link
#define HFS_S_IFREG                 0x80000000  //regular file
#define HFS_S_IFBLK                 0x60000000  //block device
#define HFS_S_IFDIR                 0x40000000  //directory
#define HFS_S_IFCHR                 0x20000000  //character device
#define HFS_S_IFIFO                 0x10000000  //fifo
#define HFS_S_IFUNKNOWN             0x00000000  //unknown file type

//--Hercules DS Commands       --//
#define CMD_REQ_DS_PING                 (0x1UL)
#define CMD_RESP_DS_PING                (0x2UL)

#define CMD_REQ_DS_ALLOC_HANDLE         (0x3UL)
#define CMD_RESP_DS_ALLOC_HANDLE        (0x4UL)

#define CMD_REQ_DS_FREE_HANDLE          (0x5UL)
#define CMD_RESP_DS_FREE_HANDLE         (0x6UL)

#define CMD_REQ_DS_READ_STRIPE          (0x7UL)
#define CMD_RESP_DS_READ_STRIPE         (0x8UL)

#define CMD_REQ_DS_WRITE_STRIPE         (0x9UL)
#define CMD_RESP_DS_WRITE_STRIPE        (0xAUL)

#define CMD_REQ_DS_READ_STRIPE_V        (0xBUL)
#define CMD_RESP_DS_READ_STRIPE_V       (0xCUL)

#define CMD_REQ_DS_WRITE_STRIPE_V       (0xDUL)
#define CMD_RESP_DS_WRITE_STRIPE_V      (0xEUL)

#define CMD_REQ_DS_GET_LENGTH           (0xFUL)
#define CMD_RESP_DS_GET_LENGTH          (0x10UL)

#define CMD_REQ_DS_GET_DISK_USAGE       (0x11UL)
#define CMD_RESP_DS_GET_DISK_USAGE      (0x12UL)

#define CMD_REQ_DS_GET_STATS            (0x13UL)
#define CMD_RESP_DS_GET_STATS           (0x14UL)
//--End Hercules DS    --//




//--Hercules MDS Commands       --//
#define CMD_REQ_MDS_PING                (0x21UL)
#define CMD_RESP_MDS_PING               (0x22UL)

#define CMD_REQ_MDS_ALLOC_HANDLE        (0x23UL)
#define CMD_RESP_MDS_ALLOC_HANDLE       (0x24UL)

#define CMD_REQ_MDS_FREE_HANDLE         (0x25UL)
#define CMD_RESP_MDS_FREE_HANDLE        (0x26UL)

#define CMD_REQ_MDS_READDIR             (0x27UL)
#define CMD_RESP_MDS_READDIR            (0x28UL)

#define CMD_REQ_MDS_CREATE_DIRENT       (0x29UL)
#define CMD_RESP_MDS_CREATE_DIRENT      (0x2AUL)

#define CMD_REQ_MDS_DELETE_DIRENT       (0x2BUL)
#define CMD_RESP_MDS_DELETE_DIRENT      (0x2CUL)

#define CMD_REQ_MDS_LOOK_UP             (0x2DUL)
#define CMD_RESP_MDS_LOOK_UP            (0x2EUL)

#define CMD_REQ_MDS_GET_ATTR            (0x2FUL)
#define CMD_RESP_MDS_GET_ATTR           (0x30UL)

#define CMD_REQ_MDS_SET_ATTR            (0x31UL)
#define CMD_RESP_MDS_SET_ATTR           (0x32UL)

#define CMD_REQ_MDS_GET_EXTENT_SIZE     (0x33UL)
#define CMD_RESP_MDS_GET_EXTENT_SIZE    (0x34UL)

#define CMD_REQ_MDS_GET_CONFIG          (0x35UL)
#define CMD_RESP_MDS_GET_CONFIG         (0x36UL)

#define CMD_REQ_MDS_LOGIN_PHASE1        (0x37UL)
#define CMD_RESP_MDS_LOGIN_PHASE1       (0x38UL)

#define CMD_REQ_MDS_LOGIN_PHASE2        (0x39UL)
#define CMD_RESP_MDS_LOGIN_PHASE2       (0x3AUL)


//--Hercules MDS Callbacks/Notifications      --//
#define CMD_REQ_MDS_FS_CONF_CHANGED     (0x51UL)  // Never used !!
#define CMD_RESP_MDS_FS_CONF_CHANGED    (0x52UL)
//--EndCmdMDS  --//


//-- Handle definitions --//
typedef __u32         HFS_SERVER_ID;
typedef __u32         HFS_META_DATA_HANDLE,HFS_DATA_HANDLE;


//-- DS Server Requests/Response --//
//#define DS_PING              (BASE_DS_CMD_START)
#define LAST_ACT_DS_READ       0x1
#define LAST_ACT_DS_WRITE      LAST_ACT_DS_READ     << 0x1
#define LAST_ACT_MDS_READ      LAST_ACT_DS_WRITE    << 0x1
#define LAST_ACT_MDS_WRITE     LAST_ACT_MDS_READ    << 0x1
#define LAST_ACT_MASK          0xF
typedef struct __REQ_DS_PING{
    __u32   cookie;
}REQ_DS_PING,PREQ_DS_PING;
typedef struct __RESP_DS_PING {
    __u32   cookie;
}RESP_DS_PING,*PRESP_DS_PING;

//#define DS_ALLOC_HANDLE          (BASE_DS_CMD_START+0x1)
typedef struct __REQ_DS_ALLOC_HANDLE{
  HFS_SERVER_ID   serverId;
  HFS_DATA_HANDLE dataHandle;
}REQ_DS_ALLOC_HANDLE,*PREQ_DS_ALLOC_HANDLE;
typedef struct __RESP_{
  HFS_SERVER_ID   serverId;
  HFS_DATA_HANDLE dataHandle;
}RESP_DS_ALLOC_HANDLE,*PRESP_DS_ALLOC_HANDLE;

//#define DS_FREE_HANDLE           (BASE_DS_CMD_START+0x2)
typedef struct __REQ_DS_FREE_HANDLE{
  HFS_SERVER_ID   serverId;
  HFS_DATA_HANDLE dataHandle;
}REQ_DS_FREE_HANDLE,*PREQ_DS_FREE_HANDLE;
typedef struct __RESP_DS_FREE_HANDLE{
  HFS_SERVER_ID   serverId;
  HFS_DATA_HANDLE dataHandle;
}RESP_DS_FREE_HANDLE,*PRESP_DS_FREE_HANDLE;

//#define DS_READ_STRIPE           (BASE_DS_CMD_START+0x3)
typedef struct __REQ_DS_READ_STRIPE{
  HFS_SERVER_ID   serverId;
  HFS_DATA_HANDLE dataHandle;
  __u32           stripeNum;
  __u32           stripeCnt;
  __u32           readSize;
}REQ_DS_READ_STRIPE,*PREQ_DS_READ_STRIPE;
typedef struct __RESP_DS_READ_STRIPE{
  HFS_SERVER_ID   serverId;
  HFS_DATA_HANDLE dataHandle;
  __u32           stripeNum;
  __u32           stripeCnt;
  __u32           readSize;
}RESP_DS_READ_STRIPE,*PRESP_DS_READ_STRIPE;

//#define DS_WRITE_STRIPE          (BASE_DS_CMD_START+0x4)
typedef struct __REQ_DS_WRITE_STRIPE{
  HFS_SERVER_ID   serverId;
  HFS_DATA_HANDLE dataHandle;
  __u32           stripeNum;
  __u32           stripeCnt;
  __u32           writeSize;
  __u32           writeInStripeOffset;
}REQ_DS_WRITE_STRIPE,*PREQ_DS_WRITE_STRIPE;
typedef struct __RESP_DS_WRITE_STRIPE{
  HFS_SERVER_ID   serverId;
  HFS_DATA_HANDLE dataHandle;
  __u32           stripeNum;
  __u32           stripeCnt;
  __u32           writeSize;
  __u32           writeInStripeOffset;
}RESP_DS_WRITE_STRIPE,*PRESP_DS_WRITE_STRIPE;

//#define DS_READ_STRIPE_V          (BASE_DS_CMD_START+0x5)
typedef struct __REQ_DS_READ_STRIPE_V{
}REQ_DS_READ_STRIPE_V,*PREQ_DS_READ_STRIPE_V;
typedef struct __RESP_DS_READ_STRIPE_V{
}RESP_DS_READ_STRIPE_V,*PRESP_DS_READ_STRIPE_V;

//#define DS_WRITE_STRIPE_V          (BASE_DS_CMD_START+0x6)
typedef struct __REQ_DS_WRITE_STRIPE_V{
}REQ_DS_WRITE_STRIPE_V,*PREQ_DS_WRITE_STRIPE_V;
typedef struct __RESP_DS_WRITE_STRIPE_V{
}RESP_DS_WRITE_STRIPE_V,*PRESP_DS_WRITE_STRIPE_V;

//#define DS_GET_LENGTH              (BASE_DS_CMD_START+0x7)
typedef struct __REQ_DS_GET_LENGTH{
  HFS_SERVER_ID   serverId;
  HFS_DATA_HANDLE dataHandle;
  __u32           extentSize;
}REQ_DS_GET_LENGTH,*PREQ_DS_GET_LENGTH;
typedef struct __RESP_DS_GET_LENGTH{
  HFS_SERVER_ID   serverId;
  HFS_DATA_HANDLE dataHandle;
  __u32           extentSize;
}RESP_DS_GET_LENGTH,*PRESP_DS_GET_LENGTH;

//#define DS_GET_DISK_USAGE          (BASE_DS_CMD_START+0x8)
typedef struct __REQ_DS_GET_DISK_USAGE{
}REQ_DS_GET_DISK_USAGE,*PREQ_DS_GET_DISK_USAGE;
typedef struct __RESP_DS_GET_DISK_USAGE{
}RESP_DS_GET_DISK_USAGE,*PRESP_DS_GET_DISK_USAGE;

//#define DS_GET_STATS               (BASE_DS_CMD_START+0x9)
typedef struct __REQ_DS_GET_STATS{
}REQ_DS_GET_STATS,*PREQ_DS_GET_STATS;
typedef struct __RESP_DS_GET_STATS{
}RESP_DS_GET_STATS,*PRESP_DS_GET_STATS;


//-- MDS Server Requests --//
//#define MDS_PING                (BASE_MDS_CMD_START)
typedef struct __REQ_MDS_PING{
    __u32   cookie;
}REQ_MDS_PING,*PREQ_MDS_PING;
typedef struct __RESP_MDS_PING{
    __u32   cookie;
}RESP_MDS_PING,*PRESP_MDS_PING;

//#define MDS_ALLOC_HANDLE        (BASE_MDS_CMD_START + 0x1)
typedef struct __REQ_MDS_ALLOC_HANDLE{
    HFS_SERVER_ID           serverId;
    HFS_META_DATA_HANDLE    metaDataHandle;
}REQ_MDS_ALLOC_HANDLE,*PREQ_MDS_ALLOC_HANDLE;
typedef struct __RESP_MDS_ALLOC_HANDLE{
    HFS_SERVER_ID           serverId;
    HFS_META_DATA_HANDLE    metaDataHandle;
}RESP_MDS_ALLOC_HANDLE,*PRESP_MDS_ALLOC_HANDLE;

//#define MDS_FREE_HANDLE         (BASE_MDS_CMD_START + 0x2)
typedef struct __REQ_MDS_FREE_HANDLE{
  HFS_SERVER_ID         serverId;
  HFS_META_DATA_HANDLE  metaDataHandle;
}REQ_MDS_FREE_HANDLE,*PREQ_MDS_FREE_HANDLE;
typedef struct __RESP_MDS_FREE_HANDLE{
  HFS_SERVER_ID         serverId;
  HFS_META_DATA_HANDLE  metaDataHandle;
}RESP_MDS_FREE_HANDLE,*PRESP_MDS_FREE_HANDLE;

//#define MDS_READDIR             (BASE_MDS_CMD_START + 0x3)
typedef struct __REQ_MDS_READDIR{
  HFS_META_DATA_HANDLE  metaDataHandle;
  __u32                 direntOffset;
}REQ_MDS_READDIR,*PREQ_MDS_READDIR;
typedef struct __RESP_MDS_READDIR{
  HFS_META_DATA_HANDLE  metaDataHandle;
  __u32                 direntCount;
}RESP_MDS_READDIR,*PRESP_MDS_READDIR;

typedef struct  __HFS_DIRENT {
    HFS_META_DATA_HANDLE    parentMetaDataHandle;
    HFS_META_DATA_HANDLE    selfMetaDataHandle;
    HFS_SERVER_ID           serverId;
    __u8                    dname[HFS_MAX_FILE_NAME];
    __u32                   dirEntMode;
}HFS_DIRENT,*PHFS_DIRENT;

//#define MDS_CREATE_DIRENT       (BASE_MDS_CMD_START + 0x4)
typedef struct __REQ_MDS_CREATE_DIRENT{
    HFS_DIRENT              hfsDirent;
}REQ_MDS_CREATE_DIRENT,*PREQ_MDS_CREATE_DIRENT;
typedef struct __RESP_MDS_CREATE_DIRENT{
    __u32                   cookie;
}RESP_MDS_CREATE_DIRENT,*PRESP_MDS_CREATE_DIRENT;

//#define MDS_DELETE_DIRENT       (BASE_MDS_CMD_START + 0x5)
typedef struct __REQ_MDS_DELETE_DIRENT{
    HFS_META_DATA_HANDLE    parentMetaDataHandle;
    HFS_META_DATA_HANDLE    selfMetaDataHandle;
}REQ_MDS_DELETE_DIRENT,*PREQ_MDS_DELETE_DIRENT;
typedef struct __RESP_MDS_DELETE_DIRENT{
    __u32                   cookie;
}RESP_MDS_DELETE_DIRENT,*PRESP_MDS_DELETE_DIRENT;

//#define MDS_LOOK_UP             (BASE_MDS_CMD_START + 0x6)
typedef struct __REQ_MDS_LOOK_UP{
    HFS_META_DATA_HANDLE    parentMetaDataHandle;
    __u32                   serverId;
    __u8                    dname[HFS_MAX_FILE_NAME];
}REQ_MDS_LOOK_UP,*PREQ_MDS_LOOK_UP;
typedef struct __RESP_MDS_LOOK_UP{
    HFS_META_DATA_HANDLE    selfHandle;
    HFS_SERVER_ID           serverId;
}RESP_MDS_LOOK_UP,*PRESP_MDS_LOOK_UP;

//#define MDS_GET_ATTR            (BASE_MDS_CMD_START + 0x7)
#define MDS_ATTR_MASK_DH          0x00000001
typedef struct __HFS_IATTR {
    HFS_META_DATA_HANDLE    selfMetaDataHandle;
    __u32                   iMode;
    __u32                   owner;
    __u32                   linkCount;
    __u32                   group;
    __u32                   a_time;
    __u32                   c_time;
    __u32                   m_time;
    __u32                   size;
    __u32                   stripe_cnt;
    __u32                   rdev_t;
    HFS_SERVER_ID           dataHandlesSrvs[HFS_MAX_STRIPE_CNT * TOTAL_COPIES]; // 1 + 1 backup
    HFS_META_DATA_HANDLE    dataHandles[HFS_MAX_STRIPE_CNT * TOTAL_COPIES];     // 1 + 1 backup
}HFS_IATTR,*PHFS_IATTR;

typedef struct __REQ_MDS_GET_ATTR{
    HFS_META_DATA_HANDLE    selfMetaDataHandle;
}REQ_MDS_GET_ATTR,*PREQ_MDS_GET_ATTR;
typedef struct __RESP_MDS_GET_ATTR{
    HFS_IATTR attr;
}RESP_MDS_GET_ATTR,*PRESP_MDS_GET_ATTR;

//#define MDS_SET_ATTR            (BASE_MDS_CMD_START + 0x8)
typedef struct __REQ_MDS_SET_ATTR{
    HFS_IATTR attr;
    __u32     attrMask;
}REQ_MDS_SET_ATTR,*PREQ_MDS_SET_ATTR;
typedef struct __RESP_MDS_SET_ATTR{
    __u32    cookie;
}RESP_MDS_SET_ATTR,*PRESP_MDS_SET_ATTR;

//#define MDS_GET_EXTENT_SIZE   (BASE_MDS_CMD_START + 0x9)
typedef struct __REQ_MDS_GET_EXTENT_SIZE{
    __u32   unused;
}REQ_MDS_GET_EXTENT_SIZE,*PREQ_MDS_GET_EXTENT_SIZE;
typedef struct __RESP_MDS_GET_EXTENT_SIZE{
    __u32   extent_size;
}RESP_MDS_GET_EXTENT_SIZE,*PRESP_MDS_GET_EXTENT_SIZE;

//#define MDS_GET_CONFIG    (BASE_MDS_CMD_START + 0xa)
typedef struct __REQ_MDS_GET_CONFIG{
  __u32                 srvrentOffset;
}REQ_MDS_GET_CONFIG,*PREQ_MDS_GET_CONFIG;
typedef struct __RESP_MDS_GET_CONFIG{
  __u32                 srvrentCount;
}RESP_MDS_GET_CONFIG,*PRESP_MDS_GET_CONFIG;

//#define MDS_GET_CONFIG    (BASE_MDS_CMD_START + 0xa)
typedef struct __REQ_MDS_FS_CONF_CHANGED{
  __u32                 padding;
}REQ_MDS_FS_CONF_CHANGED,*PREQ_MDS_FS_CONF_CHANGED;
typedef struct __RESP_MDS_FS_CONF_CHANGED{
  __u32                 padding;
}RESP_MDS_FS_CONF_CHANGED,*PRESP_MDS_FS_CONF_CHANGED;


//#define MDS_LOGIN_PHASE1        (0x37UL)
#define HFS_MAX_USER_NAME         8
#define HFS_MAX_ENC_PASSWORD      8         // To make it run on POSIX systems
                                            // HFS_MAX_ENC_PASSWORD < PASS_MAX
typedef struct __REQ_MDS_LOGIN_PHASE1{
  char          user[HFS_MAX_USER_NAME];
}REQ_MDS_LOGIN_PHASE1,*PREQ_MDS_LOGIN_PHASE1;
typedef struct __RESP_MDS_LOGIN_PHASE1{
  union {
  __u32                 encryptedNonce_high;
  __u32                 encryptedNonce_low;
  __u64                 encryptedNonce;
  }u;
}RESP_MDS_LOGIN_PHASE1,*PRESP_MDS_LOGIN_PHASE1;

//#define MDS_LOGIN_PHASE2        (0x39UL)
typedef struct __REQ_MDS_LOGIN_PHASE2{
  char                  user[HFS_MAX_USER_NAME];
  union {
  __u32                 decryptedNonce_high;
  __u32                 decryptedNonce_low;
  __u64                 dencryptedNonce;
  }u;
}REQ_MDS_LOGIN_PHASE2,*PREQ_MDS_LOGIN_PHASE2;
typedef struct __RESP_MDS_LOGIN_PHASE2{
  __u32                 padding;
}RESP_MDS_LOGIN_PHASE2,*PRESP_MDS_LOGIN_PHASE2;




typedef struct _HFS_SERVER_INFO  {
    char    hostName[HFS_MAX_HOST_NAME];
    char    ipAddr[HFS_MAX_IP_ADDRESS];
    char    port[HFS_MAX_PORT_STR];
    char    proto[HFS_MAX_PROTO_STR];
    char    dataStorePath[HFS_MAX_DATA_STORE_PATH];
    char    logFile[HFS_MAX_LOG_FILE_PATH];
    char    filesystemName[HFS_MAX_FILE_SYSTEM_NAME];
    int     serverIdx;
    int     role;
}HFS_SERVER_INFO,*PHFS_SERVER_INFO;


//-- Protocol Header --//
#define COOKIE_COUNT            4
#define CREDS_COUNT             4
#define HFS_PROTO_MAGIC         (*((__u32 *)".HFS"))
typedef struct __HFS_PROTO_HEADER {
    __u32   protoMagic;
    __u32   version;
    __u32   command;
    __u32   pduExtentSize;          // Variable tail length if any All have to set this//
    __u32   status;
    __u32   credentials[CREDS_COUNT];
    __u64   cookies[COOKIE_COUNT];  // we save pointers here
}HFS_PROTO_HEADER;

typedef struct __HFS_SERVER_REQ {
    HFS_PROTO_HEADER hdr;
    union {
        REQ_DS_PING             reqDSPing;
        REQ_DS_ALLOC_HANDLE     reqDSAllocHandle;
        REQ_DS_FREE_HANDLE      reqDSFreeHandle;
        REQ_DS_READ_STRIPE      reqDSReadStripe;
        REQ_DS_WRITE_STRIPE     reqDSWriteStripe;
        REQ_DS_READ_STRIPE_V    reqDSReadStripeV;
        REQ_DS_WRITE_STRIPE_V   reqDSWriteStripeV;
        REQ_DS_GET_LENGTH       reqDSGetLength;
        REQ_DS_GET_DISK_USAGE   reqDSGetDiskUsage;
        REQ_DS_GET_STATS        reqDSGetstats;
        REQ_MDS_PING            reqMDSPing;
        REQ_MDS_ALLOC_HANDLE    reqMDSAllocHandle;
        REQ_MDS_FREE_HANDLE     reqMDSFreeHandle;
        REQ_MDS_READDIR         reqMDSReaddir;
        REQ_MDS_CREATE_DIRENT   reqMDSCreateDirent;
        REQ_MDS_DELETE_DIRENT   reqMDSDeleteDirent;
        REQ_MDS_LOOK_UP         reqMDSLookUp;
        REQ_MDS_GET_ATTR        reqMDSGetAttr;
        REQ_MDS_SET_ATTR        reqMDSSetAttr;
        REQ_MDS_GET_EXTENT_SIZE reqMDSGetExtentSize;
        REQ_MDS_GET_CONFIG      reqMDSGetConfig;
        REQ_MDS_FS_CONF_CHANGED reqMDSFsConfChanged;
        REQ_MDS_LOGIN_PHASE1    reqMDSLoginPhase1;
        REQ_MDS_LOGIN_PHASE2    reqMDSLoginPhase2;
    }req;
}HFS_SERVER_REQ,*PHFS_SERVER_REQ;



typedef struct __HFS_SERVER_RESP {
    HFS_PROTO_HEADER hdr;
    union {
        RESP_DS_PING            respDSPing;
        RESP_DS_ALLOC_HANDLE    respDSAllocHandle;
        RESP_DS_FREE_HANDLE     respDSFreeHandle;
        RESP_DS_READ_STRIPE     respDSReadStripe;
        RESP_DS_WRITE_STRIPE    respDSWriteStripe;
        RESP_DS_READ_STRIPE_V   respDSReadStripeV;
        RESP_DS_WRITE_STRIPE_V  respDSWriteStripeV;
        RESP_DS_GET_LENGTH      respDSGetLength;
        RESP_DS_GET_DISK_USAGE  respDSGetDiskUsage;
        RESP_DS_GET_STATS       respDSGetstats;
        RESP_MDS_PING           respMDSPing;
        RESP_MDS_ALLOC_HANDLE   respMDSAllocHandle;
        RESP_MDS_FREE_HANDLE    respMDSFreeHandle;
        RESP_MDS_READDIR        respMDSReaddir;
        RESP_MDS_CREATE_DIRENT  respMDSCreateDirent;
        RESP_MDS_DELETE_DIRENT  respMDSDeleteDirent;
        RESP_MDS_LOOK_UP        respMDSLookUp;
        RESP_MDS_GET_ATTR       respMDSGetAttr;
        RESP_MDS_SET_ATTR       respMDSSetAttr;
        RESP_MDS_GET_EXTENT_SIZE    respMDSGetExtentSize;
        RESP_MDS_GET_CONFIG         respMDSGetConfig;
        RESP_MDS_FS_CONF_CHANGED    respMDSFsConfChanged;
        RESP_MDS_LOGIN_PHASE1       respMDSLoginPhase1;
        RESP_MDS_LOGIN_PHASE2       respMDSLoginPhase2;
    }resp;
}HFS_SERVER_RESP,*PHFS_SERVER_RESP;

//- Protocol Error Codes -//
#define HFS_PROTO_ERROR_MASK                  0x80000000
#define HFS_PROTO_STATUS_SUCCESS              0x0
#define HFS_PROTO_STATUS_NOT_IMPLEMENTED      (HFS_PROTO_ERROR_MASK | 0x1)
#define HFS_PROTO_STATUS_FATAL_ERROR          (HFS_PROTO_ERROR_MASK | 0x2)
#define HFS_PROTO_ERROR_BAD_PATH              (HFS_PROTO_ERROR_MASK | 0x3)
#define HFS_PROTO_AUTHENTICATION_FAILURE      (HFS_PROTO_ERROR_MASK | 0x4)


#pragma pack(pop)
#endif //__PROTO_COMMON_H__
/*
REQ_DS_PING
RESP_DS_PING

REQ_DS_ALLOC_HANDLE
RESP_DS_ALLOC_HANDLE

REQ_DS_FREE_HANDLE
RESP_DS_FREE_HANDLE

REQ_DS_READ_STRIPE
RESP_DS_READ_STRIPE

REQ_DS_WRITE_STRIPE
RESP_DS_WRITE_STRIPE

REQ_DS_READ_STRIPE_V
RESP_DS_READ_STRIPE_V

REQ_DS_WRITE_STRIPE_V
RESP_DS_WRITE_STRIPE_V

REQ_DS_GET_LENGTH
RESP_DS_GET_LENGTH

REQ_DS_GET_DISK_USAGE
RESP_DS_GET_DISK_USAGE

REQ_DS_GET_STATS
RESP_DS_GET_STATS

MDS
REQ_MDS_PING
RESP_MDS_PING

REQ_MDS_ALLOC_HANDLE
RESP_MDS_ALLOC_HANDLE

REQ_MDS_FREE_HANDLE
RESP_MDS_FREE_HANDLE

REQ_MDS_READDIR
RESP_MDS_READDIR

REQ_MDS_CREATE_DIRENT
RESP_MDS_CREATE_DIRENT

REQ_MDS_DELETE_DIRENT
RESP_MDS_DELETE_DIRENT

REQ_MDS_LOOK_UP
RESP_MDS_LOOK_UP

REQ_MDS_GET_ATTR
RESP_MDS_GET_ATTR

REQ_MDS_SET_ATTR
RESP_MDS_SET_ATTR

REQ_MDS_GET_EXTENT_SIZE
RESP_MDS_GET_EXTENT_SIZE

REQ_MDS_GET_CONFIG
RESP_MDS_GET_CONFIG

REQ_MDS_FS_CONF_CHANGED
RESP_MDS_FS_CONF_CHANGED

REQ_MDS_LOGIN_PHASE1
RESP_MDS_LOGIN_PHASE1

REQ_MDS_LOGIN_PHASE2
RESP_MDS_LOGIN_PHASE2
*/
