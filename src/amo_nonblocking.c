
#include <stdint.h>
#include <bp_utils.h>

uint64_t amo_target __attribute__ ((aligned (16))) = 0;

void main(uint64_t argc, char * argv[]) {
  for (int i = 0; i < 10000; i++) {
    __asm__ __volatile__ ("amoadd.d x0, %0, 0(%1)" : : "r" (1), "r" (&amo_target));
  }

  if (amo_target == 10000) {
    bp_finish(0);
  } else {
    bp_finish(1);
  }
}
