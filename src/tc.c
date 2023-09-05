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
    volatile uint32_t result   [BUF_SIZE] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {0};
    volatile uint32_t expected [BUF_SIZE] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};


#elif defined(TEST_CASE_2)

  #define I 8
  #define J 4
  #define K 8

  #define I_PRIME 2
  #define J_PRIME 1
  #define K_PRIME 2

  //volatile uint32_t A[1024] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {1654615998,1806341205,173879092,1112038970,2087043557,1739178872,1302718217,2046968324,1537810351,938204377,598176026,1210484339,600203567,407295012,1075916535,631194409,1332073689,424185324,316721330,1418186270,2027837527,432508404,1519522183,1864753826,1358054485,878225224,2048741382,1901353491,1118805955,267488770,60308648,400599612};
  //volatile uint32_t W[1024] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {1712934065,4908357,2119900799,1430804514,952210990,1024847607,612008755,1924014660,468399889,1294704107,1250224899,536057929,393556845,1653137829,1361705852,1039842312,1047589226,1396742090,270515404,820626892,391769539,345512145,1374624034,2101470722,1429152570,872745308,1235478542,1911213317,1246955724,789710164,813429542,801997237};
  //volatile uint32_t result[1024] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {0};
  //volatile uint32_t expected[1024] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {325473286,1406991641,11274198,1154471106,2116909427,1050136684,1843866275,696580535,1434839804,356467683,1179629868,36465208,209192502,1287827241,531781094,1540458226,2004204986,502204918,2030580771,1387259291,440307919,1889805443,339941718,1071958254,831914816,1527559948,1072401113,340548449,1751904138,49372102,1712743955,1303492523,1108115834,45492325,1371984554,363135422,115931237,1179799930,354593717,1363245217,1034237931,2099224308,779052091,1779218223,190863798,1272639657,413796966,1556732274,639592646,1283841162,677568863,63254759,1660621273,140617109,1820760128,1193363960,1505156695,858548763,1472944078,2087487094,1869852682,99157062,1628322195,1252649003};

  volatile uint32_t A[1024]        __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {1654615998,1806341205,173879092,1112038970,2087043557,1739178872,1302718217,2046968324,1537810351,938204377,598176026,1210484339,600203567,407295012,1075916535,631194409,1332073689,424185324,316721330,1418186270,2027837527,432508404,1519522183,1864753826,1358054485,878225224,2048741382,1901353491,1118805955,267488770,60308648,400599612};
  volatile uint32_t W[1024]        __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {1712934065,4908357,2119900799,1430804514,952210990,1024847607,612008755,1924014660,468399889,1294704107,1250224899,536057929,393556845,1653137829,1361705852,1039842312,1047589226,1396742090,270515404,820626892,391769539,345512145,1374624034,2101470722,1429152570,872745308,1235478542,1911213317,1246955724,789710164,813429542,801997237};
  volatile uint32_t result[1024]   __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {0};
  volatile uint32_t expected[1024] __attribute__ ((aligned (64))) __attribute__ ((section (".data"))) = {325473286,1406991641,11274198,1154471106,2116909427,1050136684,1843866275,696580535,1434839804,356467683,1179629868,36465208,209192502,1287827241,531781094,1540458226,2004204986,502204918,2030580771,1387259291,440307919,1889805443,339941718,1071958254,831914816,1527559948,1072401113,340548449,1751904138,49372102,1712743955,1303492523,1108115834,45492325,1371984554,363135422,115931237,1179799930,354593717,1363245217,1034237931,2099224308,779052091,1779218223,190863798,1272639657,413796966,1556732274,639592646,1283841162,677568863,63254759,1660621273,140617109,1820760128,1193363960,1505156695,858548763,1472944078,2087487094,1869852682,99157062,1628322195,1252649003};

#endif


//#define I_PRIME CEIL_DIV(I, 4)
//#define J_PRIME CEIL_DIV(J, 4)
//#define K_PRIME CEIL_DIV(K, 4)


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

 //#define matmul1_b0(wt,addr) \
 //    register uint64_t res1 asm("t0") = (wt); \
 //    register uint64_t res2 asm("t1") = (addr); \
 //    __asm__ __volatile__ ( \
 //        ".insn i 0x0b, 0b000, t0, t0, 64\n\t" \
 //        ".insn i 0x0b, 0b010, t1, t1, 64\n\t" \
 //        "nop\n\t" \
 //        "nop\n\t" \
 //        "nop\n\t" \
 //        : : : "t0", "t1" \
 //    );

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

        uint64_t A_base = (uint64_t)(&A);
        uint64_t W_base = (uint64_t)(&W);
        uint64_t R_base = (uint64_t)(&result);

        A_base -= 16;
        W_base -= 16;

        bool buf = 0;

        #if defined(USE_TC)
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 1; j++) {
                    for (int k = 0; k < 2; k++) {
                        uint64_t A_ptr = A_base + (64 * (j + i*1));
                        uint64_t W_ptr = W_base + (64 * (k + j*2));
                        uint64_t R_ptr = R_base + (64 * (k + i*2));

                        bp_hprint_uint64(64 * (k + i*2));
                        bp_cprint('\n');

                        bp_hprint_uint64(R_ptr);
                        bp_cprint('\n');

                        if (!buf) {
                            tensor_csr_st(0, R_ptr);
                            matmul1_b0(W_ptr, A_ptr);
                        } else {
                            tensor_csr_st(1, R_ptr);
                            matmul1_b1(W_ptr, A_ptr);
                        }

                        buf = !buf;
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
