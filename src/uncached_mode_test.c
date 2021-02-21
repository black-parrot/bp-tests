#include <stdint.h>
#include <bp_utils.h>

// This test checks for uncached and cached accesses to the same address

uint64_t *cfg_reg_dcache_mode = 0x200043;
uint64_t *access_addr = 0x80000000;

void main(uint64_t argc, char * argv[]) {
  int value = 0;

  // Start in uncached mode
  *cfg_reg_dcache_mode = 0;
  *access_addr = 0x80;
  
  // Check for the value in cached mode
  *cfg_reg_dcache_mode = 1;
  value = *access_addr;
  if (value != 0x80)
    bp_finish(1);

  // Store new value in cached mode and load it
  *access_addr = 0x70;
  value = *access_addr;
  if (value != 0x70)
    bp_finish(1);

  // Store another value in uncached mode
  *cfg_reg_dcache_mode = 0;
  *access_addr = 0x90;

  // Load the new value in cached mode
  *cfg_reg_dcache_mode = 1;
  value = *access_addr;
  if (value != 0x90)
    bp_finish(1);

  // Store a new value in cached mode
  // and load it in uncached mode
  *access_addr = 0x60;
  *cfg_reg_dcache_mode = 0;
  value = *access_addr;
  if (value != 0x60)
    bp_finish(1);

  bp_finish(0);
}
