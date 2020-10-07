// This test is used to stress the BP Stream Protocol design implemented between UCE and L2,
// Aimming to do frequent read/write with L1 cache miss and L2 hit.
// To do so, the  difference between L1 and L2 is utilized

#include <stdint.h>
#include "bp_utils.h"

int main(int argc, char** argv) {
    // parameter define
    uint64_t l1_sets  = 64;
    uint64_t l1_assoc = 8;

    uint64_t l2s_sets = 128;
    uint64_t l2s_assoc = 8;
    
    
    uint64_t paddr_bits = 40;

    uint64_t offset_bits = 6;
    uint64_t index_bits = 7;
    uint64_t tag_bits = 27;

    uint64_t index_offset = offset_bits;
    uint64_t tag_offset = index_bits + offset_bits;

    uint64_t core_id = bp_get_hart();
    uint64_t *base_addr = (uint64_t *)(0x80100000 + (core_id << 14));

    // write the first 64-bit data of a block locating at incrementing indices and tags.
    // i iterates on indices
    // j iterates on tags
    for (int i = 0; i < l1_sets; i++) {
        for (int j = 0, j < l1_assoc, j++) {
            uint64_t *addr = base_addr + (i << index_offset) + (j << tag_offset); // address of same index (0) and different tag (0-100)
            uint64_t data = (uint64_t) addr | 0x0fff;

            *addr = data;
        }
    }

    // check the writeback result
    // i iterates on indices
    // j iterates on tags, given we only want those 
    for (int i = 0; i < l1_sets; i++) {
        fot (int j = 0, j < l1_assoc; j++) {
            uint64_t *addr = base_addr + (i << index_offset) + (j << tag_offset);
            uint64_t expected = (uint64_t) addr | 0x0fff;
            uint64_t actual = *addr;

            if (expected != actual) {
              bp_finish(-1);
            }
        }
        
    }
    bp_finish(0);
    
    return 0;
}
