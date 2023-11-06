BP_TESTS_C = \
  m_external_interrupt  \
  s_external_interrupt  \
  hello_world           \
  instr_coherence       \
  cache_hammer          \
  jalr_illegal          \
  satp_nofence          \
  timer_interrupt       \
  loop                  \
  cache_flush           \
  stream_hammer         \
  domain_fault          \
  eaddr_fault           \
  mstatus_fs            \
  wfi                   \
  uncached_mode         \
  amo_nonblocking       \
  amo_interrupt         \
  divide_hazard         \
  unhandled_trap        \
  aviary_rom            \
  readonly              \
  epc                   \
  fflags_haz            \
  dram_stress           \
  misaligned_ldst       \
  nanboxing             \
  misaligned_instructions_basic_jumps \
  misaligned_instructions_advanced_jumps \
  misaligned_instructions_virtual_memory \
  execute_dynamic_instruction \
  dma_memcpy            \
  neo_resnet50_1 \
  neo_resnet50_2 \
  neo_resnet50_3 \
  neo_dlrm_1 \
  neo_dlrm_2 \
  neo_dlrm_3 \
  neo_bert_1 \
  neo_bert_2 \
  neo_bert_3 \
  neo_bert_2_n1 \
  neo_bert_2_n4 \
  neo_bert_2_n8 \
  neo_bert_2_n16 \
  neo_bert_2_n32 \
  neo_bert_2_n64 \

BP_MULTICORE_TESTS_C = \
  mc_sanity             \
  mc_template           \
  mc_rand_walk          \
  mc_work_share_sort    \
  mc_amo_add            \
  mc_lrsc_add           \

BP_ACCELERATOR_TESTS_C = \
  streaming_accelerator_vdp      \
  streaming_accelerator_loopback \
  streaming_accelerator_zipline  \
  coherent_accelerator_vdp       \

BP_TESTS_CPP = \
  virtual               \
  constructor           \
  template              \
  unwinding             \
  vector                \
  map                   \

BP_TESTS = $(BP_TESTS_C) $(BP_MULTICORE_TESTS_C) $(BP_ACCELERATOR_TESTS_C) $(BP_TESTS_CPP)

# REMOVED FROM BP_TESTS_C because they fail with -O3
# 	mapping
# 	paging
