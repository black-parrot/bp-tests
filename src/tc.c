#include <stdio.h>
#include <stdint.h>
#include "bp_utils.h"
#include "tc.h"

#define CEIL_DIV(A,D) (((A)+(D-1))/(D))

//#include "tc_1.h"
//#include "tc_2.h"
//#include "tc_3.h"
//#include "tc_4.h"
//#include "tc_5.h"
//#include "tc_6.h"
//#include "tc_7.h"
//#include "tc_8.h"
//#include "tc_9.h"
#include "tc_10.h"

#define BP_PRINTF_MAXLEN 1024

char bp_printf_buf[BP_PRINTF_MAXLEN];

#define bp_printf(fmt, ...) \
    snprintf(bp_printf_buf, BP_PRINTF_MAXLEN, fmt, __VA_ARGS__); \
    bp_print_string(bp_printf_buf);

#define nop() ({ asm volatile ("nop"); })

#define read_csr(reg) ({ \
    unsigned long __tmp; \
    asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
    __tmp; \
})

#define matmul1_b0(wt,addr) \
     __asm__ __volatile__ ( \
         ".insn i 0x0b, 0b000, %0, %0, 64\n\t" \
         ".insn i 0x0b, 0b010, %1, %1, 64\n\t" \
         "nop\n\t" \
         "nop\n\t" \
         "nop\n\t" \
        : "+r" (wt), "+r" (addr) : \
     );

#define matmul1_b1(wt,addr) \
    __asm__ __volatile__ ( \
        ".insn i 0x0b, 0b001, %0, %0, 64\n\t" \
        ".insn i 0x0b, 0b011, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        : "+r" (wt), "+r" (addr) : \
    );

#define matmul2_b0(wt,addr) ({ \
    __asm__ __volatile__ ( \
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

#define matmul2_b1(wt,addr) ({ \
    __asm__ __volatile__ ( \
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

#define matmul4_b0(wt,addr) ({ \
    __asm__ __volatile__ ( \
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

#define matmul4_b1(wt,addr) ({ \
    __asm__ __volatile__ ( \
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

#define matmul8_b0(wt,addr) ({ \
    __asm__ __volatile__ ( \
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

#define matmul8_b1(wt,addr) ({ \
    __asm__ __volatile__ ( \
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

void check() {
    uint64_t errors = 0;
    for (int i = 0; i < I*K; i++) {
        if (R[i] != E[i]) {
            errors++;
            //bp_printf("%d: %d != %d\n", i, R[i], E[i]);
            //bp_printf("%d\n", i);
        }
    }
    bp_printf("Number of errors = %ld\n", errors);
}

void check_blk() {
    uint64_t errors = 0;
    for (int i = 0; i < I*K*(J/4); i++) {
        if (R_BLK[i] != E_BLK[i]) {
            errors++;
            //bp_printf("%d: %d != %d\n", i, R[i], E[i]);
            //bp_printf("%d\n", i);
        }
    }
    bp_printf("Number of errors = %ld\n", errors);
}

int main(int argc, char** argv) {
    for (int t = 0; t < 1; t++) {
        uint64_t start_cycles = read_csr(mcycle);

        bool buf = 0;

        uint64_t A_base = (uint64_t)(&A[0]);
        uint64_t W_base = (uint64_t)(&W[0]);
        uint64_t R_base = (uint64_t)(&R_BLK[0]);

        A_base -= 64;
        W_base -= 64;

        const int IBLK = I / 4;
        const int JBLK = J / 4;
        const int KBLK = K / 4;

        for (uint64_t j = 0; j < JBLK; j++) {
        for (uint64_t k = 0; k < KBLK; k++) {
        for (uint64_t i = 0; i < IBLK; i++) {

            uint64_t W_offset = (k + j*KBLK) * 64;
            uint64_t W_ptr = W_base + W_offset;

            uint64_t A_offset = (j + i*JBLK) * 64;
            uint64_t A_ptr = A_base + A_offset;

            uint64_t R_offset = (k + i*KBLK + j*KBLK*IBLK) * 64;
            uint64_t R_ptr = R_base + R_offset;

            nop();
            nop();
            nop();
            if (!buf) {
                tensor_csr_st(0, R_ptr);
                matmul1_b0(W_ptr, A_ptr);
            } else {
                tensor_csr_st(1, R_ptr);
                matmul1_b1(W_ptr, A_ptr);
            }
            buf = !buf;
        }}}

        uint64_t end_cycles = read_csr(mcycle);
        uint64_t diff = end_cycles - start_cycles;
        bp_printf("Cycle Count = %lu\n", diff);

        check_blk();

        //float flops = (float)(I*J*K) / diff;
        //bp_printf("FLOPS = %f\n", flops);
    }

    return 0;
}
