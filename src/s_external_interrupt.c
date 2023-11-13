

/*
 * This test checks if S-mode external interrupt works
 */
#include <stdint.h>
#include "bp_utils.h"

void pass() { bp_finish(0); }
void fail() { bp_finish(1); }

volatile unsigned *s_mode_ext = (unsigned *) 0x30b004UL;
volatile unsigned state;

void trap_handler(void) __attribute__((interrupt));
void trap_handler() {
  // Read mcause
  uint64_t mcause;
  __asm__ __volatile__ ("csrr %0, mcause" : "=r" (mcause));

  // Check if it is really an S-mode external interrupt
  if(mcause == ((1UL << 63) | 9UL)) {
    state = 1;

    // Clear fake S-mode PLIC bit
    *s_mode_ext = 0;
  }
  else {
    fail();
  }
}

volatile unsigned cnt; // avoid being optimized away
void small_delay()
{
  cnt = 0;
  while(cnt < 256)
    cnt++;
}


void check_1()
{
  // check if we get (SEIP || s_mode_ext) from mip with a CSR read
  uint64_t mip, set_val;
  mip = (1UL << 9);
  __asm__ __volatile__ ("csrc mip, %0": :"r" (mip)); // clear bit
  *s_mode_ext = 1;
  small_delay();
  set_val = 1; // Read-Modify-Write
  __asm__ __volatile__ ("csrrs %0, mip, %1" : "=r" (mip) : "r"(set_val));
  if(!(mip & (1UL << 9))) {
    fail();
  }
  *s_mode_ext = 0;
  small_delay();
  __asm__ __volatile__ ("csrrs %0, mip, x0" : "=r" (mip)); // read
  // check if the previous setting of SEIP falsely depends on the s_mode_ext
  if(mip & (1UL << 9)) {
    fail();
  }
}

void check_2()
{
  uint64_t mip;
  // check if the we can set the S-mode pending bit
  *s_mode_ext = 0;
  small_delay();
  mip = (1UL << 9);
  __asm__ __volatile__ ("csrs mip, %0": :"r" (mip)); // set bit
  __asm__ __volatile__ ("csrr %0, mip" : "=r" (mip)); // read
  if(!(mip & (1UL << 9))) {
    fail();
  }
}

void check_3()
{
  uint64_t mie, mip, mstatus;
  // Set up trap to alternate handler
  __asm__ __volatile__ ("csrw mtvec, %0": : "r" (&trap_handler));

  mip = (1UL << 9);
  __asm__ __volatile__ ("csrc mip, %0": :"r" (mip)); // clear bit

  mstatus = (3 << 13) | (1 << 3); // Enable M-mode global interrupt
  __asm__ __volatile__ ("csrw mstatus, %0" : : "r" (mstatus));

  // init state
  state = 0;
  // Set fake S-mode PLIC bit
  *s_mode_ext = 1;
  small_delay();

  // check if the interrupt is really disabled
  if(state != 0) {
    fail();
  }

  // Enable S-mode external interrupt
  mie = (1 << 9);
  __asm__ __volatile__ ("csrw mie, %0": : "r" (mie));
  small_delay();

  // check if the interrupt is really enabled
  if(state != 1)
    fail();
}

void main(uint64_t argc, char * argv[]) {
  check_1();
  check_2();
  check_3();
  pass();
}

