#include <stdio.h>
#include <stdint.h>
#include "bp_utils.h"
#include "bp_asm.h"
//#include "bp_dma.h"

//#define USE_FOR_LOOP
#define USE_TC

    #define I 4
    #define J 128
    #define K 128

// BERT-1
//    #define I 4
//    #define J 768
//    #define K 768

// BERT-2
//    #define I 4
//    #define J 3072
//    #define K 768

// BERT-3
//    #define I 4
//    #define J 768
//    #define K 3072

#define I_SUB_PRIME (4)
#define J_SUB_PRIME (4)
#define K_SUB_PRIME (4)

#define I_PRIME (I+3 / 4)
#define J_PRIME (J+3 / 4)
#define K_PRIME (K+3 / 4)

#define BUF_SIZE (128*128)

volatile uint32_t A        [BUF_SIZE] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
volatile uint32_t W        [BUF_SIZE] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17};
volatile uint32_t expected [BUF_SIZE] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {0};
volatile uint32_t result   [BUF_SIZE] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {0};

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

#define matmul1(wt,addr) \
    __asm__ __volatile__ ( \
        ".insn i 0x0b, 0b000, %0, %0, 64\n\t" \
        ".insn i 0x0b, 0b010, %1, %1, 64\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        "nop\n\t" \
        : : "r" (wt), "r" (addr)  \
    );

#define matmul2(wt,addr) ({ \
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
        : : "r" (wt), "r" (addr)  \
    ); \
})

#define matmul4(wt,addr) ({ \
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
        : : "r" (wt), "r" (addr)  \
    ); \
})

#define matmul8(wt,addr) ({ \
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
        : : "r" (wt), "r" (addr)  \
    ); \
})

int main(int argc, char** argv) {

    for (int t = 0; t < 2; t++) {
        uint64_t start_cycles = read_csr(mcycle);
        uint32_t *const R_ptr = &result;
        uint32_t *const W_ptr = &W;
        uint32_t *const A_ptr = &A;

        #if defined(USE_TC)
            for (int i = 0; i < I_PRIME; i++) {
                for (int j = 0; j < J_PRIME; j++) {
                    for (int k = 0; k < K_PRIME; k++) {
                        uint32_t a_idx = ((uint32_t)A_ptr) + (j + i*J) - 16;
                        uint32_t w_idx = ((uint32_t)W_ptr) + (k + j*K) - 16;
                        uint32_t r_idx = ((uint32_t)R_ptr) + (k + i*K);

                        tensor_csr_st(0, r_idx);
                        matmul1(a_idx, w_idx);

            }}}

        //#elif defined(USE_FOR_LOOP)
        //    for (int i = 0; i < I; i++) {
        //    for (int j = 0; j < J; j++) {
        //    for (int k = 0; k < K; k++) {
        //        uint32_t a_idx = j + i*J;
        //        uint32_t w_idx = k + j*K;
        //        uint32_t p_idx = k + i*K;
        //        result[p_idx] ^= A[a_idx] ^ W[w_idx];
        //    }}}

        #endif

        uint64_t end_cycles = read_csr(mcycle);
        uint64_t diff = end_cycles - start_cycles;
        bp_printf("Cycle Count = %lu\n", diff);

        float flops = (float)(I*J*K) / diff;
        bp_printf("FLOPS = %f\n", flops);

        uint64_t errors = 0;
        for (int i = 0; i < I*K; i++) {
            if (result[i] != expected[i]) {
                errors++;
                //bp_printf("%d: %d != %d\n", i, R[i], P[i]);
            }
        }

        bp_printf("Number of errors = %ld\n", errors);
    } // for (t)

    return 0;
}
