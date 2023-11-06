#ifndef __NEO_H__
#define __NEO_H__

#include "bp_printf.h"

// BP ASM
#define nop() __asm__ __volatile__("nop")
#define get_cycle() read_csr(mcycle)

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

#endif // __NEO_H__
