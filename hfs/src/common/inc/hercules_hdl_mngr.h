/*
 * hercules_hdl_mngr.h
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

#ifndef SRVHDLMNGR_H_
#define SRVHDLMNGR_H_

typedef struct __DS_HANDLE_MGR_CTX_{
}DS_HANDLE_MGR_CTX,*PDS_HANDLE_MGR_CTX;

typedef struct __MDS_HANDLE_MGR_CTX {
}MDS_HANDLE_MGR_CTX,*PMDS_HANDLE_MGR_CTX;



// data handles //
HFS_STATUS initDSHdlMngr    (PHFS_SERVER_INFO,PDS_HANDLE_MGR_CTX);
HFS_STATUS cleanupDSHdlMngr (PDS_HANDLE_MGR_CTX);
HFS_STATUS flushDSHandles   (PDS_HANDLE_MGR_CTX);
HFS_STATUS getDSFreeHandle  (PDS_HANDLE_MGR_CTX,HFS_DATA_HANDLE * pHdl);
HFS_STATUS freeDSHandle     (PDS_HANDLE_MGR_CTX,HFS_DATA_HANDLE hdl);


// Meta data handles //
HFS_STATUS initMDSHdlMngr    (PHFS_SERVER_INFO,PMDS_HANDLE_MGR_CTX);
HFS_STATUS cleanupMDSHdlMngr (PMDS_HANDLE_MGR_CTX);
HFS_STATUS flushMDSHandles   (PMDS_HANDLE_MGR_CTX);
HFS_STATUS getMDSFreeHandle  (PMDS_HANDLE_MGR_CTX,HFS_META_DATA_HANDLE *pHdl);
HFS_STATUS freeMDSHandle     (PMDS_HANDLE_MGR_CTX,HFS_META_DATA_HANDLE  hdl);

#endif /*SRVHDLMNGR_H_*/
