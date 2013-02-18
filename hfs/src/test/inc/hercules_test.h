/*
 * hercules_test.h
 *
 *      Test include
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

#ifndef __HERCULES_TEST__
#define __HERCULES_TEST__

typedef int (*test_unit_main) (int argc, char *argv[]);

typedef struct __TEST_UNIT_ {
  char *moduleName;
  test_unit_main testUnitEntry;
} TEST_UNIT;

extern int io_lib_test_main (int argc, char *argv[]);
#define TEST_UNIT_LIST {                                                       \
    {"io_lib", io_lib_test_main},                                              \
    {NULL, NULL}                                                               \
  }

#endif   // __HERCULES_TEST__
