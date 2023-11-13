#include <stdint.h>
#include <bp_utils.h>

// This test checks for uncached and cached accesses to the same address

void main(uint64_t argc, char * argv[]) {
  int value = 0;
  uint64_t val = 0;

  volatile uint64_t access_addr;
  volatile uint64_t check_addr;
  volatile uint64_t check_addr2;
  volatile uint64_t check_addr3;
  volatile uint64_t check_addr4;
  volatile uint64_t check_addr5;
  volatile uint64_t check_addr6;
  volatile uint64_t check_addr7;
  volatile uint64_t check_addr8;
  volatile uint64_t check_addr9;

  volatile uint64_t *cfg_reg_dcache_mode = (volatile uint64_t *) 0x200408;
  val = 0; __asm__ __volatile__ ("sd %1, 0(%0)": : "r" (cfg_reg_dcache_mode), "r" (val));

  access_addr = 0x80;
  
  // Check for the value in cached mode
  val = 1; __asm__ __volatile__ ("sd %1, 0(%0)": : "r" (cfg_reg_dcache_mode), "r" (val));
  value = access_addr;
  if (value != 0x80)
    bp_finish(1);

  // Store new value in cached mode and load it
  access_addr = 0x70;
  value = access_addr;
  if (value != 0x70)
    bp_finish(1);

  // Store another value in uncached mode
  val = 0; __asm__ __volatile__ ("sd %1, 0(%0)": : "r" (cfg_reg_dcache_mode), "r" (val));
  access_addr = 0x90;

  // Load the new value in cached mode
  val = 1; __asm__ __volatile__ ("sd %1, 0(%0)": : "r" (cfg_reg_dcache_mode), "r" (val));
  value = access_addr;
  if (value != 0x90)
    bp_finish(1);

  // Store a new value in cached mode
  // and load it in uncached mode
  access_addr = 0x60;
  check_addr  = 0x40;
  check_addr2 = 0x40;
  check_addr3 = 0x40;
  check_addr4 = 0x40;
  check_addr5 = 0x40;
  check_addr6 = 0x40;
  check_addr7 = 0x40;
  check_addr8 = 0x40;
  val = 0; __asm__ __volatile__ ("sd %1, 0(%0)": : "r" (cfg_reg_dcache_mode), "r" (val));
  value = access_addr;
  if (value != 0x60)
    bp_finish(1);

  // Do an uncached load to evict it from the cache
  value = check_addr;
  val = 1; __asm__ __volatile__ ("sd %1, 0(%0)": : "r" (cfg_reg_dcache_mode), "r" (val));
  access_addr = 0x50;
  check_addr9 = 0x40;

  // Load all the different values
  if (value != 0x40)
    bp_finish(1);
  value = access_addr;
  if (value != 0x50)
    bp_finish(1);

  value = check_addr2;
  if (value != 0x40)
    bp_finish(1);

  value = check_addr3;
  if (value != 0x40)
    bp_finish(1);

  value = check_addr4;
  if (value != 0x40)
  if (value != 0x40)
    bp_finish(1);

  if (value != 0x40)
  value = check_addr5;
  if (value != 0x40)
    bp_finish(1);

  value = check_addr6;
  if (value != 0x40)
    bp_finish(1);

  value = check_addr7;
  if (value != 0x40)
    bp_finish(1);

  value = check_addr8;
  if (value != 0x40)
    bp_finish(1);

  value = check_addr9;
  if (value != 0x40)
    bp_finish(1);

  bp_finish(0);
}

