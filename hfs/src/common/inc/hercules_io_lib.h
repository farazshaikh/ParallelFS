/*
 * hercules_client_inbound.c
 *
 *      Ops exported by the Data Server
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

#ifndef HERCULES_IO_LIB_H_
#define HERCULES_IO_LIB_H_

#include <linux/types.h>
#include "hercules_types.h"
#include "proto_common.h"

#define MAX_EXTENT_FILE_NAME_LEN 80

HFS_STATUS srvAllocDataHandle(HFS_DATA_HANDLE *pHdl);

HFS_STATUS srvFreeDataHandle(HFS_DATA_HANDLE hdl);

HFS_STATUS srvReadStripe(HFS_DATA_HANDLE    hdl,
                         __u32        stripeSize,
                         __u32        stripeNum,
                         __u32        stripeCnt,
                         __u32       *readSize,
                         void        *buf);

HFS_STATUS srvWriteStripe (HFS_DATA_HANDLE   hdl,
                           __u32       stripeSize,
                           __u32       stripeNum,
                           __u32       stripeCnt,
                           __u32       inStripeOffset,
                           __u32       writeSize,
                           void       *buf);

HFS_STATUS srvGetExtentSize (HFS_DATA_HANDLE  hdl,
                             __u32       *extSize);

HFS_STATUS
hdlToFileName(HFS_DATA_HANDLE hdl, char *filename);

#endif /*HERCULES_IO_LIB_H_*/
