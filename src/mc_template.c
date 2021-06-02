/*
 * Name:
 *   mc_template.c
 *
 * Description:
 *   This is a template program for multicore tests that run "baremetal" on BlackParrot.
 *
 *   This mimics a set of threads running, but does not use an actual threading
 *   model. It is assumed that threads map to cores directly, e.g., thread 0
 *   executes on core 0, thread 1 on core 1, etc.
 *
 *   Core 0 is the controlling core. At the end of execution, all cores synchronize
 *   using a barrier. Core 0 waits for all cores to synchronize then returns from
 *   main. All other cores call bp_finish() with argument of 0 as PASS, 1 as FAIL
 *   and then enter a busy-wait loop. Only core 0 calls return to avoid races between
 *   multiple cores calling return.
 *
 */

#include <stdint.h>
#include "bp_utils.h"
#include "mc_util.h"

#ifndef N
#define N 16
#endif

typedef uint32_t matrix[N];
matrix MATRIX;

#ifndef NUM_CORES
#define NUM_CORES 2
#endif

volatile uint64_t __attribute__((aligned(64))) global_lock = 0;
volatile uint64_t __attribute__((aligned(64))) start_barrier_mem = 0;
volatile uint64_t __attribute__((aligned(64))) end_barrier_mem = 0;

uint64_t thread_main() {
  uint64_t core_id;
  __asm__ volatile("csrr %0, mhartid": "=r"(core_id): :);

  // TODO: make the thread do something interesting

  // synchronize at end of computation by incrementing the end barrier
  lock(&global_lock);
  end_barrier_mem += 1;
  unlock(&global_lock);

}

uint64_t main(uint64_t argc, char * argv[]) {
  uint64_t core_id;
  __asm__ volatile("csrr %0, mhartid": "=r"(core_id): :);

  // only core 0 intializes data structures
  if (core_id == 0) {
    global_lock = 0;

    for (int i = 0; i < N; i++) {
      MATRIX[i] = i;
    }

    // signal done with initialization
    start_barrier_mem = 0xdeadbeef;
  }
  else {
    while (start_barrier_mem != 0xdeadbeef) { }
  }

  // all threads execute
  thread_main();

  if (core_id == 0) {
    // core 0 waits for all threads to finish
    // wait for all threads to finish
    while (end_barrier_mem != NUM_CORES) { }
    return 0;
  } else {
    // all other cores call finish (0 = pass, 1 = fail) ...
    bp_finish(0);
    // ... then busy-wait for core 0 to terminate the execution
    while (1) { }
  }

  // no core should reach this, return non-zero (error)
  return 1;
}

