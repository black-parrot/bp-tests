
#include <stdint.h>

int main() {

    uint64_t data_base = 0x81000000;
    uint64_t block_shift  = 6;

    for (int i = 0; i < 10000; i++) {
        *(uint64_t*)(data_base + (i << block_shift)) = i;
    }

    for (int i = 0; i < 10000; i++) {
        uint64_t x = *(uint64_t*)(data_base + (i << block_shift));
        if(x != i)
          return -1;
    }
    return 0;
}
