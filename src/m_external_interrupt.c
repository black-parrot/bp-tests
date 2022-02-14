

/*
 * This test checks if M-mode external interrupt works
 */
#include <stdint.h>
#include "bp_utils.h"

void pass() { bp_finish(0); }
void fail() { bp_finish(1); }

volatile unsigned *m_mode_ext = 0x30b000UL;
volatile unsigned cnt; // avoid being optimized away
volatile unsigned state;

void trap_handler(void) __attribute__((interrupt));
void trap_handler() {
  // Read mcause
  uint64_t mcause;
  __asm__ __volatile__ ("csrr %0, mcause" : "=r" (mcause));

  // Check if it is really a M-mode external interrupt
  if(mcause == ((1UL << 63) | 11UL)) {
    state = 1;

    // Clear fake M-mode PLIC bit
    *m_mode_ext = 0;
  }
  else
    fail();
}
void main(uint64_t argc, char * argv[]) {
  uint64_t mie;
  // Set up trap to alternate handler
  __asm__ __volatile__ ("csrw mtvec, %0": : "r" (&trap_handler));

  uint64_t mstatus = (3 << 13) | (1 << 3); // Enable M-mode global interrupt
  __asm__ __volatile__ ("csrw mstatus, %0" : : "r" (mstatus));

  // init state
  state = 0;
  // Set fake M-mode PLIC bit
  *m_mode_ext = 1;

  // Wait and see if we will get the interrupt
  while(cnt < 1000)
    cnt++;
  if(state != 0)
    fail();

  cnt = 0;
  // Enable M-mode external interrupt
  mie = (1 << 11);
  __asm__ __volatile__ ("csrw mie, %0": : "r" (mie));

  // Wait and see if we will get the interrupt
  while(cnt < 1000)
    cnt++;
  if(state == 1)
    pass();

  fail();
}

