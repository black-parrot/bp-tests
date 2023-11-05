#ifndef __NEO_H__
#define __NEO_H__

#include "../bp_printf.h"
#include "mm.h"


// BP ASM
#define nop() __asm__ __volatile__("nop")
#define get_cycle() read_csr(mcycle)

// TensorCore ASM
#define tc_nop() __asm__ __volatile__(".insn i 0x0b, 0b100, x0, x0, 0"::);
#define tc_load_w(ptr) __asm__ __volatile__(".insn i 0x0b, 0b000, %0, %0, 0":"+r"(ptr):);
#define tc_load_a(ptr) __asm__ __volatile__(".insn i 0x0b, 0b010, %0, %0, 0":"+r"(ptr):);

#define tc_load_a_off(src,dest,off) ({ \
    __asm__ __volatile__(".insn i 0x0b, 0b010, %0, %1, " #off "" : "+r"(dest) : "r"(src) ); \
})
#define tc_load_a_off_last(src,dest,off) ({ \
    __asm__ __volatile__(".insn i 0x0b, 0b011, %0, %1, " #off "" : "+r"(dest) : "r"(src) ); \
})

#define tc_load_w_off(ptr,off) ({ \
    __asm__ __volatile__(".insn i 0x0b, 0b000, %0, %0, " #off "" : : "r"(ptr)); \
})
#define tc_load_w_off_last(ptr,off) ({ \
    __asm__ __volatile__(".insn i 0x0b, 0b001, %0, %0, " #off "" : : "r"(ptr)); \
})

#define tc_store(ptr)  __asm__ __volatile__(".insn i 0x0b, 0b101, x0, %0, 0"::"rK"(ptr));

/**
 * Convert an I by J matrix into a tiled matrix.
 */
void tile_matrix_act (
    volatile uint32_t* src,
    volatile uint32_t* dest,
    const int I,
    const int J
) {
    const int tx = 4;
    const int ty = 4;
    const int block_size = tx * ty;
    const int num_blocks_x = J / tx;
    const int num_blocks_y = I / ty;

    for (int i = 0; i < I; i++) {
        for (int j = 0; j < J; j++) {
            int src_idx = i*J + j;

            int by = i / ty;
            int bx = j / tx;
            int ly = i % ty;
            int lx = j % tx;

            int dest_idx = by * (num_blocks_x * block_size) + bx * (block_size) + ly * tx + lx;
            dest[dest_idx] = src[src_idx];
        }
    }
}

/**
 * Convert an I by J matrix into a tiled matrix.
 */
void tile_matrix_weight (
    volatile uint32_t* src,
    volatile uint32_t* dest,
    const int I,
    const int J
) {
    const int tx = 4;
    const int ty = 4;
    const int block_size = tx * ty;
    const int num_blocks_x = J / tx;
    const int num_blocks_y = I / ty;

    for (int i = 0; i < I; i++) {
        for (int j = 0; j < J; j++) {
            int src_idx = i*J + j;

            int by = i / ty;
            int bx = j / tx;
            int ly = i % ty;
            int lx = j % tx;

            int dest_idx = bx * (num_blocks_y * block_size) + by * (block_size) + ly * tx + lx;
            dest[dest_idx] = src[src_idx];
        }
    }
}


/**
 * Convert an tiled matrix into a I by J matrix
 */
void untile_matrix_act (
    volatile uint32_t* src,
    volatile uint32_t* dest,
    const int I,
    const int J
) {
    const int tx = 4;
    const int ty = 4;
    const int block_size = tx * ty;
    const int num_blocks_x = J / tx;
    const int num_blocks_y = I / ty;

    for (int i = 0; i < I; i++) {
        for (int j = 0; j < J; j++) {
            int dest_idx = i*J + j;

            int by = i / ty;
            int bx = j / tx;
            int ly = i % ty;
            int lx = j % tx;

            int src_idx = by * (num_blocks_x * block_size) + bx * (block_size) + ly * tx + lx;
            dest[dest_idx] = src[src_idx];
        }
    }
}

/**
 * Convert an tiled matrix into a I by J matrix
 */
void untile_matrix_weight (
    volatile uint32_t* src,
    volatile uint32_t* dest,
    const int I,
    const int J
) {
    const int tx = 4;
    const int ty = 4;
    const int block_size = tx * ty;
    const int num_blocks_x = J / tx;
    const int num_blocks_y = I / ty;

    for (int i = 0; i < I; i++) {
        for (int j = 0; j < J; j++) {
            int dest_idx = i*J + j;

            int by = i / ty;
            int bx = j / tx;
            int ly = i % ty;
            int lx = j % tx;

            int src_idx = bx * (num_blocks_y * block_size) + by * (block_size) + ly * tx + lx;
            dest[dest_idx] = src[src_idx];
        }
    }
}


/**
 * Perform a tiled matmul on the CPU.
 */
void tiled_matmul_cpu (
    volatile uint32_t* a_ptr,
    volatile uint32_t* w_ptr,
    volatile uint32_t* r_ptr,
    const int I,
    const int J,
    const int K
) {
    for (int i = 0; i < I*K; i++) { r_ptr[i] = 0; }

    const int IBLK = I / 4;
    const int JBLK = J / 4;
    const int KBLK = K / 4;

    const uint64_t A_base = (uint64_t)(&a_ptr[0]);
    const uint64_t W_base = (uint64_t)(&w_ptr[0]);
    const uint64_t R_base = (uint64_t)(&r_ptr[0]);

    for (uint64_t k1 = 0; k1 < KBLK; k1++) {
    for (uint64_t i1 = 0; i1 < IBLK; i1++) {
    for (uint64_t j1 = 0; j1 < JBLK; j1++) {
        for (int k0 = 0; k0 < 4; k0++) {
        for (int i0 = 0; i0 < 4; i0++) {
        for (int j0 = 0; j0 < 4; j0++) {
            uint64_t a_idx = i1*(JBLK*16) + i0*4 + j1*16 + j0;
            uint64_t w_idx = k1*(JBLK*16) + j0*4 + j1*16 + k0;
            uint64_t r_idx = i1*(KBLK*16) + i0*4 + k1*16 + k0;
            r_ptr[r_idx] ^= a_ptr[a_idx] ^ w_ptr[w_idx];
        }}}
    }}}
}


/**
 * Perform a normal matmul on the CPU.
 */
void matmul_cpu (
    volatile uint32_t* a_ptr,
    volatile uint32_t* w_ptr,
    volatile uint32_t* r_ptr,
    const int I,
    const int J,
    const int K
) {
    for (int i = 0; i < I*K; i++) { r_ptr[i] = 0; }

    for (int i = 0; i < I; i++) {
    for (int j = 0; j < J; j++) {
    for (int k = 0; k < K; k++) {
        uint64_t a_idx = j + i*J;
        uint64_t w_idx = k + j*K;
        uint64_t r_idx = k + i*K;
        r_ptr[r_idx] ^= a_ptr[a_idx] ^ w_ptr[w_idx];
    }}}
}


/**
 *
 */
void matmul_os (
    volatile uint32_t* a_ptr,
    volatile uint32_t* w_ptr,
    volatile uint32_t* r_ptr,
    const int I,
    const int J,
    const int K
) {
    const int IBLK = I / 4;
    const int JBLK = J / 4;
    const int KBLK = K / 4;

    //uint32_t dnull;
    //volatile uint32_t* foo = &dnull;

    uint64_t start_cycles = get_cycle();

    mm(a_ptr, w_ptr, r_ptr);

    uint64_t end_cycles = get_cycle();
    uint64_t diff = end_cycles - start_cycles;
    bp_printf("Cycle Count = %lu\n", diff);

    uint64_t FLOPS = (I*J*K) / diff;
    bp_printf("FLOPs (floor) = %lu\n", FLOPS);
}


/**
 *
 */
bool check_array (
    uint32_t *result,
    uint32_t *expected,
    uint32_t els,
    bool debug
) {
    int errors = 0;
    for (int i = 0; i < els; i++) {
        if (result[i] != expected[i]) {
            if (debug) {
                bp_printf("%d: %x != %x\n", i, result[i], expected[i]);
            }
            errors++;
        }
    }
    bp_printf("Number of errors: %d\n", errors);
    return (errors == 0);
}


/**
 *
 */
void print_array_hex (uint32_t *arr, uint32_t els)
{
    for (int i = 0; i < els; i++) {
        bp_printf("%d: %x\n", i, arr[i]);
    }
}


/**
 *
 */
void print_array_dec (uint32_t *arr, uint32_t els)
{
    for (int i = 0; i < els; i++) {
        bp_printf("%d: %d\n", i, arr[i]);
    }
}

#endif __NEO_H__
