
#include <stdint.h>
#include <bp_utils.h>

// This test will fail if divide stored to zero still writes to the register
// due to forwarding garbage values to the subsequent add instructions
void main(uint64_t argc, char * argv[]) {
      uint64_t garbage0 = 0x1234;
      uint64_t garbage1 = 0x5678;
      __asm__ __volatile__ ("div zero, %0, %1": : "r"(garbage0), "r"(garbage1));
      __asm__ __volatile__ ("div zero, %0, %1": : "r"(garbage0), "r"(garbage1));
      __asm__ __volatile__ ("div zero, %0, %1": : "r"(garbage0), "r"(garbage1));
      __asm__ __volatile__ ("div zero, %0, %1": : "r"(garbage0), "r"(garbage1));
      __asm__ __volatile__ ("div zero, %0, %1": : "r"(garbage0), "r"(garbage1));
      __asm__ __volatile__ ("add t0, %0, %1"  : : "r"(garbage0), "r"(garbage1));
      __asm__ __volatile__ ("add t1, %0, %1"  : : "r"(garbage0), "r"(garbage1));
      __asm__ __volatile__ ("add t2, %0, %1"  : : "r"(garbage0), "r"(garbage1));
      __asm__ __volatile__ ("add t3, %0, %1"  : : "r"(garbage0), "r"(garbage1));
      __asm__ __volatile__ ("add t4, %0, %1"  : : "r"(garbage0), "r"(garbage1));

      bp_finish(0);
}
