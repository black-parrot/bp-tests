/*
 * Name:
 *   mc_util.h
 *
 * Description:
 *   This header defines some useful functions for multicore tests.
 *
 */

#ifndef MC_UTIL_H
#define MC_UTIL_H

// Random Number Generator
// Source: stackoverflow.com/questions/7602919/how-do-i-generate-random-numbers-without-rand-function
uint16_t mc_rand() {
  static uint16_t lfsr = 0xACE1;
  uint16_t bit = (lfsr ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1;
  lfsr = (lfsr >> 1) | (bit << 15);
  return lfsr;
}

// Random bit generator
// Source: https://www.schneier.com/academic/archives/1994/09/pseudo-random_sequen.html
// Example 2. VERYRANDOM
uint32_t mc_rand_bit() {
  // these should be initialized with a random value
  // here, we give them arbitrary values, but we could change the initialization
  static uint32_t regA = 0x12345678;
  static uint32_t regB = 0xfe8c20a8;
  static uint32_t regC = 0x357cc1dd;
  regA = ((((regA>>31)^(regA>>6)^(regA>>4)^(regA>>2)^(regA<<1)^regA)
         & 0x00000001)<<31) | (regA>>1);
  regB = ((((regB>>30)^(regB>>2)) & 0x00000001)<<30) | (regB>>1);
  regC = ((((regC>>28)^(regC>>1)) & 0x00000001)<<28) | (regC>>1);
  /*regB is a 31-bit LFSR.  regC is a 29-bit LFSR.*/
  /*Both feedback sequences are chosen to be maximum length.*/
  return ((regA & regB) | (!regA & regC)) & 0x00000001;
}


// Lock and unlock routines
void lock(volatile uint64_t *addr) {
    uint64_t swap_value = 1;

    do {
        __asm__ __volatile__ ("amoswap.d %0, %2, (%1)": "=r"(swap_value) 
                                                 : "r"(addr), "r"(swap_value)
                                                 :);
    } while (swap_value != 0);
}

void unlock(volatile uint64_t *addr) {
    uint64_t swap_value = 0;
    __asm__ __volatile__ ("amoswap.d %0, %2, (%1)": "=r"(swap_value) 
                                                 : "r"(addr), "r"(swap_value)
                                                 :);
}

void sync_barrier(volatile uint64_t *barrier_addr) {
    __asm__ __volatile__ ("amoadd.d x0, %1, (%0)": : "r"(barrier_addr), "r"(1));
    uint32_t num_cores = bp_param_get(PARAM_CC_X_DIM) * bp_param_get(PARAM_CC_Y_DIM);
    while (*barrier_addr != num_cores) { };
    uint64_t core_id;
    __asm__ __volatile__ ("csrr %0, mhartid": "=r"(core_id): :);
    //bp_cprint('b');
    //bp_cprint('a');
    //bp_cprint('r');
    //bp_cprint(' ');
    //bp_hprint(core_id);
    //bp_cprint('-');
    //bp_hprint(num_cores);
    //bp_cprint('\n');
    if (core_id == 0) { *barrier_addr = 0; }
}

#endif
