/*
 * io_lib_test.c
 *
 * Test for the data server
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

#define TEST_STRIPE_SIZE 100

void AllocHandle ()
{
    HFS_STATUS status;
    __u32 hdl;

    status = srvAllocDataHandle(&hdl);
    if (!HFS_SUCCESS(status)) {
        printf (" Failed to allocate data handle.\n");
        return;
    }
    printf (" Allocated handle : %u.\n", hdl);
}

void FreeHandle (__u32 hdl)
{
    HFS_STATUS status;

    status = srvFreeDataHandle(hdl);
    if (!HFS_SUCCESS(status)) {
        printf (" Failed to free data handle %u.\n", hdl);
        return;
    }
    printf (" Freed handle : %u.\n", hdl);
}

void WriteToExtent (__u32 hdl, __u32 stripeNum, __u32 writeSize)
{
    HFS_STATUS status;
    char stripe[TEST_STRIPE_SIZE];

    memset (stripe, 0, TEST_STRIPE_SIZE);
    sprintf (stripe, "Test Stripe, Handle: %u & Stripe: %u.", hdl, stripeNum);
    status = srvWriteStripe(hdl, TEST_STRIPE_SIZE, stripeNum, 1, writeSize,0,stripe);
    if (!HFS_SUCCESS(status)) {
        printf (" Failed to write stripe %u for handle %u.\n", stripeNum, hdl);
        return;
    }
    printf (" Written stripe %u for handle %u.\n", stripeNum, hdl);
}

void ReadFromExtent (__u32 hdl, __u32 stripeNum)
{
    HFS_STATUS status;
    char stripe[TEST_STRIPE_SIZE];
    __u32 readSize;

    memset (stripe, 0, TEST_STRIPE_SIZE);
    status = srvReadStripe(hdl, TEST_STRIPE_SIZE, stripeNum, 1, &readSize, stripe);
    if (!HFS_SUCCESS(status)) {
        printf (" Failed to read stripe %u for handle %u.\n", stripeNum, hdl);
        return;
    }
    printf (" Stripe %u for handle %u --> %-*s.\n", stripeNum, hdl, readSize, stripe);
}

void PrintExtentSize (__u32 hdl)
{
    HFS_STATUS status;
    __u32 extentSize;

    status = srvGetExtentSize (hdl, &extentSize);
    if (!HFS_SUCCESS(status)) {
        printf (" Failed to read extent size for handle %u.\n", hdl);
        return;
    }
    printf (" Extent size for handle %u = %u.\n", hdl, extentSize);
}

int io_lib_test_main(int argc, char *argv[])
{
    umask (S_IROTH | S_IWOTH);
    printf (" Testing the io library... \n");

    //-- Test allocation and deallocation of handles --//
    AllocHandle ();
    AllocHandle ();
    FreeHandle (0);
    //FreeHandle (1);
    FreeHandle (2);
    AllocHandle ();
    AllocHandle ();

    //-- Test read and write to extents --//
    WriteToExtent (1, 0, TEST_STRIPE_SIZE);
    WriteToExtent (2, 0, TEST_STRIPE_SIZE);
    WriteToExtent (3, 0, TEST_STRIPE_SIZE);
    WriteToExtent (1, 1, 30);
    WriteToExtent (2, 1, 30);
    WriteToExtent (3, 1, 30);

    ReadFromExtent (1, 0);
    ReadFromExtent (2, 0);
    ReadFromExtent (3, 0);
    ReadFromExtent (1, 1);
    ReadFromExtent (2, 1);
    ReadFromExtent (3, 1);

    PrintExtentSize (1);
    PrintExtentSize (2);
    PrintExtentSize (3);

    FreeHandle (1);
    FreeHandle (2);
    FreeHandle (3);

    return 0;
}
