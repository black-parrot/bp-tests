#include <stdio.h>
#include <stdint.h>
#include "bp_utils.h"
#include "bp_asm.h"

#define CEIL_DIV(A,D) (((A)+(D-1))/(D))

//volatile uint32_t A[16] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {1654615998,1806341205,173879092,1112038970,2087043557,1739178872,1302718217,2046968324,1537810351,938204377,598176026,1210484339,600203567,407295012,1075916535,631194409};
//volatile uint32_t W[16] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {1332073689,424185324,316721330,1418186270,2027837527,432508404,1519522183,1864753826,1358054485,878225224,2048741382,1901353491,1118805955,267488770,60308648,400599612};
//volatile uint32_t expected[16] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {1690754557,2047255479,1893154686,473323126,160297096,390958786,496767499,1904048899,572875527,1021648205,909410692,1523968140,2077790157,1696133511,1875471694,52954182};
//volatile uint32_t result[16] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {0};

//#define USE_FOR_LOOP
#define USE_TC

#define TEST_CASE_1

#if defined(TEST_CASE_1)
    #define I 4
    #define J 4
    #define K 4

    #define BUF_SIZE (128*128)
    volatile uint32_t A        [BUF_SIZE] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {1,1,1,1,2,2,2,2,4,4,4,4,8,8,8,8};
    volatile uint32_t W        [BUF_SIZE] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {16,32,64,128,16,32,64,128,16,32,64,128,16,32,64,128};
    volatile uint32_t expected [BUF_SIZE] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};
    volatile uint32_t result   [BUF_SIZE] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {0};

#endif






#define I_PRIME CEIL_DIV(I, 4)
#define J_PRIME CEIL_DIV(J, 4)
#define K_PRIME CEIL_DIV(K, 4)


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

        uint32_t *R_ptr = &result;

        uint32_t *A_ptr = &A;
        A_ptr -= 16;

        uint32_t *W_ptr = &W;
        W_ptr -= 16;

        #if defined(USE_TC)
            for (int i = 0; i < I_PRIME; i++) {
                for (int j = 0; j < J_PRIME; j++) {
                    for (int k = 0; k < K_PRIME; k++) {
                        //uint64_t a_idx = ((uint64_t)A_ptr) + (j + i*J) - 16;
                        //uint64_t w_idx = ((uint64_t)W_ptr) + (k + j*K) - 16;
                        //uint64_t r_idx = ((uint64_t)R_ptr) + (k + i*K);

                        tensor_csr_st(0, R_ptr);
                        matmul1(A_ptr, W_ptr);
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

        //float flops = (float)(I*J*K) / diff;
        //bp_printf("FLOPS = %f\n", flops);

        uint64_t errors = 0;
        for (int i = 0; i < I*K; i++) {
            if (result[i] != expected[i]) {
                errors++;
                bp_printf("%d: %d != %d\n", i, result[i], expected[i]);
            }
        }

        bp_printf("Number of errors = %ld\n", errors);
    } // for (t)

    return 0;
}
