#include <stdint.h>
#include <bp_utils.h>

// This test checks for uncached and cached accesses to the same address

uint64_t *cfg_reg_dcache_mode = 0x200404;
uint64_t *access_addr = 0x80000000;
uint64_t *check_addr  = 0x80001060;
uint64_t *check_addr2 = 0x80002060;
uint64_t *check_addr3 = 0x80003060;
uint64_t *check_addr4 = 0x80004060;
uint64_t *check_addr5 = 0x80005060;
uint64_t *check_addr6 = 0x80006060;
uint64_t *check_addr7 = 0x80007060;
uint64_t *check_addr8 = 0x80008060;
uint64_t *check_addr9 = 0x80009060;
uint64_t *uncached_addr = 0x40;

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
  *check_addr = 0x40;
  *check_addr2 = 0x40;
  *check_addr3 = 0x40;
  *check_addr4 = 0x40;
  *check_addr5 = 0x40;
  *check_addr6 = 0x40;
  *check_addr7 = 0x40;
  *check_addr8 = 0x40;
  *cfg_reg_dcache_mode = 0;
  value = *access_addr;
  if (value != 0x60)
    bp_finish(1);

  // Do an uncached load to evict it from the cache
  value = *check_addr;
  *cfg_reg_dcache_mode = 1;
  *access_addr = 0x50;
  *check_addr9 = 0x40;

  // Get the load address ready for back to back UC store and load requests
  __asm__ __volatile__ ("addi a3,gp,-2032");
  __asm__ __volatile__ ("ld a3,0(a3)");
  *uncached_addr = 0x50;
  __asm__ __volatile__ ("ld a4,0(a3)");

  // Load all the different values
  if (value != 0x40)
    bp_finish(1);
  value = *access_addr;
  if (value != 0x50)
    bp_finish(1);

  value = *check_addr2;
  if (value != 0x40)
    bp_finish(1);

  value = *check_addr3;
  if (value != 0x40)
    bp_finish(1);

  value = *check_addr4;
  if (value != 0x40)
    bp_finish(1);

  value = *check_addr5;
  if (value != 0x40)
    bp_finish(1);

  value = *check_addr6;
  if (value != 0x40)
    bp_finish(1);

  value = *check_addr7;
  if (value != 0x40)
    bp_finish(1);

  value = *check_addr8;
  if (value != 0x40)
    bp_finish(1);

  value = *check_addr9;
  if (value != 0x40)
    bp_finish(1);

  bp_finish(0);
}
