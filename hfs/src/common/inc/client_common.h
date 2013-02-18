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

//-- Hercules Server command line options --//
//-- used by both format and server       --//
typedef struct _HFS_CLIENT_OPTIONS {
    char *fsConfigFilePath;
    char *clientLogFile;
    char *mountPoint;
    char *rootMDSAddr;
    char *rootMDSPort;
    int  logMask;
    int  maxServersSupported;
    int  maxMdsServersSupported;
} HFS_CLIENT_OPTIONS, *PHFS_CLIENT_OPTIONS;
