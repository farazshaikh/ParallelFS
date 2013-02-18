/*
 * logger_common.h
 *
 *     logging functionality
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

#ifndef __LOGGER_COMMON_
#define __LOGGER_COMMON_
#include <stdio.h>

#define  DO_DEBUG_LOG             0         // control logging
extern   FILE *stderr;

#define LOG_MASK_LOG_ALL         -1

#define STRINGIFY(X) #X
#define TOSTR(X)   STRINGIFY(X)
#define __FNLN__ __FILE__ ":" TOSTR(__LINE__)

/*
#ifndef     COMPONENT_ID
    #error "HFS:COMPILE:ERROR COMPONENT_ID not defined !!"
#endif
*/

#define    LOG_ERROR    0x1
#define    LOG_WARN     0x2
#define    LOG_INFO     0x4
#define    LOG_ENTRY    0x8

#define HFS_LOG_ERROR(fmt,vargs...)                                     \
    do{                                                                 \
        char osErrString[256];                                          \
        char *osErrStringp;                                             \
        fprintf(stderr, "HFS_ERROR:"__FNLN__":"fmt"\n",##vargs);        \
        osErrString[0] = 0;                                             \
        if (errno != 0) {                                               \
            osErrStringp = strerror_r(errno, osErrString, 256);         \
            fprintf(stderr, "OS Error Code %d : %s\n",                  \
                    errno, osErrStringp);                               \
        }                                                               \
    } while(0)

#define HFS_LOG_WARN(fmt,vargs...)                                      \
    do{ fprintf(stderr,"HFS_WARN:"__FNLN__":"fmt"\n",##vargs);} while(0)

#define HFS_LOG_ALWAYS(fmt,vargs...)                                    \
    do{                                                                 \
        fprintf(stderr,__FNLN__":"fmt"\n",##vargs);                     \
    } while(0)


#if DO_DEBUG_LOG
#define HFS_LOG_INFO(fmt,vargs...)                                      \
    do{ fprintf(stderr, "HFS_INFO:"__FNLN__":"fmt"\n",##vargs);} while(0)
#define HFS_ENTRY(fnName)                                               \
    do{ fprintf(stderr, "ENTRY: %s \n",__FUNCTION__); } while(0)
#define HFS_LEAVE(fnName)                                               \
    do{ fprintf(stderr, "LEAVE: %s \n",__FUNCTION__); } while(0)
#else
    #define NOP
    #define HFS_LOG_INFO(fmt,vargs...)  NOP
    #define HFS_ENTRY(fnName)           NOP
    #define HFS_LEAVE(fnName)           NOP
#endif

HFS_STATUS loggerInit(char *logFileName,int logMask);

#endif
