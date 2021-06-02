/*
 * Name:
 *   mc_sanity.c
 *
 * Description:
 *   This program has each core write to unique entries in memory and then locally sum
 *   the values. The cores deliberately false share cache blocks, forcing the coherence
 *   system to transfer blocks between the cores as a basic sanity check of coherence
 *   functionality in a multicore system.
 *
 */

#include <stdint.h>
#include "bp_utils.h"
#include "mc_util.h"

#ifndef NUM_CORES
#define NUM_CORES 2
#endif

#define K 512

#ifndef N
#define N (NUM_CORES*K)
#endif

volatile uint64_t __attribute__((aligned(64))) global_lock = 0;
volatile uint64_t __attribute__((aligned(64))) start_barrier_mem = 0;
volatile uint64_t __attribute__((aligned(64))) end_barrier_mem = 0;

typedef uint64_t matrix[N];
matrix MATRIX;

uint64_t thread_main() {
  uint64_t core_id;
  __asm__ volatile("csrr %0, mhartid": "=r"(core_id): :);

  uint64_t i;
  uint64_t sum = 0;
  for (int i = 0; i < K; i++) {
    MATRIX[i*NUM_CORES + core_id] = 1;
    sum += MATRIX[i*NUM_CORES + core_id];
  }

  // synchronize at end of computation by incrementing the end barrier
  lock(&global_lock);
  end_barrier_mem += 1;
  unlock(&global_lock);

  return sum;
}

uint64_t main(uint64_t argc, char * argv[]) {
  uint64_t core_id;
  __asm__ volatile("csrr %0, mhartid": "=r"(core_id): :);

  // all threads execute
  uint64_t sum = thread_main();

  if (core_id == 0) {
    // core 0 waits for all threads to finish
    // wait for all threads to finish
    while (end_barrier_mem != NUM_CORES) { }
    if (sum != K) {
      return 1;
    }
    return 0;
  } else {
    if (sum != K) {
      bp_finish(1);
    } else {
      bp_finish(0);
    }
    while (1) { }
  }

  // no core should reach this, return non-zero (error)
  return 1;
}
