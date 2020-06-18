#include <stdint.h>
#include <bp_utils.h>

void trap_success() {
      uint64_t mcause;

      __asm__ __volatile__ ("csrr %0, mcause" : "=r" (mcause));

      if (mcause == 15)
        bp_finish(0);

      bp_finish(1);
}

void main(uint64_t argc, char * argv[]) {
      // MPP bits are set as 01 (supervisor) and MPRV bit is set to 1
      uint64_t mstatus = ((1 << 17) | (1 << 11));
      // satp has the PPN as 0 and mode = sv39
      uint64_t satp = 0x8000000000000000;

      // Set up trap to alternate handler
      __asm__ __volatile__ ("csrw mtvec, %0": : "r" (&trap_success));

      __asm__ __volatile__ ("csrw mstatus, %0": : "r" (mstatus));
      __asm__ __volatile__ ("csrw satp, %0": : "r" (satp));

      // Sending a store to an address that would trigger an eaddr page fault
      __asm__ __volatile__ ("li t0, 0x470000000000");
      __asm__ __volatile__ ("sd zero, 0(t0)");
     
      bp_finish(1);
}
