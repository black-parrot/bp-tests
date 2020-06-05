

/*
 * This test checks if timer interrupts are working 
 */
#include <stdint.h>
#include "bp_utils.h"

uint8_t int_count = 48;
uint64_t mtimecmp_data = 64;
uint64_t mtimecmp = 0x304000;
uint64_t mtime = 0x30bff8;

void trap_success(void) __attribute__((interrupt));
void trap_success() {
    // Read mcause
    uint64_t mcause;
    __asm__ __volatile__ ("csrr %0, mcause" : "=r" (mcause));

    // Check for interrupt
    if ((mcause >> 63) == 1) {
      if(((mcause & 7) == 4) || ((mcause & 7) == 5) || ((mcause & 7) == 7))
        int_count++;
        // To account for mtimecmp updation time. Arbitrarily chosen number.
        mtimecmp_data += 1024;
        // Need to write to mtimecmp to clear the interrupt
        __asm__ __volatile__ ("li t3, 0x304000");
        __asm__ __volatile__ ("sd %0, 0(t3)": : "r"(mtimecmp_data-1));
        bp_cprint(int_count);
        bp_cprint(10);
        if((int_count - 48) == 8)
          bp_finish(0);
    }
    else {
      bp_cprint(int_count);
      bp_cprint(10);
      bp_finish(1);
    }
}

void main(uint64_t argc, char * argv[]) {
   uint64_t mie = 176; // M + S + U timer interrupt enable
   uint64_t mstatus = 11; // Global interrupt enable
   int iter_count = 0;
   // Set up trap to alternate handler
   __asm__ __volatile__ ("csrw mtvec, %0": : "r" (&trap_success));
   // Setting up mtimecmp and mtime with some arbitrary values
   __asm__ __volatile__ ("li t4, 0x304000");
   __asm__ __volatile__ ("sd %0, 0(t4)": : "r"(mtimecmp_data-1));
   __asm__ __volatile__ ("li a5, 0x30bff8");
   __asm__ __volatile__ ("sd zero, 0(a5)");
   // Enabling interrupts for User, Supervisor and Machine mode
   __asm__ __volatile__ ("csrw mie, %0": : "r" (mie));
   __asm__ __volatile__ ("csrw mstatus, %0" : : "r" (mstatus));
  
   while(1);
}

