

/*
 * This test checks if timer interrupts are working 
 */
#include <stdint.h>
#include "bp_utils.h"

uint8_t int_count = 48;
uint64_t *mtimecmp = 0x304000;
uint64_t *mtime = 0x30bff8;

void pass() { bp_finish(0); }
void fail() { bp_finish(1); }

void trap_success() {
    uint64_t minstret;
    __asm__ __volatile__ ("csrr %0, minstret" : "=r" (minstret));

    // Check if we've been retiring instrets in the loop
    if (minstret > 1000) {
       bp_finish(1);
    }

    uint64_t mstatus = 0; // Disable global interrupts
    __asm__ __volatile__ ("csrw mstatus, %0" : : "r" (mstatus));

    *mtimecmp = 200000;
    __asm__ __volatile__ ("fence");
    __asm__ __volatile__ ("wfi"::);

    // Requires manual verification
    bp_finish(0);
}

void main(uint64_t argc, char * argv[]) {
   uint64_t mie = (1 << 7) | (1 << 5) | (1 << 4); // M + S + U timer interrupt enable
   uint64_t mstatus = (1 << 3) | (1 << 1) | (1 << 0); // Global interrupt enable
   // Set up trap to alternate handler
   __asm__ __volatile__ ("csrw mtvec, %0": : "r" (&trap_success));
   // Setting up mtimecmp and mtime with some arbitrary values
   *mtimecmp = 100000;
    __asm__ __volatile__ ("fence");

   // Enabling interrupts for User, Supervisor and Machine mode
   __asm__ __volatile__ ("csrw mie, %0": : "r" (mie));
   __asm__ __volatile__ ("csrw mstatus, %0" : : "r" (mstatus));

   while (1) {
     __asm__ __volatile__ ("wfi"::);
   }
}

