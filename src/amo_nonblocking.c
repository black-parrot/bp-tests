
#include <stdint.h>
#include <bp_utils.h>


uint64_t amo_target __attribute__ ((aligned (16))) = 0;

void main(uint64_t argc, char * argv[]) {
   __asm__ __volatile__ ("amoadd.d x0, %0, 0(%1)" : : "r" (1), "r" (&amo_target));
   __asm__ __volatile__ ("amoadd.d x0, %0, 0(%1)" : : "r" (1), "r" (&amo_target));
   __asm__ __volatile__ ("amoadd.d x0, %0, 0(%1)" : : "r" (1), "r" (&amo_target));
   __asm__ __volatile__ ("amoadd.d x0, %0, 0(%1)" : : "r" (1), "r" (&amo_target));
   __asm__ __volatile__ ("amoadd.d x0, %0, 0(%1)" : : "r" (1), "r" (&amo_target));

   bp_finish(0);
}
