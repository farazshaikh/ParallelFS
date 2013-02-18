/*
 * updateFSConfig.c
 *
 *      IPC to the servers/clients to update the File syste config
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

#include <hercules_common.h>
#include <hercules_server.h>

// Wakes up the servers config update thread //
int
main(int argc, char **argv)
{
    int sem, ret;
    struct sembuf       sop;

    sem = semget(MDS_SERVER_FS_UPDATE_SEM, 1, 0);
    if(-1 == sem) {
        printf("Cannot open shared semaphore for Hercules Server\n");
        return ENOENT;
    }

    sop.sem_num =  0;
    sop.sem_op  =  1;
    sop.sem_flg =  0;

    ret = semop(sem, &sop, 1);
    if(-1 == ret) {
        printf("Cannot signal shared semaphore for Hercules Server\n");
    } else {
        printf("Server Nudged Successfully for FS Config update\n");
    }

    return 0;
}
