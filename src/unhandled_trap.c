#include <stdint.h>
#include "bp_utils.h"

// This test triggers a load access fault by reading an invalid address. Correct
// behavior under perch is to print a message of the following form, before
// finishing:
//
// mcause: 0x0000000000000005
// mtval:  0x0000007fffffffff
// mepc:   0x000000008000019c
// Unhandled machine-mode trap
//
// This test is intended to fail ([CORE0 FSH] FAIL) as this is the desired
// outcome for normal programs in the case of an unhandled trap.

int main(int argc, char** argv) {
    *(volatile int*)0xFFFFFFFFFFFFFFFF;

    // Should not get here
    bp_finish(1);

    return 0;
}
