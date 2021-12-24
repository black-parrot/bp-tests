
// This test verifies that the low two bits of EPC are hardcoded to 00
//

#include <stdint.h>
#include "bp_utils.h"

int main(uint64_t argc, char * argv[]) {
    int mepc, sepc;

    int epc = -1;
    __asm__ __volatile__ ("csrw mepc, %0": : "r" (epc));
    __asm__ __volatile__ ("csrr  %0, mepc": "=r" (mepc): :);
    __asm__ __volatile__ ("csrr  %0, mepc": "=r" (mepc): :);

    if (mepc != -4) bp_finish(-1);

    __asm__ __volatile__ ("csrw sepc, %0": : "r" (epc));
    __asm__ __volatile__ ("csrr  %0, sepc": "=r" (sepc): :);
    __asm__ __volatile__ ("csrr  %0, sepc": "=r" (sepc): :);

    if (sepc != -4) bp_finish(-1);

    return 0;
}

