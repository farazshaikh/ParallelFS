/*
 * logger.c
 *
 *      Initialize logging
 *
 *      Bugs: fshaikh@cs.cmu.edu
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
static int errorLogMask;


HFS_STATUS
loggerInit(char *logFileName,
           int logMask)
{
    HFS_STATUS ret=HFS_STATUS_SUCCESS;
    if (NULL == freopen(logFileName, "a+",stderr)) {
        printf("HFS cannot init logging on %s",logFileName);
        return HFS_INTERNAL_ERROR;
    }

    setbuf(stderr,NULL);

    HFS_LOG_ALWAYS("=========== Starting logger =============\n");
    errorLogMask = logMask;
    return ret;
}
