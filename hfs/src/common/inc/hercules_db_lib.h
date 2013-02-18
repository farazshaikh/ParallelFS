/*
 * client_common.h
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



#ifndef _HERCULES_DB_H_
#define _HERCULES_DB_H_

#include <linux/types.h>
#include <mysql.h>

#define IN
#define OUT


// String Format specifiers for data base strings //
#define HFS_DB_HOST_NAME    80
#define HFS_MAX_QUERY       512


// Format Specifiers for queries //
#define FMT_QUERY_CHECK_FS          "use %s"
#define FMT_QUERY_DROP_DATABASE     "drop database %s"
#define FMT_QUERY_CREATE_FS         "create database %s"
#define FMT_QUERY_CREATE_INODE_TABLE   \
    "CREATE TABLE inode (\
    metadata_handle INT UNSIGNED NOT NULL,\
        imode INT UNSIGNED NOT NULL,\
        owner INT UNSIGNED NOT NULL,\
        link_count INT UNSIGNED NOT NULL,\
        grp INT UNSIGNED NOT NULL,\
        a_time INT UNSIGNED NOT NULL,\
        m_time INT UNSIGNED NOT NULL,\
        c_time INT UNSIGNED NOT NULL,\
        size   INT UNSIGNED NOT NULL,\
        stripe_cnt INT UNSIGNED NOT NULL,\
        rdev_t INT UNSIGNED NOT NULL,\
    primary key(metadata_handle))"
#define FMT_QUERY_CREATE_DATA_TABLE \
    "CREATE TABLE data_handle (\
    metadata_handle INT UNSIGNED NOT NULL,\
        data_handle INT UNSIGNED NOT NULL,\
        server_id SMALLINT UNSIGNED NOT NULL,\
    FOREIGN KEY (metadata_handle) references inode(metadata_handle))"
#define FMT_QUERY_CREATE_FREE_HANDLE_TABLE \
    "CREATE TABLE metadata_handle (\
                                   metadata_handle INT UNSIGNED NOT NULL)"
#define FMT_QUERY_CREATE_FILE_SYSTEM_CONFIGURATION \
    "CREATE TABLE fs_config (\
    hostname CHAR("TOSTR(HFS_MAX_HOST_NAME)") NOT NULL,\
        server_id SMALLINT UNSIGNED NOT NULL,\
        ip_addr CHAR("TOSTR(HFS_MAX_IP_ADDRESS)") NOT NULL,\
        port SMALLINT UNSIGNED NOT NULL,\
        proto CHAR("TOSTR(HFS_MAX_PROTO_STR)") NOT NULL,\
        data_path VARCHAR("TOSTR(HFS_MAX_DATA_STORE_PATH)") NOT NULL,\
        log_file VARCHAR("TOSTR(HFS_MAX_LOG_FILE_PATH)") NOT NULL,\
        role SMALLINT UNSIGNED NOT NULL)"
#define FMT_QUERY_CREATE_NAME_SPACE \
    "CREATE TABLE namespace (\
    parent_handle INT UNSIGNED NOT NULL,\
        self_handle   INT UNSIGNED NOT NULL, \
        mode          INT UNSIGNED NOT NULL, \
        namespace     CHAR("TOSTR(HFS_MAX_FILE_NAME)") NOT NULL,\
        serverId      INT UNSIGNED NOT NULL, \
        FOREIGN KEY  (parent_handle) references inode(metadata_handle))"
#define FMT_QUERY_CREATE_GEN_ID_TABLE \
    "CREATE TABLE generation_id \
    (generation_id INT UNSIGNED NOT NULL)"
#define FMT_QUERY_CREATE_STRIPE_SIZE_TABLE \
    "CREATE TABLE stripe_size \
    (stripeSize INT UNSIGNED NOT NULL)"
#define FMT_QUERY_CREATE_USER_TABLE \
    "CREATE TABLE fs_users (\
    user_name CHAR("TOSTR(HFS_MAX_USER_NAME)") NOT NULL,\
        password  CHAR("TOSTR(HFS_MAX_ENC_PASSWORD)") NOT NULL,\
        primary key(user_name))"
#define FMT_QUERY_GET_ENCRYPTED_PASS \
    "select password from fs_users \
    where user_name = '%s'"

                                                   //- Processing Queries -//
#define FMT_QUERY_MDS_READDIR \
    "Select parent_handle,self_handle,mode,namespace \
    from namespace where parent_handle = %d"
#define FMT_QUERY_MDS_LOOKUP \
    "Select self_handle,serverid \
        from namespace where parent_handle = %d and namespace = '%s'"
#define FMT_QUERY_MDS_GET_ATTR \
    "select metadata_handle,imode,owner,link_count,grp,a_time,m_time,c_time,size,stripe_cnt,rdev_t \
    from inode \
    where  metadata_handle=%d"
#define FMT_QUERY_MDS_SET_ATTR \
   "update inode "\
   "set " \
   "imode=%d, "\
   "owner=%d, "\
   "link_count=%d, "\
   "grp=%d, "\
   "a_time=%d, "\
   "m_time=%d, "\
   "c_time=%d, "\
   "size=%d, "\
   "stripe_cnt=%d, "\
   "rdev_t=%d "\
   "where metadata_handle=%d"

#define FMT_QUERY_ADD_INODE \
    "insert into inode(metadata_handle,imode,owner,link_count,grp,a_time,m_time,c_time,size,stripe_cnt,rdev_t)\
    values(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)"
#define FMT_QUERY_MDS_GET_DATA_HANDLES \
    "select server_id,data_handle \
    from data_handle \
    where  metadata_handle=%d"
#define FMT_QUERY_MDS_SET_DATA_HANDLES \
    "insert into data_handle(metadata_handle,server_id,data_handle) \
    values(%d,%d,%d)"
#define FMT_QUERY_MDS_GET_EXTENT_SIZE \
    "select stripeSize \
    from stripe_size"
#define FMT_QUERY_MDS_GET_CONFIG \
    "select hostname,server_id,ip_addr,port,proto,data_path,log_file,role \
    from fs_config where role=0\
    union \
        select hostname,server_id,ip_addr,port,proto,data_path,log_file,role \
    from fs_config where role=2\
    union \
        select hostname,server_id,ip_addr,port,proto,data_path,log_file,role \
    from fs_config where role=1"
                                                   // Insert into namespace //
#define FMT_QUERY_ADD_DIRENT \
    "insert into namespace(parent_handle,self_handle,mode,serverId,namespace)\
    values(%d,%d,%d,%d,'%s')"
// Insert into config table //
#define FMT_QUERY_ADD_SRVR_CONFIG \
    "insert into fs_config(hostname,server_id,ip_addr,port,proto,data_path,log_file,role)\
    values('%s',%d,'%s',%d,'%s','%s','%s',%d)"
// Insert into stripe size table //
#define FMT_QUERY_ADD_STRIPE_SIZE \
    "insert into stripe_size(stripeSize)\
    values(%d)"
                                                   // Insert into fs users //
#define FMT_QUERY_ADD_USER \
    "insert into fs_users(user_name,password)\
    values('%s','%s')"

// Search for free metadata handle //
#define FMT_QUERY_GET_MAX_META_HANDLE \
    "select MAX(metadata_handle) from inode"
#define FMT_QUERY_REMOVE_FREE_META_HANDLE \
    "remove from metadata_handle where metadata_handle = %d"
#define FMT_QUERY_ADD_FREE_META_HANDLE \
    "insert into metadata_handle(metadata_handle) values(%d)"
#define FMT_QUERY_GET_MIN_FREE_META_HANDLE \
    "select MIN(metadata_handle) from metadata_handle"

                                                   // Utilities //
#define BUILD_QUERY(query,fmt,vargs...) do { \
    memset(query,0,HFS_MAX_QUERY); \
    snprintf(query,HFS_MAX_QUERY,fmt,##vargs); \
    }while(0)





                                                   typedef struct __HFS_DB_CTX {
                                                       // Connection info required by mysql client --//
                                                       MYSQL     *pConn;
                                                       char      filesystemName[HFS_MAX_FILE_SYSTEM_NAME];
                                                   }HFS_DB_CTX,*PHFS_DB_CTX;

HFS_STATUS dbFileSystemExist(PHFS_DB_CTX pDbCtx);
HFS_STATUS dbOpenConnection  (PHFS_DB_CTX pDbCtx);
HFS_STATUS dbFormatMDS       (PHFS_DB_CTX pDbCtx,int serverIdx, PHFS_FS_CTX pHFSFSCtx);
HFS_STATUS dbCloseConnection (PHFS_DB_CTX pDbCtx);
HFS_STATUS dbGetError        (PHFS_DB_CTX pDbCtx, char * errorStr, size_t strSize);

// Queries required for working //
HFS_STATUS dbGetDirListing(PHFS_DB_CTX pDbCtx,
                           __u32       parent_handle,
                           char        *buffer,
                           int         size,
                           IN __u32            offset,
                           IN OUT __u32       *copiedDirent);


HFS_STATUS dbLookUp(PHFS_DB_CTX             pDbCtx,
                    __u32                   parent_handle,
                    char                   *dname,
                    HFS_META_DATA_HANDLE   *pHFSMetaDataHandle,
                    __u32                  *serverId);

HFS_STATUS dbGetAttr(PHFS_DB_CTX pDbCtx,
                     HFS_META_DATA_HANDLE,
                     PHFS_IATTR pIAttr);

HFS_STATUS dbSetAttr(PHFS_DB_CTX             pDbCtx,
                     PHFS_IATTR              pIAttr,
                     __u32                   attrMask);

HFS_STATUS dbAllocMetaDataHandle(PHFS_DB_CTX pDbCtx,
                                 HFS_META_DATA_HANDLE *pMetaDataHandle);

HFS_STATUS dbFreeHandle(PHFS_DB_CTX             pDbCtx,
                        HFS_META_DATA_HANDLE    metaDataHandle);

HFS_STATUS dbAddDirent(PHFS_DB_CTX     pDbCtx,
                       PHFS_DIRENT     pDirent);

HFS_STATUS dbGetExtentSize(PHFS_DB_CTX             pDbCtx,
                           __u32                  *extentSize);

HFS_STATUS dbGetSrvrListing(PHFS_DB_CTX pDbCtx,
                            char        *buffer,
                            int         size,
                            IN __u32            offset,
                            IN OUT __u32       *copiedDirent);

#define    HFS_CRYPT_SALT "hf"
HFS_STATUS dbGetEncryptedNonce(PHFS_DB_CTX             pDbCtx,
                               char                   *userName,
                               __u64                  *randomNonce,
                               __u64                  *encryptedRandomNonce);

HFS_STATUS __executeQuery(PHFS_DB_CTX pDbCtx,
                          char *sql_query);

#endif // _HERCULES_DB_H_
