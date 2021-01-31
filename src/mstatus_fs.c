
/*
 * This test checks that SATP updates are reflected in instruction fetches
 *   WITHOUT an sfence
 */
#include <stdint.h>
#include "bp_utils.h"

uint64_t num_traps = 0;

void trap_continue() {
    uint64_t return_pc;

    num_traps++;

    __asm__ __volatile__ ("csrr  %0, mepc": "=r" (return_pc): :);
    return_pc += 4;
    __asm__ __volatile__ ("csrw mepc, %0": : "r" (return_pc));
    __asm__ __volatile__ ("mret");
}

void trap_fail() {
    bp_finish(1);
}

void main(uint64_t argc, char * argv[]) {
   // Set up trap to alternate handler
   __asm__ __volatile__ ("csrw mtvec, %0": : "r" (&trap_continue));
   // Clear FS
   uint64_t mstatus_fs = 3 << 13;
   __asm__ __volatile__ ("csrc mstatus, %0" : : "r" (mstatus_fs));

   // Test reading/writing registers traps
   __asm__ __volatile__ ("fadd.d f0, f0, f0");
   // Test reading/writing registers traps
   __asm__ __volatile__ ("fld f0, 0(x0)");
   // Test reading/writing fcsr traps
   __asm__ __volatile__ ("frcsr x0");
   // Test reading/writing frm traps
   __asm__ __volatile__ ("frrm x0");
   // Test reading/writing fflags traps
   __asm__ __volatile__ ("frflags x0");

   if (num_traps != 5)
     bp_finish(1);

   bp_finish(0);
}

