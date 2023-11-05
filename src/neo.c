#include <stdio.h>
#include <stdint.h>
#include "bp_utils.h"
#include "bp_asm.h"
#include "neo/neo.h"


int main(int argc, char** argv)
{
    int errors = 0;

    volatile uint32_t A[  4 * 768] __attribute__ ((aligned (64)));
    volatile uint32_t W[768 * 768] __attribute__ ((aligned (64)));
    volatile uint32_t R[  4 * 768] __attribute__ ((aligned (64)));
    cbo_flush_all();
    matmul_os(A, W, R, 4, 768, 768);
    matmul_os(A, W, R, 4, 768, 768);
    cbo_flush_all();

    bp_printf("%s\n", "Test complete!");
    return 0;
}
