
#include <stdint.h>
#include "bp_utils.h"

uint32_t preinitialized_instructions[] = {
    0x04200513, // li  a0, 0x42
    0x00008067, // ret
};

int main(int argc, char** argv) {
    uint64_t (*func)() = &preconfigured_instructions;

    uint64_t result = func();

    bp_hprint_uint64(result);
    bp_cprint("\n");

    if (result == 0x42) {
        bp_finish(0);
    } else {
        bp_finish(1);
    }

    return 0;
}
