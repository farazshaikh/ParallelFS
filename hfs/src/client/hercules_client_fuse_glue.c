/*
 * hercules_client_fuse_glue.c
 *
 *      fuse glue on top of hercules client that talks to meta data
 *      and data servers to provide filesystem services
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

#define FUSE_USE_VERSION 26
#include <hercules_common.h>
#include <hercules_client.h>
#include <hercules_client_lib.h>
#include <fuse.h>

static int clnt_fuse_open(const char *path, struct fuse_file_info *fi);

HFS_STATUS
SAVE_CLIENT_FILE_HANDLE(uint64_t *fh,
                        PHFS_CLIENT_FILE_HDL pClientFileHandle)
{
    HFS_STATUS status;

    // Save the update pClientFileHandle in the fh
    status = hfs_getIAttr(pClientFileHandle,&pClientFileHandle->iAttr);
    if (HFS_SUCCESS(status)) {
        *((uint64_t*)fh) = (uint64_t) pClientFileHandle; // Fix up the compiler warning
    } else {
        *((uint64_t*)fh) = (uint64_t) 0x0;   // Fix up the compiler warning
    }

    return status;
}

HFS_STATUS
GET_CLIENT_FILE_HANDLE(uint64_t *fh,
                       PHFS_CLIENT_FILE_HDL *ppclientFileHandle)
{
    // Get back the pointer to the client file handle from the file handle
    *ppclientFileHandle = (PHFS_CLIENT_FILE_HDL) *fh;

    if (!*ppclientFileHandle) {
        HFS_LOG_ERROR("Coding logic for the saving client side handle has bugs");
        return HFS_PROTOCOL_ERROR;
    }

    return HFS_STATUS_SUCCESS;
}

//- handler for SIGPIPE -//
void handler1(int sig)
{
    HFS_ENTRY();
    HFS_LOG_INFO("Client got a sig pipe");
    HFS_LEAVE();
    return;
}

static int
clnt_fuse_getattr(const char *path,
                  struct stat *stbuf)
{
    HFS_STATUS status;

    HFS_ENTRY();

    status = hfs_getattr(path, stbuf);
    if (HFS_STATUS_DB_NO_SUCH_RECORD == status) {
        errno = ENOENT;
        return -errno;
    }else if (!HFS_SUCCESS(status)) {
        return -EREMOTEIO;
    }

    HFS_LEAVE();
    return 0;
}


static int
clnt_fuse_access(const char *path, int mask)
{
    HFS_ENTRY();
    /*res = access(path, mask);
      if (res == -1)
      return -errno; */
    HFS_LEAVE();
    return 0;
}


static int
clnt_fuse_readlink(const char *path, char *buf, size_t size)
{
    int res;
    HFS_ENTRY();

    memset(buf, 0, size);
    res = readlink(path, buf, size - 1);
    if (res == -1) {
      return -errno;
    }

    HFS_LEAVE();
    return 0;
}


static int
clnt_fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                  off_t offset, struct fuse_file_info *fi)
{
    HFS_STATUS status;
    __u32     direntsPerExtent, direntsReturned;
    PHFS_DIRENT   pHFSDirent;
    PHFS_QUEUE_ITEM  pQueueItem;
    PHFS_SERVER_RESP pServResp;
    HFS_CLIENT_FILE_HDL clientFileHandle;
    HFS_ENTRY();

    status = hfs_namei((char *)path, &clientFileHandle);
    if (!HFS_SUCCESS(status)) {
        return -ENOENT;
    }

    status = hfs_readdir(&clientFileHandle, offset, &pQueueItem);
    if (!HFS_SUCCESS(status)) {
        return -EREMOTEIO;
    }

    // Got things from the server give it now to fuse //
    pServResp = (PHFS_SERVER_RESP)(pQueueItem + 1);
    direntsPerExtent = getHFSFSCtx()->stripeSize / sizeof(HFS_DIRENT);
    direntsReturned = pServResp->resp.respMDSReaddir.direntCount;

    // return the dirent names only //
    pHFSDirent = (PHFS_DIRENT)
        ((char*)pServResp + sizeof(pServResp->hdr) + sizeof(pServResp->resp.respMDSReaddir));
    while(direntsReturned && direntsPerExtent) {

        if (filler(buf, (char *)pHFSDirent->dname, NULL, ++offset)) {
            break;
        }

        pHFSDirent++;
        direntsReturned--;
        direntsPerExtent--;
    }

    HFS_LEAVE();
    hfsFreeProtoBuff(pQueueItem);
    return 0;
}


static int
clnt_fuse_mknod(const char *path, mode_t mode, dev_t rdev)
{
    HFS_CLIENT_FILE_HDL clientFileHandle;
    HFS_STATUS   status;
    HFS_ENTRY();
    // lookup the path
    status = hfs_namei((char *)path, &clientFileHandle);


    if (HFS_SUCCESS(status)) {
        return -EEXIST;
    } else {
        //-- Lookup failed and asked to create a new file --//
        //-- Create new file with the given mode --//
        status = hfs_create((char *)path, mode, (__u32)rdev, &clientFileHandle);
        if (!HFS_SUCCESS(status)) {
            return -EREMOTEIO;
        }
    }
    HFS_LEAVE();
    return 0;
}


static int
clnt_fuse_mkdir(const char *path, mode_t mode)
{
    HFS_CLIENT_FILE_HDL clientFileHandle;
    HFS_STATUS   status;
    HFS_ENTRY();

    // lookup the path
    status = hfs_namei((char *)path, &clientFileHandle);
    if (HFS_SUCCESS(status)) {
        return -EEXIST;
    }else {
        //-- Lookup failed and asked to create a new file --//
        //-- Create new file with the given mode --//
        status = hfs_create((char *)path, (S_IFDIR | 0755), 0, &clientFileHandle);
        if (!HFS_SUCCESS(status)) {
            return -EREMOTEIO;
        }
    }

    HFS_LEAVE();
    return 0;
}

static int
clnt_fuse_unlink(const char *path)
{
    HFS_ENTRY();

    /*res = unlink(path);
      if (res == -1)
      return -errno;*/

    HFS_LEAVE();
    return 0;
}

static int
clnt_fuse_rmdir(const char *path)
{
    HFS_ENTRY();

    /*res = rmdir(path);
      if (res == -1)
      return -errno;*/

    HFS_LEAVE();
    return 0;
}

static int
clnt_fuse_symlink(const char *from, const char *to)
{
    HFS_ENTRY();

    /*res = symlink(from, to);
      if (res == -1)
      return -errno;*/

    HFS_LEAVE();
    return 0;
}

static int
clnt_fuse_rename(const char *from, const char *to)
{
    HFS_ENTRY();

    /*res = rename(from, to);
      if (res == -1)
      return -errno;*/

    HFS_LEAVE();
    return 0;
}

static int
clnt_fuse_link(const char *from, const char *to)
{
    HFS_ENTRY();

    /* res = link(from, to);
       if (res == -1)
       return -errno; */

    HFS_LEAVE();
    return 0;
}

static int
clnt_fuse_chmod(const char *path, mode_t mode)
{
    HFS_ENTRY();

    /* res = chmod(path, mode);
       if (res == -1)
       return -errno; */

    HFS_LEAVE();
    return 0;
}

static int
clnt_fuse_chown(const char *path, uid_t uid, gid_t gid)
{
    HFS_ENTRY();

    /* res = lchown(path, uid, gid);
       if (res == -1)
       return -errno; */

    HFS_LEAVE();
    return 0;
}

static int
clnt_fuse_truncate(const char *path, off_t size)
{
    HFS_ENTRY();

    /* res = truncate(path, size);
       if (res == -1)
       return -errno; */

    HFS_LEAVE();
    return 0;
}

static int
clnt_fuse_utimens(const char *path, const struct timespec ts[2])
{
    int res;
    struct timeval tv[2];
    HFS_ENTRY();

    tv[0].tv_sec = ts[0].tv_sec;
    tv[0].tv_usec = ts[0].tv_nsec / 1000;
    tv[1].tv_sec = ts[1].tv_sec;
    tv[1].tv_usec = ts[1].tv_nsec / 1000;

    res = utimes(path, tv);
    if (res == -1) {
        // ignore utime update errors
    }

    HFS_LEAVE();
    return 0;
}

#define IS_FLAG_SET(flag, mask) ((flag) & (mask))
static int
clnt_fuse_open(const char *path, struct fuse_file_info *fi)
{
    PHFS_CLIENT_FILE_HDL pClientFileHandle;
    HFS_STATUS   status;
    HFS_ENTRY();

    //- Allocate it and then do go ahead -//
    pClientFileHandle = hfsCalloc(sizeof(*pClientFileHandle));
    if (pClientFileHandle == NULL) {
        return -ENOMEM;
    }

    // lookup the path
    status = hfs_namei((char *)path, pClientFileHandle);

    if (HFS_SUCCESS(status)) {
        // Lookup succeeded //
        if (!IS_FLAG_SET(fi->flags, O_CREAT)) {
            //-- Lookup succeeded and not asked to create a new file --//
            status = SAVE_CLIENT_FILE_HANDLE(&fi->fh, pClientFileHandle);
            if (!HFS_SUCCESS(status)) {
                return -EREMOTEIO;
            }

            return 0;
        }

        if (IS_FLAG_SET(fi->flags, O_CREAT) && IS_FLAG_SET(fi->flags, O_EXCL)) {
            //-- Lookup succeeded asked to create and O_EXCL flag was specified --//
            return -EEXIST;
        }

        if (IS_FLAG_SET(fi->flags, O_CREAT)) {
            //-- Lookup succeeded asked to create O_EXCL was not specified --//
            //-- TODO Truncate the file --//
            status = SAVE_CLIENT_FILE_HANDLE(&fi->fh, pClientFileHandle);
            if (!HFS_SUCCESS(status)) {
                return -EREMOTEIO;
            }
            return 0;
        }
    } else {

        // Lookup failed //
        if (!IS_FLAG_SET(fi->flags, O_CREAT)) {
            //-- Lookup failed and not asked to create --//
            return -ENOENT;
        }

        if (IS_FLAG_SET(fi->flags, O_CREAT)) {
            //-- Lookup failed and asked to create a new file --//
            //-- Create new file --//
            HFS_LOG_ERROR("This is fuse bug !!");
            status = hfs_create((char *)path, (S_IFREG | 0444), 0, pClientFileHandle);
            if (!HFS_SUCCESS(status)) {
                return -EREMOTEIO;
            }
        }
    }

    // Lookup succeeded and
    // Save the client file handle for further use //
    status = SAVE_CLIENT_FILE_HANDLE(&fi->fh, pClientFileHandle);
    if (!HFS_SUCCESS(status)) {
        return -EREMOTEIO;
    }

    HFS_LEAVE();
    return 0;
}

static int
clnt_fuse_read(const char *path, char *buf, size_t size, off_t offset,
               struct fuse_file_info *fi)
{
    PHFS_CLIENT_FILE_HDL pClientFileHandle;
    HFS_STATUS   status;
    HFS_ENTRY();

    GET_CLIENT_FILE_HANDLE(&fi->fh, &pClientFileHandle);
    if (NULL == pClientFileHandle ||
        HFS_INVALID_HANDLE == pClientFileHandle->metaDataHandle) {
        return -EBADFD;
    }

    status = hfs_read(pClientFileHandle, buf, offset, &size);
    if (!HFS_SUCCESS(status)) {
        return -EREMOTEIO;
    } else {
        return size;
    }

    HFS_LEAVE();
    return 0;
}


static int
clnt_fuse_write(const char *path, const char *buf, size_t size,
                off_t offset, struct fuse_file_info *fi)
{
    PHFS_CLIENT_FILE_HDL pClientFileHandle;
    HFS_STATUS   status;
    HFS_ENTRY();

    GET_CLIENT_FILE_HANDLE(&fi->fh, &pClientFileHandle);
    if (NULL == pClientFileHandle ||
        HFS_INVALID_HANDLE == pClientFileHandle->metaDataHandle) {
        return -EBADFD;
    }

    status = hfs_write(pClientFileHandle, buf, offset, &size);
    if (!HFS_SUCCESS(status)) {
        return -EREMOTEIO;
    } else {
        return size;
    }

    HFS_LEAVE();
    return 0;
}

static int
clnt_fuse_statfs(const char *path, struct statvfs *stbuf)
{
    int res;
    HFS_ENTRY();

    res = statvfs(path, stbuf);
    if (res == -1) {
        return -errno;
    }

    HFS_LEAVE();
    return 0;
}

static int
clnt_fuse_release(const char *path, struct fuse_file_info *fi)
{
    PHFS_CLIENT_FILE_HDL pClientFileHandle;
    /* Just a stub.  This method is optional and can safely be left
       unimplemented */

    HFS_ENTRY();
    GET_CLIENT_FILE_HANDLE(&fi->fh, &pClientFileHandle);
    if (NULL == pClientFileHandle ||
        HFS_INVALID_HANDLE == pClientFileHandle->metaDataHandle) {
        return 0;  // No returning errors from this function
    }

    hfsFree(pClientFileHandle); // Free the cached handle //
    HFS_LEAVE();
    return 0;
}

static int
clnt_fuse_fsync(const char *path, int isdatasync,
                struct fuse_file_info *fi)
{
    /* Just a stub.  This method is optional and can safely be left
       unimplemented */
    HFS_ENTRY();
    (void) path;
    (void) isdatasync;
    (void) fi;
    HFS_LEAVE();
    return 0;
}

void *
clnt_fuse_init (struct fuse_conn_info *conn)
{
    HFS_STATUS status;
    HFS_ENTRY();
    //-- Build up the Server Infra-structure
    //-- These are the core data-structures need for operations--//
    status = hfsBuildClientKernel(getHFSClient(), getHFSFSCtx());
    if (status == HFS_STATUS_AUTH_FAILED) {
        exit(255);
    }

    if (!HFS_SUCCESS(status)) {
        return NULL;
    }

    HFS_LEAVE();
    return NULL;
}


#ifdef HAVE_SETXATTR
/* xattr operations are optional and can safely be left unimplemented */
static int
clnt_fuse_setxattr(const char *path, const char *name, const char *value,
                   size_t size, int flags)
{
    int res = lsetxattr(path, name, value, size, flags);
    if (res == -1) {
        return -errno;
    }

    return 0;
}

static int
clnt_fuse_getxattr(const char *path, const char *name, char *value,
                   size_t size)
{
    int res = lgetxattr(path, name, value, size);
    if (res == -1) {
        return -errno;
    }

    return res;
}

static int
clnt_fuse_listxattr(const char *path, char *list, size_t size)
{
    int res = llistxattr(path, list, size);

    if (res == -1) {
        return -errno;
    }

    return res;
}

static int
clnt_fuse_removexattr(const char *path, const char *name)
{
    int res = lremovexattr(path, name);

    if (res == -1) {
        return -errno;
    }

    return 0;
}
#endif /* HAVE_SETXATTR */

static struct fuse_operations clnt_fuse_oper = {
    .getattr = clnt_fuse_getattr,
    .access = clnt_fuse_access,
    .readlink = clnt_fuse_readlink,
    .readdir = clnt_fuse_readdir,
    .mknod = clnt_fuse_mknod,
    .mkdir = clnt_fuse_mkdir,
    .symlink = clnt_fuse_symlink,
    .unlink = clnt_fuse_unlink,
    .rmdir = clnt_fuse_rmdir,
    .rename = clnt_fuse_rename,
    .link = clnt_fuse_link,
    .chmod = clnt_fuse_chmod,
    .chown = clnt_fuse_chown,
    .truncate = clnt_fuse_truncate,
    .utimens = clnt_fuse_utimens,
    .open = clnt_fuse_open,
    .read = clnt_fuse_read,
    .write = clnt_fuse_write,
    .statfs = clnt_fuse_statfs,
    .release = clnt_fuse_release,
    .fsync = clnt_fuse_fsync,
    .init = clnt_fuse_init,
#ifdef HAVE_SETXATTR
    .setxattr = clnt_fuse_setxattr,
    .getxattr = clnt_fuse_getxattr,
    .listxatt = clnt_fuse_listxattr,
    .removexattr= clnt_fuse_removexattr,
#endif
};


HFS_STATUS
hfsFuseInit(int argc, char *argv[])
{
    HFS_STATUS status = HFS_STATUS_SUCCESS;
    HFS_ENTRY();
    HFS_LOG_INFO("All MDS and DS Connected\n");
    umask(0);

    status = fuse_main(argc, argv, &clnt_fuse_oper, NULL);
    if (status) {
        HFS_LOG_ERROR("Fuse Initialization failed");
    }

    HFS_LEAVE();
    return status;
}
