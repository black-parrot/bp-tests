
// This test verifies that the LSU throws appropriate misaligned exceptions
//

#include <stdint.h>
#include "bp_utils.h"

void trap_store() {
    uint64_t mcause;
    __asm__ __volatile__ ("csrr %0, mcause" : "=r" (mcause));

    if (mcause != 6) bp_finish(1);

    uint64_t return_pc;
    __asm__ __volatile__ ("csrr  %0, mepc": "=r" (return_pc): :);
    return_pc += 8;
    __asm__ __volatile__ ("csrw mepc, %0": : "r" (return_pc));
    __asm__ __volatile__ ("mret");
}

void trap_load() {
    uint64_t mcause;
    __asm__ __volatile__ ("csrr %0, mcause" : "=r" (mcause));

    if (mcause != 4) bp_finish(1);

    uint64_t return_pc;
    __asm__ __volatile__ ("csrr  %0, mepc": "=r" (return_pc): :);
    return_pc += 8;
    __asm__ __volatile__ ("csrw mepc, %0": : "r" (return_pc));
    __asm__ __volatile__ ("mret");
}

uint64_t *misaligned_addr_1 = (uint64_t *) 0x80100001;
uint64_t *misaligned_addr_2 = (uint64_t *) 0x80100002;
uint64_t *misaligned_addr_3 = (uint64_t *) 0x80100003;
uint64_t *misaligned_addr_4 = (uint64_t *) 0x80100004;
uint64_t *misaligned_addr_7 = (uint64_t *) 0x80100007;

int main(uint64_t argc, char * argv[]) {
    // Set up trap to alternate handler
    __asm__ __volatile__ ("csrw mtvec, %0": : "r" (&trap_store));

    __asm__ __volatile__ ("fmv.d.x f0, x0" ::);

    // Check store alignment
    // Store bytes should not trap
    __asm__ __volatile__ ("sb x0, 0(%0)": : "r" (misaligned_addr_1));
    __asm__ __volatile__ ("sb x0, 0(%0)": : "r" (misaligned_addr_2));
    __asm__ __volatile__ ("sb x0, 0(%0)": : "r" (misaligned_addr_3));
    __asm__ __volatile__ ("sb x0, 0(%0)": : "r" (misaligned_addr_4));
    __asm__ __volatile__ ("sb x0, 0(%0)": : "r" (misaligned_addr_7));

    // Store halves should trap on certain alignments
    __asm__ __volatile__ ("sh x0, 0(%0)": : "r" (misaligned_addr_1));
    bp_finish(1);
    __asm__ __volatile__ ("sh x0, 0(%0)": : "r" (misaligned_addr_2));
    __asm__ __volatile__ ("sh x0, 0(%0)": : "r" (misaligned_addr_3));
    bp_finish(1);
    __asm__ __volatile__ ("sh x0, 0(%0)": : "r" (misaligned_addr_4));
    __asm__ __volatile__ ("sh x0, 0(%0)": : "r" (misaligned_addr_7));
    bp_finish(1);

    // Store words should trap on certain alignments
    __asm__ __volatile__ ("sw x0, 0(%0)": : "r" (misaligned_addr_1));
    bp_finish(1);
    __asm__ __volatile__ ("sw x0, 0(%0)": : "r" (misaligned_addr_2));
    bp_finish(1);
    __asm__ __volatile__ ("sw x0, 0(%0)": : "r" (misaligned_addr_3));
    bp_finish(1);
    __asm__ __volatile__ ("sw x0, 0(%0)": : "r" (misaligned_addr_4));
    __asm__ __volatile__ ("sw x0, 0(%0)": : "r" (misaligned_addr_7));
    bp_finish(1);

    // Store double should trap on certain alignments
    __asm__ __volatile__ ("sd x0, 0(%0)": : "r" (misaligned_addr_1));
    bp_finish(1);
    __asm__ __volatile__ ("sd x0, 0(%0)": : "r" (misaligned_addr_2));
    bp_finish(1);
    __asm__ __volatile__ ("sd x0, 0(%0)": : "r" (misaligned_addr_3));
    bp_finish(1);
    __asm__ __volatile__ ("sd x0, 0(%0)": : "r" (misaligned_addr_4));
    bp_finish(1);
    __asm__ __volatile__ ("sd x0, 0(%0)": : "r" (misaligned_addr_7));
    bp_finish(1);

    // Check amo alignment
    // AMO words should trap on certain alignments
    __asm__ __volatile__ ("amoadd.w x0, x0, 0(%0)": : "r" (misaligned_addr_1));
    bp_finish(1);
    __asm__ __volatile__ ("amoadd.w x0, x0, 0(%0)": : "r" (misaligned_addr_2));
    bp_finish(1);
    __asm__ __volatile__ ("amoadd.w x0, x0, 0(%0)": : "r" (misaligned_addr_3));
    bp_finish(1);
    __asm__ __volatile__ ("amoadd.w x0, x0, 0(%0)": : "r" (misaligned_addr_4));
    __asm__ __volatile__ ("amoadd.w x0, x0, 0(%0)": : "r" (misaligned_addr_7));
    bp_finish(1);

    // AMO doubles should trap on certain alignments
    __asm__ __volatile__ ("amoadd.d x0, x0, 0(%0)": : "r" (misaligned_addr_1));
    bp_finish(1);
    __asm__ __volatile__ ("amoadd.d x0, x0, 0(%0)": : "r" (misaligned_addr_2));
    bp_finish(1);
    __asm__ __volatile__ ("amoadd.d x0, x0, 0(%0)": : "r" (misaligned_addr_3));
    bp_finish(1);
    __asm__ __volatile__ ("amoadd.d x0, x0, 0(%0)": : "r" (misaligned_addr_4));
    bp_finish(1);
    __asm__ __volatile__ ("amoadd.d x0, x0, 0(%0)": : "r" (misaligned_addr_7));
    bp_finish(1);

    // Check fstore alignment
    // fstore words should trap on certain alignments
    __asm__ __volatile__ ("fsw f0, 0(%0)": : "r" (misaligned_addr_1));
    bp_finish(1);
    __asm__ __volatile__ ("fsw f0, 0(%0)": : "r" (misaligned_addr_2));
    bp_finish(1);
    __asm__ __volatile__ ("fsw f0, 0(%0)": : "r" (misaligned_addr_3));
    bp_finish(1);
    __asm__ __volatile__ ("fsw f0, 0(%0)": : "r" (misaligned_addr_4));
    __asm__ __volatile__ ("fsw f0, 0(%0)": : "r" (misaligned_addr_7));
    bp_finish(1);

    // fstore doubles should trap on certain alignments
    __asm__ __volatile__ ("fsd f0, 0(%0)": : "r" (misaligned_addr_1));
    bp_finish(1);
    __asm__ __volatile__ ("fsd f0, 0(%0)": : "r" (misaligned_addr_2));
    bp_finish(1);
    __asm__ __volatile__ ("fsd f0, 0(%0)": : "r" (misaligned_addr_3));
    bp_finish(1);
    __asm__ __volatile__ ("fsd f0, 0(%0)": : "r" (misaligned_addr_4));
    bp_finish(1);
    __asm__ __volatile__ ("fsd f0, 0(%0)": : "r" (misaligned_addr_7));
    bp_finish(1);

    // Check load alignment
    __asm__ __volatile__ ("csrw mtvec, %0": : "r" (&trap_load));
    // Load bytes should not trap
    __asm__ __volatile__ ("lb x0, 0(%0)": : "r" (misaligned_addr_1));
    __asm__ __volatile__ ("lb x0, 0(%0)": : "r" (misaligned_addr_2));
    __asm__ __volatile__ ("lb x0, 0(%0)": : "r" (misaligned_addr_3));
    __asm__ __volatile__ ("lb x0, 0(%0)": : "r" (misaligned_addr_4));
    __asm__ __volatile__ ("lb x0, 0(%0)": : "r" (misaligned_addr_7));

    // Load halves should trap on certain alignments
    __asm__ __volatile__ ("lh x0, 0(%0)": : "r" (misaligned_addr_1));
    bp_finish(1);
    __asm__ __volatile__ ("lh x0, 0(%0)": : "r" (misaligned_addr_2));
    __asm__ __volatile__ ("lh x0, 0(%0)": : "r" (misaligned_addr_3));
    bp_finish(1);
    __asm__ __volatile__ ("lh x0, 0(%0)": : "r" (misaligned_addr_4));
    __asm__ __volatile__ ("lh x0, 0(%0)": : "r" (misaligned_addr_7));
    bp_finish(1);

    // Load words should trap on certain alignments
    __asm__ __volatile__ ("lw x0, 0(%0)": : "r" (misaligned_addr_1));
    bp_finish(1);
    __asm__ __volatile__ ("lw x0, 0(%0)": : "r" (misaligned_addr_2));
    bp_finish(1);
    __asm__ __volatile__ ("lw x0, 0(%0)": : "r" (misaligned_addr_3));
    bp_finish(1);
    __asm__ __volatile__ ("lw x0, 0(%0)": : "r" (misaligned_addr_4));
    __asm__ __volatile__ ("lw x0, 0(%0)": : "r" (misaligned_addr_7));
    bp_finish(1);

    // Load double should trap on certain alignments
    __asm__ __volatile__ ("ld x0, 0(%0)": : "r" (misaligned_addr_1));
    bp_finish(1);
    __asm__ __volatile__ ("ld x0, 0(%0)": : "r" (misaligned_addr_2));
    bp_finish(1);
    __asm__ __volatile__ ("ld x0, 0(%0)": : "r" (misaligned_addr_3));
    bp_finish(1);
    __asm__ __volatile__ ("ld x0, 0(%0)": : "r" (misaligned_addr_4));
    bp_finish(1);
    __asm__ __volatile__ ("ld x0, 0(%0)": : "r" (misaligned_addr_7));
    bp_finish(1);

    // fload words should trap on certain alignments
    __asm__ __volatile__ ("flw f0, 0(%0)": : "r" (misaligned_addr_1));
    bp_finish(1);
    __asm__ __volatile__ ("flw f0, 0(%0)": : "r" (misaligned_addr_2));
    bp_finish(1);
    __asm__ __volatile__ ("flw f0, 0(%0)": : "r" (misaligned_addr_3));
    bp_finish(1);
    __asm__ __volatile__ ("flw f0, 0(%0)": : "r" (misaligned_addr_4));
    __asm__ __volatile__ ("flw f0, 0(%0)": : "r" (misaligned_addr_7));
    bp_finish(1);

    // fload double should trap on certain alignments
    __asm__ __volatile__ ("fld f0, 0(%0)": : "r" (misaligned_addr_1));
    bp_finish(1);
    __asm__ __volatile__ ("fld f0, 0(%0)": : "r" (misaligned_addr_2));
    bp_finish(1);
    __asm__ __volatile__ ("fld f0, 0(%0)": : "r" (misaligned_addr_3));
    bp_finish(1);
    __asm__ __volatile__ ("fld f0, 0(%0)": : "r" (misaligned_addr_4));
    bp_finish(1);
    __asm__ __volatile__ ("fld f0, 0(%0)": : "r" (misaligned_addr_7));
    bp_finish(1);

    return 0;
}

