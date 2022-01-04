
#include <stdint.h>
#include "bp_utils.h"

#define NUM_TEST_INSTRUCTIONS (sizeof(preinitialized_instructions)/sizeof(uint32_t))

volatile uint32_t preinitialized_instructions[] = {
    0x04200513, // li  a0, 0x42
    0x00008067, // ret
};

volatile uint32_t dynamic_instructions[NUM_TEST_INSTRUCTIONS] = { 0 };

int main(int argc, char** argv) {
    // Invoke preinitialized version of function
    uint64_t (*preinitialized_instructions_func)() = (void*)&preinitialized_instructions;

    uint64_t preinitialized_result = preinitialized_instructions_func();

    bp_hprint_uint64(preinitialized_result);
    bp_cprint('\n');

    if (preinitialized_result != 0x42) {
        bp_finish(1);
    }

    // Copy preinitialized function into dynamic array
    for (int i = 0; i < NUM_TEST_INSTRUCTIONS; i++) {
        dynamic_instructions[i] = preinitialized_instructions[i];
    }

    asm volatile ("fence.i");

    // Invoke dynamic version of function
    uint64_t (*dynamic_instructions_func)() = (void*)&dynamic_instructions;

    uint64_t dynamic_result = dynamic_instructions_func();

    bp_hprint_uint64(dynamic_result);
    bp_cprint('\n');

    if (dynamic_result == 0x42) {
        bp_finish(0);
    } else {
        bp_finish(1);
    }

    return 0;
}
