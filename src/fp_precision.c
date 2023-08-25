#include "bp_utils.h"

/* this tests floating point accumulations.
   this can be useful to detect precision mismatches
   of SP (float) operations in DP (double) elements */

void main(uint64_t argc, char *argv[]) {
  uint64_t result;
  asm volatile(
    "li        a4, 0x3e8a3d71;"
    "fmv.w.x   fa5,a4;"
    "li        a4, 0x3df5c28f;"
    "fmv.w.x   fa4,a4;"
    "fadd.s    fa5,fa5,fa4;"   // a particular addition that might accumulate precision 
                               // that should otherwise be rounded off
                               // this may not manifest functionally unless reused
    "fadd.s    fa5,fa5,fa4;"
    "li        a4, 0x3e8a3d71;"
    "fmv.w.x   fa4,a4;"
    "fadd.s    fa5,fa5,fa4;"
    "fmv.x.w   %0,fa5;"
    : "+r" (result)
  );

  if(result == 0x3f47ae14)   // expected value per IEEE 754
    bp_finish(0);
  else bp_finish(1);
}
