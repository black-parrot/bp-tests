BP_TESTS_C = \
  hello_world           \
  bubblesort_demo       \
  streaming_accelerator_vdp\
  streaming_accelerator_loopback\
  accelerator_vdp_multicore_4\
  accelerator_loopback_multicore_4\
  streaming_accelerator_zipline\
  coherent_accelerator_vdp\
  copy_example          \
  mc_sanity             \
  mc_template           \
  mc_rand_walk          \
  mc_work_share_sort    \
  cache_hammer          \
  jalr_illegal          \
  satp_nofence          \
  timer_interrupt_test  \
  loop_test             \
  cache_flush           \
  stream_hammer         \
  domain_fault          \
  eaddr_fault           \
  paging                \
  mapping               \
  mstatus_fs            \
  wfi_test              \
  uncached_mode_test    \
  amo_nonblocking       \
  amo_interrupt         \
  divide_hazard         \
  illegal_instruction   \
  unhandled_trap        \
  aviary_rom_test       \
  readonly

BP_TESTS_CPP = \
  virtual               \
  constructor           \
  template              \
  unwinding             \
  vector                \
  map

BP_TESTS = $(BP_TESTS_C) $(BP_TESTS_CPP)

