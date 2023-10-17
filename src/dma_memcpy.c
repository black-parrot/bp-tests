#include <stdio.h>
#include <stdint.h>
#include "bp_asm.h"
#include "bp_utils.h"

///////////////////////////////////////////////////////////////////////////////
//
// BP_PRINTF
//

#define BP_PRINTF_MAXLEN 1024

char bp_printf_buf[BP_PRINTF_MAXLEN];

#define bp_printf(fmt, ...) \
    snprintf(bp_printf_buf, BP_PRINTF_MAXLEN, fmt, __VA_ARGS__); \
    bp_print_string(bp_printf_buf);

///////////////////////////////////////////////////////////////////////////////
//
// DMA CSR Offsets
//

#define STA 0
#define CTL 1
#define INT 2
#define RBA 3
#define WBA 4
#define MS0 5
#define ME0 6
#define LS0 7

///////////////////////////////////////////////////////////////////////////////
//
// READS CSRs Directly
//

#define read_csr(reg) ({ \
    unsigned long __tmp; \
    asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
    __tmp; \
})

///////////////////////////////////////////////////////////////////////////////
//
// MAIN (ENTRY POINT)
//

#define USE_DMA
//#define USE_MEMCPY
//#define USE_FORLOOP

volatile uint64_t src  [1024] __attribute__ ((aligned (16)));
volatile uint64_t dest [1024] __attribute__ ((aligned (16)));

int main(int argc, char** argv) {

    for (int i = 0; i < 2; i++) {

        for (int i= 0; i < 1024; i++) {
            src[i] = i+1;
            dest[i] = 0;
        }
        cbo_flush_all();

        uint64_t start_cycles = read_csr(mcycle);

        #if defined(USE_DMA)
            volatile uint64_t* base_dma_addr = 0x500000;
            *(base_dma_addr + RBA) = &src;
            *(base_dma_addr + WBA) = &dest;
            *(base_dma_addr + MS0) = 0xFFFFFFFF;
            *(base_dma_addr + ME0) = 0xFFFFFFFF;
            *(base_dma_addr + LS0) = (0x0200 << 16) | (0x04 << 8) | 0x04;
            *(base_dma_addr + CTL) = 1;

            do {} while(!*(base_dma_addr + INT));
            *(base_dma_addr + INT) = 0;

        #elif defined(USE_MEMCPY)
            memcpy(&dest, &src, sizeof(uint64_t)*1024);

        #elif defined(USE_FORLOOP)
            for (int i = 0; i < 1024; i++) {
                dest[i] = src[i];
            }

        #endif

        uint64_t end_cycles = read_csr(mcycle);
        uint64_t diff = end_cycles - start_cycles;
        bp_printf("Cycle Count = %lu\n", diff, start_cycles, end_cycles);

        uint64_t errors = 0;

        for (int i = 0; i < 1024; i++) {
            if (src[i] == dest[i]) {
            } else {
                errors++;
            }
        }

        if (errors) {
            bp_print_string("\n~~~~~ ERRORS ~~~~~\n\n");
            bp_printf("Number of errors: %lu\n", errors);

        } else {
            bp_print_string("\nTEST PASSED!!!\n\n");
        }
    }


    return 0;
}
