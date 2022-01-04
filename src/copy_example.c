#include <stdint.h>

#include "bp_utils.h"

extern char _end;
// A sufficiently large address offset (in bytes) to ensure a fresh cache line
#define CACHE_LINE_SKIP_SIZE 128

void copy_function(void) {
    uint64_t print_addr = (uint64_t)(0x000000008FFFFFFF);
    uint64_t print_item = 1;

    bp_hprint(print_item); // TODO: relative addressing is bad here
}

uint64_t main(uint64_t argc, char *argv[]) {
    uint64_t *copy_start = (uint64_t *)(&copy_function);
    uint64_t *copy_end = (uint64_t *)(&main);
    uint64_t *copy_addr;
    volatile uint64_t *copy_dest = (volatile uint64_t *)(((void*)&_end) + CACHE_LINE_SKIP_SIZE); 
    void (*copied_function)() = (void *)copy_dest;

    uint64_t *silly_fill_the_dcache_address = (uint64_t *)(((void*)&_end) + 8*CACHE_LINE_SKIP_SIZE);
    
    bp_hprint_uint64(copy_start);
    bp_cprint('\n');
    bp_hprint_uint64(copy_end);
    bp_cprint('\n');

    for (copy_addr = copy_start; copy_addr < copy_end; 
            copy_addr += 1, copy_dest += 1) {
        *copy_dest = *copy_addr;
        bp_hprint_uint64(copy_dest);
        bp_cprint('\n');
    }

    // bp_hprint_uint64(copy_dest);
    // bp_cprint('\n');

    asm volatile ("fence.i");

    // for (volatile uint64_t *a = silly_fill_the_dcache_address; a < silly_fill_the_dcache_address + 128*CACHE_LINE_SKIP_SIZE; a++) {
    //     *a = 0x13371337;
    // }


    copied_function();

    return 0;
}
