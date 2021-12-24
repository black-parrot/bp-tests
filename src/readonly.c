#include <stdint.h>
#include <bp_utils.h>

void trap_continue() {
    uint64_t mcause;
    __asm__ __volatile__ ("csrr %0, mcause" : "=r" (mcause));

    if (mcause != 2)
        bp_finish(1);

    uint64_t return_pc;
    __asm__ __volatile__ ("csrr  %0, mepc": "=r" (return_pc): :);
    return_pc += 8;
    __asm__ __volatile__ ("csrw mepc, %0": : "r" (return_pc));
    __asm__ __volatile__ ("mret");
}

int main(uint64_t argc, char * argv[]) {
    // MPP bits are set as 01 (supervisor) and MPRV bit is set to 1
    uint64_t mstatus = ((1 << 17) | (1 << 11));
    // satp has the PPN as 0 and mode = sv39
    uint64_t satp = 0x8000000000000000;

    // Set up trap to alternate handler
    __asm__ __volatile__ ("csrw mtvec, %0": : "r" (&trap_continue));

    // Attempt to write read-only CSRs
    __asm__ __volatile__ ("csrw mvendorid, %0": : "r" (-1));
    bp_finish(1);
    __asm__ __volatile__ ("csrw marchid, %0": : "r" (-1));
    bp_finish(1);
    __asm__ __volatile__ ("csrw mimpid, %0": : "r" (-1));
    bp_finish(1);
    __asm__ __volatile__ ("csrw mhartid, %0": : "r" (-1));
    bp_finish(1);

    return 0;
}
