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

#ifndef __HERCULES_COMMON__
#define __HERCULES_COMMON__

#define _BSD_SOURCE
#define _GNU_SOURCE

//- Portable GLibc Headers -//
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include <assert.h>

//- Linux Headers               -//
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/poll.h>
#include <linux/types.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/personality.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#include <utime.h>
#include <sys/wait.h>
#include <sys/vfs.h>
#ifndef __USE_GNU
    #define __USE_GNU
#endif
#include <signal.h>
#include <rpc/des_crypt.h>


// - Fuse Header    -//
//#define FUSE_USE_VERSION 21
#include <fuse.h>


//- Sockets                -//
#include <sys/ioctl.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//-- Db Headers --//
#include <mysql.h>

//- Internal Headers -//
#include <hercules_types.h>
#include <hercules_errors.h>
#include <logger_common.h>
#include <utils_common.h>
#include <hercules_hdl_mngr.h>
#include <hercules_io_lib.h>
#include <hercules_db_lib.h>
#include <proto_common.h>
#include <client_common.h>
#include <server_common.h>
#include <hercules_proto_buff.h>

#define HFS_UNUSED(status)  ((void) (status))

#endif // __HERCULES_COMMON_
