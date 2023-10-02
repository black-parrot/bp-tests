#ifndef __TC_H__
#define __TC_H__

#define CEIL_DIV(N,D) (((N)+(D-1))/(D))

///////////////////////////////////////////////////////////////////////////////////////////////////
// PRINTF
//

#define BP_PRINTF_MAXLEN 1024
char bp_printf_buf[BP_PRINTF_MAXLEN];
#define bp_printf(fmt, ...) \
    snprintf(bp_printf_buf, BP_PRINTF_MAXLEN, fmt, __VA_ARGS__); \
    bp_print_string(bp_printf_buf);

///////////////////////////////////////////////////////////////////////////////////////////////////
// NUKE L2
//

// 128KB = 16K * 8 Bytes
volatile uint64_t l2nuke [16*1024] __attribute__ ((aligned (64))) = {0};

void nuke_l2 () {
    bp_print_string("Nuking l2... ");
    for (int q = 0; q < 16*1024; q++) {
        uint64_t a = l2nuke[q];
    }
    bp_print_string("Done!\n");
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// GENERAL ASM
//

#define nop() ({ \
    asm volatile ("nop"); \
})

#define read_csr(reg) ({ \
    unsigned long __tmp; \
    asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
    __tmp; \
})

#define get_cycle() read_csr(mcycle)

///////////////////////////////////////////////////////////////////////////////////////////////////
// TENSOR CORE ASM
//

#define tensor_wl(addr,buf,incr) ({                             \
    asm volatile (                                              \
         ".insn i 0x0b, 0b00" #buf ", %0, %0, " #incr "\n\t"    \
         : "+r" (addr)                                          \
         :                                                      \
    );                                                          \
})

#define tensor_al(addr,buf,incr) ({                             \
    asm volatile (                                              \
         ".insn i 0x0b, 0b01" #buf ", %0, %0, " #incr "\n\t"    \
         : "+r" (addr)                                          \
         :                                                      \
    );                                                          \
})

#endif // __TC_H__
