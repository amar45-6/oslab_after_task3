/*
 * demand_paging_test.c
 *
 * Tests demand paging by:
 *  1. Touching every page of a large array (forces page faults across many pages)
 *  2. Writing and reading back known values to verify correctness
 *  3. Printing a result so we can confirm it ran completely
 */

#include "syscall.h"

#define ARRAY_SIZE 1024   /* 1024 ints = 4 KB, spans multiple 128-byte pages */

int arr[ARRAY_SIZE];

int main() {
    int i, sum, expected;

    /* --- Phase 1: write to every element (causes page faults on first touch) --- */
    for (i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i + 1;
    }

    /* --- Phase 2: read back and verify --- */
    sum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        sum += arr[i];
    }

    /* expected = 1+2+...+ARRAY_SIZE = ARRAY_SIZE*(ARRAY_SIZE+1)/2 = 524800 */
    expected = ARRAY_SIZE * (ARRAY_SIZE + 1) / 2;

    if (sum == expected) {
        PrintString("DEMAND PAGING TEST PASSED\n");
    } else {
        PrintString("DEMAND PAGING TEST FAILED\n");
    }

    Halt();
    return 0;
}
