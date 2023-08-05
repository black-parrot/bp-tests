#include "bp_utils.h"

/* this tests cases where fmul should compute to NaN, but BP computes to -0
*/

void main(uint64_t argc, char *argv[]) {
  uint64_t result_0;
  asm volatile(
    "li        a4, 0x80000000;"
    "li        a5, 0x12345678;"
    "fmv.w.x   fa4,a4;"
    "fmv.w.x   fa5,a5;"
    "fmul.d    fa6,fa4,fa5;"// from the RISC-V ISA, result should be a NaN because with nan boxing,
                            // double precision ops on single precision operands result in NaN
    "fmul.d    fa7,fa5,fa4;"
    "fclass.d  %0, fa6;"
    : "+r" (result_0)
  );
  if(result_0 == 0x200)
    bp_finish(0);
  else bp_finish(1);
}
