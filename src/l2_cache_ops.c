
#include "bp_utils.h"
#include "bp_l2_cache.h"

uint64_t data;

int main(int argc, char** argv) {
    uintptr_t addr = ((uintptr_t)&data);

    l2cache_afl(addr);
    l2cache_aflinv(addr);
    l2cache_ainv(addr);
    l2cache_alock(addr);
    l2cache_aunlock(addr);
    l2cache_tagfl(0, 1, 2, 3, 4);
    l2cache_taglv(0, 1, 2, 3, 4);
    l2cache_tagla(0, 1, 2, 3, 4);
    l2cache_tagst(0, 1, 2, 3, 4, 0xdead);

    return 0;
}

