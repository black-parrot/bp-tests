#include <stdint.h>
#include "bp_utils.h"

// This test triggers an illegal instruction exception. Correct behavior under
// perch is to print a message of the following form, before finishing:
//
// mcause: 0x0000000000000002
// mtval:  0x0000000000000000
// mepc:   0x000000008effefec
// Illegal instruction
//
// This test is intended to fail ([CORE0 FSH] FAIL) as this is the desired
// outcome for normal programs in the case of an illegal instruction.

int main(int argc, char** argv) {
    int x = 0xDEADBEEF;
    ((void (*)(void))&x)();

    // Should not get here
    bp_finish(1);

    return 0;
}
