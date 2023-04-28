#include "bp_utils.h"

/* this tests floating point accumulations.
   this can be useful to detect precision mismatches
   of SP (float) operations in DP (double) elements */

void main(uint64_t argc, char *argv[]) {
  uint64_t result_0, result_1;
  asm volatile(
    "li        a4, 0x80000000;"
    "li        a5, 0x12345678;"
    "fmv.w.x   fa4,a4;"
    "fmv.w.x   fa5,a5;"
    "fmul.s    fa6,fa4,fa5;"
    "fmul.s    fa7,fa5,fa4;"
    "fmul.d    fa6,fa4,fa5;"
    "fmul.d    fa7,fa5,fa4;"
    "fmv.x.w   %0,fa6;"
    "fmv.x.w   %1,fa7;"
    : "+r" (result_0, result_1)
  );

  if(result_0 == 0x7ff8000000000000)   // expected value per IEEE 754
    bp_finish(0);
  else if(result_1 == 0x7ff8000000000000)
    bp_finish(0);
  else bp_finish(1);
}
