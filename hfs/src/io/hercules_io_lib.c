/*
 * hercules_db_lib.c
 *
 * Operations exported by the Data Server
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

HFS_DATA_HANDLE
readHandleFromFile()
{
    char            fileName[MAX_EXTENT_FILE_NAME_LEN];
    HFS_DATA_HANDLE handle=HFS_INVALID_HANDLE;
    int             fd = -1,retval;
    HFS_STATUS      status = HFS_STATUS_CONFIG_ERROR;

    do {
        status = hdlToFileName(HFS_DATA_SERVER_META_FILE_HANDLE, fileName);
        if (!HFS_SUCCESS(status)) {
            break;
        }

        fd = open (fileName, O_RDWR);
        if (fd < 0) {
            fd = -1;
            HFS_LOG_ERROR("Error opening meta-data file for this DS server %d",errno);
            handle   = HFS_INVALID_HANDLE;
            break;
        }

        //- Read handle -//
        retval = read(fd,&handle,sizeof(handle));
        if(sizeof(handle) != retval) {
            HFS_LOG_ERROR("Error reading data handle for this data server");
            handle   = HFS_INVALID_HANDLE;
            break;
        }
        handle++;
        //- Write handle -//
        retval = lseek(fd,0, SEEK_SET);
        if (retval < 0) {
            HFS_LOG_ERROR("Error seeking for data handle for this data server");
            handle   = HFS_INVALID_HANDLE;
            break;
        }

        retval = write(fd,&handle,sizeof(handle));
        if (sizeof(handle) != retval) {
            HFS_LOG_ERROR("Error writing back updated handle");
            handle   = HFS_INVALID_HANDLE;
            break;
        }
    }while(0);

    if (-1 !=fd){
        close(fd);
    }

    return handle;
}

HFS_STATUS
srvAllocDataHandle(HFS_DATA_HANDLE *pHdl)
{
    HFS_STATUS status;
    char fileName[MAX_EXTENT_FILE_NAME_LEN];
    HFS_DATA_HANDLE handle;
    int fd;

    do {
        //-- Get a free handle --//
        handle = HFS_INVALID_HANDLE;
        fd     = -1;
        status = HFS_STATUS_CONFIG_ERROR;

        //-- Create a file corresponding to the handle --//
        handle = readHandleFromFile();
        if (HFS_INVALID_HANDLE == handle) {
            status    = HFS_STATUS_CONFIG_ERROR;
            handle    = HFS_INVALID_HANDLE;
            break;
        }

        //- Crate the data file -//
        memset(fileName,0,MAX_EXTENT_FILE_NAME_LEN);
        status = hdlToFileName(handle, fileName);
        if (!HFS_SUCCESS(status)) {
            break;
        }

        fd = open (fileName, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        if (fd < 0) {
            fd = -1;
            HFS_LOG_ERROR("Error creating data file for this DS server %d",errno);
            status = HFS_STATUS_CONFIG_ERROR;
            handle   = HFS_INVALID_HANDLE;
            break;
        }
        status = HFS_STATUS_SUCCESS;
    } while(0);


    //finally:
    //-- Close the file --//
    if (-1 != fd) {
        close (fd);
    }

    *pHdl = handle;
    return status;
}


HFS_STATUS srvFreeDataHandle(HFS_DATA_HANDLE hdl)
{
    char fileName[MAX_EXTENT_FILE_NAME_LEN];
    HFS_STATUS status;
    int retval;
    do {
            //-- Delete the file corresponding to the handle --//
            status = hdlToFileName(hdl, fileName);
            if (!HFS_SUCCESS(status)) {
                break;
            }

            retval = unlink(fileName);
            if (retval < 0) {
                HFS_LOG_ERROR("Cannot free data handle %x",hdl);
                status = HFS_INTERNAL_ERROR;
                break;
            }

            //-- Free the handle --//
            status = HFS_STATUS_SUCCESS;
    } while(0);

    return status;
}

HFS_STATUS srvReadStripe(HFS_DATA_HANDLE    hdl,
                         __u32        stripeSize,
                         __u32        stripeNum,
                         __u32        stripeCnt,
                         __u32       *readSize,
                         void        *buf)
{
    HFS_STATUS status;
    char fileName[MAX_EXTENT_FILE_NAME_LEN];
    int fd;
    long pos;
    long len;
    int retval;

#ifdef PARAM_CHECKING
    if (stripeCnt == 0) {
        return HFS_STATUS_INVALID_PARAM_3;
    }

    if (buf == NULL) {
        return HFS_STATUS_INVALID_PARAM_4;
    }

    if (readSize == NULL) {
        return HFS_STATUS_INVALID_PARAM_5;
    }
#endif

    do {
        //-- Get file name from handle --//
        status = hdlToFileName(hdl, fileName);
        if (!HFS_SUCCESS(status)) {
            break;
        }

        //-- Open the file for reading --//
        fd = open(fileName, O_RDONLY);
        if (fd < 0) {
            status = HFS_INVALID_HANDLE_ERROR;
            break;
        }

        //-- Seek to appropriate position --//
        pos = stripeNum * stripeSize;
        retval = lseek(fd, pos, SEEK_SET);
        if (retval < 0) {
            status = HFS_INVALID_STRIPE_ERROR;
            break;
        }

        //-- Read data into buffer --//
        len = stripeCnt * stripeSize;
        retval = read(fd, buf, len);
        if (retval < 0) {
            status = HFS_HOST_OS_ERROR;
            break;
        }

        *readSize = retval;
        status = HFS_STATUS_SUCCESS;
    } while (0);

    //-- Close the file --//
    if (fd > 0) {
        close (fd);
    }

    return status;
}

HFS_STATUS
srvWriteStripe(HFS_DATA_HANDLE   hdl,
               __u32       stripeSize,
               __u32       stripeNum,
               __u32       stripeCnt,
               __u32       inStripeOffset,
                          __u32       writeSize,
               void       *buf)
{
    HFS_STATUS status;
    char fileName[MAX_EXTENT_FILE_NAME_LEN];
    int fd;
    long pos;
    long len;
    int retval;

#ifdef PARAM_CHECKING
    if (stripeCnt == 0)
        return HFS_STATUS_INVALID_PARAM_3;
    if (buf == NULL)
        return HFS_STATUS_INVALID_PARAM_4;
#endif

    do {
        //-- Get file name from handle --//
        status = hdlToFileName(hdl, fileName);
        if (!HFS_SUCCESS(status)) {
            break;
        }

        //-- Open the file for writing --//
        fd = open(fileName, O_WRONLY);
        if (fd < 0) {
            status = HFS_INVALID_HANDLE_ERROR;
            break;
        }

        //-- Seek to appropriate position --//
        pos = (stripeNum * stripeSize) + inStripeOffset;
        retval = lseek(fd, pos, SEEK_SET);
        if (retval < 0) {
            status = HFS_HOST_OS_ERROR;
            break;
        }

        //-- Write data into buffer --//
        //len = stripeCnt * stripeSize;
        len = writeSize;
        retval = write(fd, buf, len);
        if (retval < 0) {
            status = HFS_HOST_OS_ERROR;
            break;
        }

        status = HFS_STATUS_SUCCESS;
    } while (0);

    //-- Close the file --//
    if (fd > 0)
        close (fd);

    return status;
}

HFS_STATUS
srvGetExtentSize (HFS_DATA_HANDLE hdl,
                  __u32       *extSize)
{
    HFS_STATUS status;
    FILE * fHdl;
    long len;
    int retval;
    char fileName[MAX_EXTENT_FILE_NAME_LEN];

#ifdef PARAM_CHECKING
    if (extSize == NULL) {
        return HFS_STATUS_INVALID_PARAM_2;
    }
#endif

    do {
        //-- Get file name from handle --//
        status = hdlToFileName(hdl, fileName);
        if (!HFS_SUCCESS(status)) {
            break;
        }

        //-- Open the file --//
        fHdl = fopen(fileName, "r+");
        if (fHdl == NULL) {
            status = HFS_INVALID_HANDLE_ERROR;
            break;
        }

        //-- Seek to the end --//
        retval = fseek(fHdl, 0, SEEK_END);
        if (retval < 0) {
            status = HFS_HOST_OS_ERROR;
            break;
        }

        //-- Get the size --//
        len = ftell(fHdl);
        if (len < 0) {
            status = HFS_HOST_OS_ERROR;
            break;
        }

        *extSize = (__u32) len;
        status = HFS_STATUS_SUCCESS;
    } while (0);

    //-- Close the file --//
    if (fHdl != NULL) {
        fclose (fHdl);
    }

    return status;
}

HFS_STATUS
hdlToFileName(HFS_DATA_HANDLE hdl, char *filename)
{
#ifdef PARAM_CHECKING
    if (fileName == NULL) {
        return HFS_STATUS_INVALID_PARAM_2;
    }
#endif

    snprintf (filename,MAX_EXTENT_FILE_NAME_LEN,"%09x.dat", hdl);
    return HFS_STATUS_SUCCESS;
}
