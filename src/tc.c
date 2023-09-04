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

//#define TEST_CASE_1
#define TEST_CASE_2

#if defined(TEST_CASE_1)
    #define I 4
    #define J 4
    #define K 4

    #define BUF_SIZE (128*128)

    volatile uint32_t A        [BUF_SIZE] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {1,1,1,1,2,2,2,2,4,4,4,4,8,8,8,8};
    volatile uint32_t W        [BUF_SIZE] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {16,32,64,128,16,32,64,128,16,32,64,128,16,32,64,128};
    volatile uint32_t expected [BUF_SIZE] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};
    volatile uint32_t result   [BUF_SIZE] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {0};

#elif defined(TEST_CASE_2)

    #define I 8
    #define J 8
    #define K 8

    volatile uint32_t A[64] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {1654615998,1806341205,173879092,1112038970,1537810351,938204377,598176026,1210484339,1332073689,424185324,316721330,1418186270,1358054485,878225224,2048741382,1901353491,2087043557,1739178872,1302718217,2046968324,600203567,407295012,1075916535,631194409,2027837527,432508404,1519522183,1864753826,1118805955,267488770,60308648,400599612,1712934065,4908357,2119900799,1430804514,952210990,1024847607,612008755,1924014660,468399889,1294704107,1250224899,536057929,393556845,1653137829,1361705852,1039842312,1047589226,1396742090,270515404,820626892,391769539,345512145,1374624034,2101470722,1429152570,872745308,1235478542,1911213317,1246955724,789710164,813429542,801997237};
    volatile uint32_t W[64] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {141615452,1116932915,2046685052,296718909,344663896,1680603714,1183839549,1011472239,2115938757,1534881946,353789296,1392964597,1043830061,69617246,1164099199,503087954,385792672,559309739,642313784,165964262,924282500,1801409089,1182021441,1935153793,495423391,2089261940,1439975734,817670926,946870804,1597986360,732213632,1428231901,1830219288,267114565,432105997,628604922,534481714,809689893,514088150,1680231637,92928119,835761764,794448426,532125690,1828018965,435940125,1116347426,300666185,939625287,194278833,317814271,114661864,393126321,1589738942,498414388,156302611,2058292873,904419226,262357606,97850419,948454521,309116047,1293049471,1504501144};
    volatile uint32_t expected[64] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {937058776,1005175177,1189011267,1667632652,1167007389,1237106380,881565702,288584009,1327489230,1126973599,1041981013,463703834,1117208861,1319428428,865337222,371928777,1092863894,1589265546,1637809474,2140605120,863418579,753875919,332104199,230847877,971392640,645907868,425426004,122850262,879683411,737633359,349429127,182069765,1450695144,1514437049,661874547,46904892,887016570,955083819,1171944161,1617077166,1121110309,1323395444,869693374,376350449,1632024899,1836390674,272762840,905631383,546209702,1058536634,3360114,507163376,1109879348,1572792616,1654280416,2090051426,875261803,733146231,345396671,178233917,398401293,136748049,922887641,688640603};
    volatile uint32_t result[64] __attribute__ ((aligned (64))) __attribute__ ((section (".data")));

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

    for (int t = 0; t < 1; t++) {

        uint64_t start_cycles = read_csr(mcycle);

        uint32_t *R_base = &result;

        uint32_t *A_base = &A;
        A_base -= 16;

        uint32_t *W_base = &W;
        W_base -= 16;

        #if defined(USE_TC)
            for (int i = 0; i < I_PRIME; i++) {
                for (int j = 0; j < J_PRIME; j++) {
                    for (int k = 0; k < K_PRIME; k++) {
                        uint32_t *A_ptr = A_base + 16 * (j + i*J);
                        uint32_t *W_ptr = W_base + 16 * (k + j*K);
                        uint32_t *R_ptr = R_base + 16 * (k + i*K);

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

        float flops = (float)(I*J*K) / diff;
        bp_printf("FLOPS = %f\n", flops);

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
