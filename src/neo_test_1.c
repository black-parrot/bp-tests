
#include <stdio.h>
#include <stdint.h>
#include "bp_utils.h"
#include "bp_asm.h"
#include "neo.h"

volatile uint32_t A[4096] __attribute__ ((aligned (64))) = {0};
volatile uint32_t W[4096] __attribute__ ((aligned (64))) = {0};
volatile uint32_t R[4096] __attribute__ ((aligned (64))) = {0};

int main(int argc, char** argv)
{
    cbo_flush_all();

    volatile uint32_t null = 0;
    volatile uint32_t *null_ptr = &null;

    volatile uint32_t *_a = A;
    volatile uint32_t *_w = W;
    volatile uint32_t *_r = R;

    volatile uint32_t *a_ptr = A;
    volatile uint32_t *w_ptr = W;
    volatile uint32_t *r_ptr = R;

    uint64_t start_cycles = get_cycle();

    for(int i = 0; i < 16; i++) {
    uint64_t a_offset = i * 256;
    _w = w_ptr - 16;
    for(int k = 0; k < 16; k++) {
    _a = a_ptr - 16 + i * a_offset;
    __asm__ __volatile__(".insn i 0x0b, 0b010, %0, 64(%1)" : "+r"(_a) : "r"(_a) );
    __asm__ __volatile__(".insn i 0x0b, 0b000, %0, 64(%1)" : "+r"(_w) : "r"(_w) );
    __asm__ __volatile__(".insn i 0x0b, 0b010, %0, 64(%1)" : "+r"(_a) : "r"(_a) );
    __asm__ __volatile__(".insn i 0x0b, 0b000, %0, 64(%1)" : "+r"(_w) : "r"(_w) );
    __asm__ __volatile__(".insn i 0x0b, 0b010, %0, 64(%1)" : "+r"(_a) : "r"(_a) );
    __asm__ __volatile__(".insn i 0x0b, 0b000, %0, 64(%1)" : "+r"(_w) : "r"(_w) );
    __asm__ __volatile__(".insn i 0x0b, 0b010, %0, 64(%1)" : "+r"(_a) : "r"(_a) );
    __asm__ __volatile__(".insn i 0x0b, 0b000, %0, 64(%1)" : "+r"(_w) : "r"(_w) );
    __asm__ __volatile__(".insn i 0x0b, 0b010, %0, 64(%1)" : "+r"(_a) : "r"(_a) );
    __asm__ __volatile__(".insn i 0x0b, 0b000, %0, 64(%1)" : "+r"(_w) : "r"(_w) );
    __asm__ __volatile__(".insn i 0x0b, 0b010, %0, 64(%1)" : "+r"(_a) : "r"(_a) );
    __asm__ __volatile__(".insn i 0x0b, 0b000, %0, 64(%1)" : "+r"(_w) : "r"(_w) );
    __asm__ __volatile__(".insn i 0x0b, 0b010, %0, 64(%1)" : "+r"(_a) : "r"(_a) );
    __asm__ __volatile__(".insn i 0x0b, 0b000, %0, 64(%1)" : "+r"(_w) : "r"(_w) );
    __asm__ __volatile__(".insn i 0x0b, 0b010, %0, 64(%1)" : "+r"(_a) : "r"(_a) );
    __asm__ __volatile__(".insn i 0x0b, 0b000, %0, 64(%1)" : "+r"(_w) : "r"(_w) );
    __asm__ __volatile__(".insn i 0x0b, 0b010, %0, 64(%1)" : "+r"(_a) : "r"(_a) );
    __asm__ __volatile__(".insn i 0x0b, 0b000, %0, 64(%1)" : "+r"(_w) : "r"(_w) );
    __asm__ __volatile__(".insn i 0x0b, 0b010, %0, 64(%1)" : "+r"(_a) : "r"(_a) );
    __asm__ __volatile__(".insn i 0x0b, 0b000, %0, 64(%1)" : "+r"(_w) : "r"(_w) );
    __asm__ __volatile__(".insn i 0x0b, 0b010, %0, 64(%1)" : "+r"(_a) : "r"(_a) );
    __asm__ __volatile__(".insn i 0x0b, 0b000, %0, 64(%1)" : "+r"(_w) : "r"(_w) );
    __asm__ __volatile__(".insn i 0x0b, 0b010, %0, 64(%1)" : "+r"(_a) : "r"(_a) );
    __asm__ __volatile__(".insn i 0x0b, 0b000, %0, 64(%1)" : "+r"(_w) : "r"(_w) );
    __asm__ __volatile__(".insn i 0x0b, 0b010, %0, 64(%1)" : "+r"(_a) : "r"(_a) );
    __asm__ __volatile__(".insn i 0x0b, 0b000, %0, 64(%1)" : "+r"(_w) : "r"(_w) );
    __asm__ __volatile__(".insn i 0x0b, 0b010, %0, 64(%1)" : "+r"(_a) : "r"(_a) );
    __asm__ __volatile__(".insn i 0x0b, 0b000, %0, 64(%1)" : "+r"(_w) : "r"(_w) );
    __asm__ __volatile__(".insn i 0x0b, 0b010, %0, 64(%1)" : "+r"(_a) : "r"(_a) );
    __asm__ __volatile__(".insn i 0x0b, 0b000, %0, 64(%1)" : "+r"(_w) : "r"(_w) );
    __asm__ __volatile__(".insn i 0x0b, 0b011, %0, 64(%1)" : "+r"(_a) : "r"(_a) );
    __asm__ __volatile__(".insn i 0x0b, 0b001, %0, 64(%1)" : "+r"(_w) : "r"(_w) );
    __asm__ __volatile__(".insn i 0x0b, 0b101, x0, %0, 0"::"rK"(_r));
    _r += 16;
    }
    }

    uint64_t end_cycles = get_cycle();
    uint64_t diff = end_cycles - start_cycles;
    bp_printf("Cycle Count = %lu\n", diff);
    bp_printf("Number of Ops = %lu\n", 262144);

    bp_printf("%s\n", "Test complete!");
    return 0;
}

