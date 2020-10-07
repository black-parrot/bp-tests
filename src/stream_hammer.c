// This test is used to stress the BP Stream Protocol design implemented between UCE and L2,
// Aimming to do frequent read/write with L1 cache miss and L2 hit.
// Given l2_sets = 2 * l1_sets, we have the address format
//       tag  |  index  |  block_offset
// L1:   28      6         6 
// L2:   27      7         6
// which indicates that the first half and the second half of L2 cache maps to the same index in L1 cache 
// We firstly write data to each block with indices from 0 to 127, using 8 different tags.
// In this way, the cache block with indices from 0 to 63 will be written back to L2 cache to hold blocks with indices from 64 to 127
// so we canhave L1 miss and L2 hit for indices from 0 to 63.
// Then, we read the cache block from 0 to 63 to check the written data.
#include <stdint.h>
#include "bp_utils.h"

int main(int argc, char** argv) {
    // parameter define
    uint64_t assoc = 8;
    uint64_t l1_sets  = 64;
    uint64_t l2_sets = 128;
    
    uint64_t paddr_bits = 40;
    uint64_t block_offset_bits = 6;

    uint64_t l1_index_bits = 6;
    uint64_t l1_tag_bits = 28;

    uint64_t l2_index_bits = 7;
    uint64_t l2_tag_bits = 27;

    uint64_t index_offset = block_offset_bits;
    uint64_t l1_tag_offset = l1_index_bits + block_offset_bits;
    uint64_t l2_tag_offset = l2_index_bits + block_offset_bits;

    uint64_t core_id = bp_get_hart();
    uint64_t *base_addr = (uint64_t *)(0x80100000 + (core_id << 14));

    // write the first 64-bit data of a block locating at incrementing indices and tags.
    // i iterates on indices
    // j iterates on tags
    for (int i = 0; i < l2_sets; i++) {
        for (int j = 0; j < assoc; j++) {
            uint64_t *addr = base_addr + (i << index_offset) + (j << l2_tag_offset); // address of same index (0) and different tag (0-100)
            uint64_t data = (uint64_t) addr | 0x0fff;

            *addr = data;
        }
    }

    // check the writeback result
    for (int i = 0; i < l1_sets; i++) {
        for (int j = 0; j < assoc; j++) {
            uint64_t *addr = base_addr + (i << index_offset) + (j << l2_tag_offset);
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
