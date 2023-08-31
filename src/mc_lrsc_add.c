
#include <stdint.h>
#include <bp_utils.h>
#include "mc_util.h"

// number of iterations for each core to run the main loop
#define N 500

volatile uint64_t barrier_mem __attribute__((aligned(64))) = 0;
volatile uint64_t amo_target __attribute__ ((aligned (64))) = 0;

uint64_t main(uint64_t argc, char * argv[]) {

  int iters = N;
  uint64_t sc_result = 0;
  uint64_t sc_load = 0;
  int i = 0;

  sync_barrier(&barrier_mem);

  // main loop
  // Execute LR/SC that performs amo_add.d of 1 to amo_target
  __asm__ __volatile__ (
    "j .Lcheck%=;"
    ".Llrsc%=:;"
    "lr.d %[sc_load], (%[amo_addr]);" // load to sc_load
    "addi %[sc_result],%[sc_load],1;" // sc_result = sc_load + 1
    "sc.d %[sc_result],%[sc_result],(%[amo_addr]);" // sc_result = store sc_result to amo_target_addr
    "bnez %[sc_result], .Llrsc%=;" // bnez sc_result
    ".Lcheck%=:;"
    "addi %[i], %[i], 1;" // i = i + 1
    "bge %[iters], %[i], .Llrsc%=;" // bge N, i
    : [sc_result] "+r" (sc_result), [sc_load] "+r" (sc_load), [i] "+r" (i)
    : [amo_addr] "r" (&amo_target), [iters] "r" (iters)
    :
    );

  // all cores synchronize at end
  sync_barrier(&barrier_mem);

  uint32_t num_cores = bp_param_get(PARAM_CC_X_DIM) * bp_param_get(PARAM_CC_Y_DIM);
  uint64_t total = num_cores * N;
  return (amo_target != total);
}
