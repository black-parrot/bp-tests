#include "bp_utils.h"

/* this tests cases where fmul computes to -0,
  but as implemented in BP, it executes a FMA: A*B + (+0) to emulate A*B
  The sign of C influences the sign of the operation in certain cases, but it should not. */

void main(uint64_t argc, char *argv[]) {
  uint64_t result_0, result_1;
  asm volatile(
    "li        a4, 0x80000000;"
    "li        a5, 0x12345678;"
    "fmv.w.x   fa4,a4;"
    "fmv.w.x   fa5,a5;"
    "fmul.s    fa6,fa4,fa5;"// expected value per IEEE 754 is 0x8000_0000 or -0 (in decimal)
    "fmul.s    fa7,fa5,fa4;"// expected value per IEEE 754 is 0x8000_0000 or -0 (in decimal)
    "fmul.d    fa6,fa4,fa5;"// from the RISC-V ISA, result should be a NaN
    "fmul.d    fa7,fa5,fa4;"
    "fmv.x.w   %0,fa6;"
    "fmv.x.w   %1,fa7;"
    : "+r" (result_0, result_1)
  );

  if(result_0 == 0x7ff8000000000000)   
    bp_finish(0);
  else if(result_1 == 0x7ff8000000000000)
    bp_finish(0);
  else bp_finish(1);
}
