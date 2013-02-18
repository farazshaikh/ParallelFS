/*
 * hercules_types.h
 *
 *      Basic types
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

#ifndef __HERCULES_TYPES__
#define __HERCULES_TYPES__

#include <stdint.h>

//- Return type --//
typedef uint64_t HFS_STATUS;
#define HFS_SUCCESS(Status) ((HFS_STATUS)(Status) == 0)
//- File system fields --//

#endif
