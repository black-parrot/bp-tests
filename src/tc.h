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

#define mm1b0(wt,addr) \
     asm volatile ( \
         ".insn i 0x0b, 0b000, %0, %0, 64\n\t" \
         ".insn i 0x0b, 0b010, %1, %1, 64\n\t" \
         "nop\n\t" \
         "nop\n\t" \
         "nop\n\t" \
        : "+r" (wt), "+r" (addr) : \
     );

#define mm1b1(wt,addr) \
    asm volatile ( \
        ".insn i 0x0b, 0b001, %0, %0, 64\n\t" \
        ".insn i 0x0b, 0b011, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        : "+r" (wt), "+r" (addr) : \
    );

#define mm2b0(wt,addr) ({ \
    asm volatile ( \
        ".insn i 0x0b, 0b000, %0, %0, 64\n\t" \
        ".insn i 0x0b, 0b010, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b010, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        : "+r" (wt), "+r" (addr) : \
    ); \
})

#define mm2b1(wt,addr) ({ \
    asm volatile ( \
        ".insn i 0x0b, 0b001, %0, %0, 64\n\t" \
        ".insn i 0x0b, 0b011, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b011, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        : "+r" (wt), "+r" (addr) : \
    ); \
})

#define mm4b0(wt,addr) ({ \
    asm volatile ( \
        ".insn i 0x0b, 0b000, %0, %0, 64\n\t" \
        ".insn i 0x0b, 0b010, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b010, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b010, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b010, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        : "+r" (wt), "+r" (addr) : \
    ); \
})

#define mm4b1(wt,addr) ({ \
    asm volatile ( \
        ".insn i 0x0b, 0b001, %0, %0, 64\n\t" \
        ".insn i 0x0b, 0b011, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b011, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b011, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b011, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        : "+r" (wt), "+r" (addr) : \
    ); \
})

#define mm8b0(wt,addr) ({ \
    asm volatile ( \
        ".insn i 0x0b, 0b000, %0, %0, 64\n\t" \
        ".insn i 0x0b, 0b010, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b010, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b010, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b010, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b010, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b010, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b010, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b010, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        : "+r" (wt), "+r" (addr) : \
    ); \
})

#define mm8b1(wt,addr) ({ \
    asm volatile ( \
        ".insn i 0x0b, 0b001, %0, %0, 64\n\t" \
        ".insn i 0x0b, 0b011, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b011, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b011, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b011, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b011, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b011, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b011, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        ".insn i 0x0b, 0b011, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        : "+r" (wt), "+r" (addr) : \
    ); \
})

#endif // __TC_H__
