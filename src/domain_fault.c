#include <stdint.h>
#include <bp_utils.h>

/*
 * This test checks if the domain fault checking is working correctly
 * The domain register is an 8-bit register and a domain is enabled if the corresponding bit is set as 1
 * For example: domain = 00001011 indicates domains 0, 1 and 3 are enabled
 */

uint64_t *cfg_domain_reg_addr = 0x200009;
uint64_t *domain_4 = 0x8000000000;
uint64_t *domain_7 = 0xe000000000;

void trap_success(){
  uint64_t mcause;
  __asm__ __volatile__ ("csrr %0, mcause": "=r"(mcause));

  // Trap should be on store access fault
  if(mcause == 7)
    bp_finish(0);

  bp_finish(1);
}

void main(uint64_t argc, char * argv[]) {
  uint64_t domain_data;
  
  // Use the alternate handler for trapping
  __asm__ __volatile__ ("csrw mtvec, %0": : "r"(&trap_success));  

  // Checking if the default enabled domain is 0;
  domain_data = *cfg_domain_reg_addr;
  if(domain_data != 1)
    bp_finish(1);
   
  // Disabling domain 0
  *cfg_domain_reg_addr = 0;

  domain_data = *cfg_domain_reg_addr;
  // Domain 0 must still be active
  if(domain_data != 1)
    bp_finish(1);

  // Enabling domain 4
  *cfg_domain_reg_addr = 16;

  // Accessing domain 4. This must print a warning on the screen in simulation
  *domain_4 = 0xdeadbeef;

  // Accessing domain 7. This should trap
  *domain_7 = 0xdeadbeef; 

  // Test fails if previous instruction didn't trap
  bp_finish(1);
}


