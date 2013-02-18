/*
 * hercules_test.c
 *
 * Test harness
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

#include "hercules_common.h"
#include "hercules_test.h"

int
main(int argc, char *argv[])
{
    TEST_UNIT testUnitList[] = TEST_UNIT_LIST;
    int i;
    int retval;

    if (argc < 2) {
        printf ("Usage: %s %s %s\n", argv[0], "<Module to be tested>",
                "<Parameters to module test unit>");
        return 0;
    }

    //-- Call the appropriate unit test utility --//
    printf ("********Hercules Unit Test Utility********\n");
    for (i=0; (testUnitList[i].moduleName != NULL) &&
             (strcmp(argv[1], testUnitList[i].moduleName)); i++);

    if (testUnitList[i].moduleName == NULL) {
        printf ("No unit test for module %s found.\n", argv[1]);
        return 0;
    }

    retval = (*testUnitList[i].testUnitEntry)(argc, argv);

    return retval;
}
