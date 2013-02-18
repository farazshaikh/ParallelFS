/*
 * hercules_errors.h
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

#ifndef __HERCULES_ERRORS__
#define __HERCULES_ERRORS__

#define HFS_STATUS_SUCCESS  0x0

#define HFS_ERROR_MASK      0x80000000
#define HFS_MASK_QUEUING    0x08000000
#define HFS_MASK_DB         0x00800000
#define HFS_MASK_IO         0x00080000
#define HFS_MASK_HDL_MNGR   0x00008000
#define HFS_MASK_CONFIG     0x00000800

// CODE ERRORS ONLY PROTOCOL ERRORS GO IN proto_common.h //
#define HFS_STATUS_PRE_CONDITION_FAILS  0x80000001

// Queuing Error //
#define HFS_STATUS_QUEUE_NOT_STARTED    (HFS_ERROR_MASK | HFS_MASK_QUEUING | 0x1)

// DB Error //
#define HFS_STATUS_DB_ERROR             (HFS_ERROR_MASK | HFS_MASK_DB | 0x1)
#define HFS_STATUS_DB_INIT_FAILED       (HFS_ERROR_MASK | HFS_MASK_DB | 0x2)
#define HFS_STATUS_DB_FS_NOT_FOUND      (HFS_ERROR_MASK | HFS_MASK_DB | 0x3)
#define HFS_STATUS_DB_FS_BUSY           (HFS_ERROR_MASK | HFS_MASK_DB | 0x4)
#define HFS_STATUS_DB_NO_SUCH_RECORD    (HFS_ERROR_MASK | HFS_MASK_DB | 0x5)

// IO Error //
#define HFS_STATUS_IO_ERROR             (HFS_ERROR_MASK | HFS_MASK_IO | 0x1)
#define HFS_INVALID_HANDLE_ERROR        (HFS_ERROR_MASK | HFS_MASK_IO | 0x2)
#define HFS_INVALID_STRIPE_ERROR        (HFS_ERROR_MASK | HFS_MASK_IO | 0x3)
#define HFS_LOW_RESOURCES               (HFS_ERROR_MASK | HFS_MASK_IO | 0x4)
#define HFS_TRANSMISSION_FAILED         (HFS_ERROR_MASK | HFS_MASK_IO | 0x5)
// Handle Manager Error //

// Configuration File Error //
#define HFS_STATUS_CONFIG_ERROR         (HFS_ERROR_MASK | HFS_MASK_CONFIG | 0x1)

// Locking Errors These are miscellaneous and come from any sub-system //
#define HFS_STATUS_LOCKING_ERROR        (HFS_ERROR_MASK | 0x2)
#define HFS_INTERNAL_ERROR              (HFS_ERROR_MASK | 0x3)
#define HFS_STATUS_OUT_OF_MEMORY        (HFS_ERROR_MASK | 0x4)
#define HFS_HOST_OS_ERROR               (HFS_ERROR_MASK | 0x5)
#define HFS_PROTOCOL_ERROR              (HFS_ERROR_MASK | 0x5)
#define HFS_CONNECTION_DROPPED          (HFS_ERROR_MASK | 0x6)
#define HFS_CLIENT_DISCONNECTED         (HFS_ERROR_MASK | 0x7)
#define HFS_STATUS_AUTH_FAILED          (HFS_ERROR_MASK | 0x7)

#endif
