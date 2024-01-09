
#include <stdint.h>
#include <bp_utils.h>

// This test checks for uncached and cached accesses to the same address

void main(uint64_t argc, char * argv[]) {
  volatile uint64_t value;
  // Check uncached DRAM space

  uint64_t *l1_access_addr = (uint64_t *) (0x083000000);
  uint64_t *l2_access_addr = (uint64_t *) (0x183000000);

  // Test uncached L1 access
  // L1 = 0xdead
  // L2 = 0xdead
  *l2_access_addr = 0xdead;
  value = *l2_access_addr;
  bp_print_string("L2 Access: ");
  bp_hprint_uint64(value);
  bp_cprint('\n');
  if (value != 0xdead)
    bp_finish(1);
  bp_print_string("L1 Access: ");
  value = *l1_access_addr;
  bp_hprint_uint64(value);
  bp_cprint('\n');
  if (value != 0xdead)
    bp_finish(1);

  // Test L1 bypass
  // L1 = 0xdead
  // L2 = 0xbeef
  *l1_access_addr = 0xdead;
  *l2_access_addr = 0xbeef;
  value = *l1_access_addr;
  if (value != 0xdead)
    bp_finish(1);
  value = *l2_access_addr;
  if (value != 0xbeef)
    bp_finish(1);

  bp_finish(0);
}
