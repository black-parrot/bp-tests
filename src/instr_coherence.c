#include <stdint.h>

#include "bp_utils.h"

#pragma GCC push_options
#pragma GCC optimize ("align-functions=16")
void copy_function(void) {
    bp_cprint('h');
    bp_cprint('e');
    bp_cprint('l');
    bp_cprint('l');
    bp_cprint('o');
    bp_cprint('\n');
}
#pragma GCC pop_options

uint64_t main(uint64_t argc, char *argv[]) {
    uint64_t *copy_start = (uint64_t *)(copy_function);
    uint64_t *copy_end = (uint64_t *)(copy_function) + 0x200;
    uint64_t *copy_addr;
    uint64_t *copy_dest = (uint64_t *)(copy_function) + 0x400; 
    void (*copied_function)() = (void *)copy_dest;
    
    for (copy_addr = copy_start; copy_addr < copy_end; 
            copy_addr += 8, copy_dest += 8) {
        *copy_dest = *copy_addr;
    }

    __asm__ volatile("fence.i");
    copied_function();

    return 0;
}

