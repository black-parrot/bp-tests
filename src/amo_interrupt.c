
#include <stdint.h>
#include <bp_utils.h>

volatile uint64_t *mtimecmp = (uint64_t *) 0x304000;
volatile uint64_t *mtime = (uint64_t *) 0x30bff8;

uint64_t amo_target __attribute__ ((aligned (16))) = 0;

void trap_success(void) __attribute__((interrupt));
__attribute__((interrupt)) void trap_success(void) {
    uint64_t return_pc;

    uint64_t current_time = *mtime;
    *mtimecmp = *mtime + 128;
}

void main(uint64_t argc, char * argv[]) {
  uint64_t mie = (1 << 7) | (1 << 5) | (1 << 4); // M + S + U timer interrupt enable
  uint64_t mstatus = (1 << 3) | (1 << 1) | (1 << 0); // Global interrupt enable
  // Set up trap to alternate handler
  __asm__ __volatile__ ("csrw mtvec, %0": : "r" (&trap_success));
  // Setting up mtimecmp and mtime with some arbitrary values
  *mtimecmp = 64;
  *mtime = 0;
  // Enabling interrupts for User, Supervisor and Machine mode
  __asm__ __volatile__ ("csrw mie, %0": : "r" (mie));
  __asm__ __volatile__ ("csrw mstatus, %0" : : "r" (mstatus));


  for (int i = 0; i < 10000; i++) {
    __asm__ __volatile__ ("amoadd.d x1, %0, 0(%1)" : : "r" (1), "r" (&amo_target));
  }

  if (amo_target == 10000) {
    bp_finish(0);
  } else {
    bp_finish(1);
  }
}
