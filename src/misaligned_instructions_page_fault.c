#include <string.h>
#include "bp_utils.h"
#include "vm.h"

volatile uint32_t *cfg_reg_icache_mode = (volatile uint32_t *) 0x200204;

#define NPT 4
#define l1pt pt[0]
#define l2pt pt[1]
#define l3pt pt[2]
// TODO: remove
#define kpt  pt[3]

// Address of start of the Nth "data page", an arbitrary range of 4k pages for testing use
// Intentionally _not_ the same VA as PA -- we apply an arbitrary 127-megapage offset.
#define DATA_PAGE_VADDR(data_page_num) ((128 * MPGSIZE) + DRAM_BASE + ((data_page_num) * PGSIZE))
#define DATA_PAGE_VADDR_TO_PADDR(addr) ((addr) - (128 * MPGSIZE) + MPGSIZE)

#define TEST_PAGE_NUM_FOR_TEST_NUM(test_num) ((test_num)*2)
// The address of the start of a fresh "data page", which can be used along with the one before it for this test
#define TEST_BOUNDARY_VADDR(test_num) (DATA_PAGE_VADDR(TEST_PAGE_NUM_FOR_TEST_NUM(test_num) + 1))

#define UMODE_TO_SMODE_CODE_OFFSET MPGSIZE

#define PAGE_PERMS_USER_ALL PTE_U_LEAF
#define PAGE_PERMS_SUPERVISOR_ALL PTE_S_LEAF
#define PAGE_PERMS_USER_NOEXEC 0
//(PAGE_PERMS_USER_ALL & ~(uint64_t)PTE_X & ~(uint64_t)PTE_R & ~(uint64_t)PTE_W & ~(uint64_t)PTE_V)

#define FAULT_MAGIC 0x8BADF00D

struct fault_info_t {
  uint64_t pc;
  uint64_t tval;
  uint64_t cause;
  bool present;
} latest_fault_info;
bool test_active = false;

void trap_entry();
void pop_tf(trapframe_t*);

typedef uint64_t (*test_gadget_t)();

// Sanity check (fully aligned)
static const uint64_t test_0_aligned_execution_across_page_boundary_gadget_address = TEST_BOUNDARY_VADDR(0) - 4;
// Misaligned but entirely within a single page (confirm that misaligned execution with VM enabled works at all)
static const uint64_t test_1_misaligned_within_single_page_gadget_address = TEST_BOUNDARY_VADDR(1) - 14;
// Misaligned instruction sequence with middle of three instructions spanning page boundary
static const uint64_t test_2_misaligned_execution_across_page_boundary_gadget_address = TEST_BOUNDARY_VADDR(2) - 6;
// Same as above, but now the first instruction is misaligned so it crosses the page boundary -- i.e., both halves miss
static const uint64_t test_3_tlb_miss_both_halves_gadget_address = TEST_BOUNDARY_VADDR(3) - 2;
// Executing misaligned instruction spanning page boundary, but second half is in ITLB and I$ while first isn't
static const uint64_t test_4_tlb_miss_first_half_only_primary_gadget_address = TEST_BOUNDARY_VADDR(4) - 6;
static const uint64_t test_4_tlb_miss_first_half_only_secondary_gadget_address = TEST_BOUNDARY_VADDR(4) + 16;
// Misaligned, entirely within a single page, and page is marked as non-executable.
static const uint64_t test_5_access_fault_within_single_page = TEST_BOUNDARY_VADDR(5) - 14;

uint64_t pt[NPT][PTES_PER_PT] __attribute__((aligned(PGSIZE))) = {0};

uint64_t userspace_trap_return_trampoline() {
  return FAULT_MAGIC;
}

static void map_cfg_page() {
  // The lowest virtual gigapage is mapped 1:1 to physical addresses for CFG registers
  // Only the first two megapages are required, but gigapage requires fewer page tables

    uint64_t aligned_va = 0;
    uint64_t aligned_pa = 0;

    bp_hprint_uint64(vpn2(aligned_va));
    bp_print_string(" cfg\n");
    l1pt[vpn2(aligned_va)] = ((uint64_t)aligned_pa >> PGSHIFT << PTE_PPN_SHIFT) | PAGE_PERMS_USER_ALL;;
}

static void map_code_page() {
  // Megapage starting at DRAM_BASE is mapped 1:1 to physical addresses so that we don't have to
  // handle a relocation

    uint64_t aligned_va = DRAM_BASE;
    uint64_t aligned_pa = DRAM_BASE;

    bp_hprint_uint64(vpn2(aligned_va));
    bp_print_string(" code\n");
    l1pt[vpn2(aligned_va)] = ((uint64_t)l2pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
    l2pt[vpn1(aligned_va)] = ((uint64_t)aligned_pa >> PGSHIFT << PTE_PPN_SHIFT) | PAGE_PERMS_USER_ALL;
    // Trap handler executes in S-mode which isn't allowed to execute instructions from U-mode pages.
    // Duplicate the code/data page for S-mode to use purely for trap handler code.
    l2pt[vpn1(aligned_va+UMODE_TO_SMODE_CODE_OFFSET)] = ((uint64_t)aligned_pa >> PGSHIFT << PTE_PPN_SHIFT) | PAGE_PERMS_SUPERVISOR_ALL;
}


static void map_test_page(uint64_t test_page_num, uint64_t leaf_perm) {
    uint64_t aligned_va = DATA_PAGE_VADDR(test_page_num);
    uint64_t aligned_pa = DATA_PAGE_VADDR_TO_PADDR(aligned_va);

    bp_hprint_uint64(vpn2(aligned_va));
    bp_print_string(" test\n");
    l1pt[vpn2(aligned_va)] = ((uint64_t)l2pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
    l2pt[vpn1(aligned_va)] = ((uint64_t)l3pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
    l3pt[vpn0(aligned_va)] = ((uint64_t)aligned_pa >> PGSHIFT << PTE_PPN_SHIFT) | leaf_perm;
}

void handle_fault(trapframe_t* tf) {
  if (!test_active) {
    bp_print_string("Fault occurred while no test active, aborting...\n");
    terminate(-1);
    while (1);
  }

  if (latest_fault_info.present) {
    bp_print_string("Second fault occurred during test, aborting...\n");
    terminate(-1);
    while (1);
  }

  latest_fault_info.pc = tf->epc;
  latest_fault_info.tval = tf->badvaddr;
  latest_fault_info.cause = tf->cause;
  latest_fault_info.present = true;

  // TODO: make offsets clearer
  tf->epc = (uint64_t)&userspace_trap_return_trampoline - UMODE_TO_SMODE_CODE_OFFSET;
}

void handle_trap(trapframe_t* tf) {
  if (tf->cause == CAUSE_FETCH_PAGE_FAULT || tf->cause == CAUSE_FETCH_ACCESS)
    handle_fault(tf);
  else
    terminate(-1);

  pop_tf(tf);
}

void init_vm() {
  // enable memory mappings
  uint64_t satp_val = ((uint64_t)l1pt >> PGSHIFT) | ((uint64_t)SATP_MODE_SV39 * (SATP_MODE & ~(SATP_MODE<<1)));
  write_csr(satp, satp_val);
  // Allow machine and supervisor modes to read/write user memory
  set_csr(mstatus, MSTATUS_SUM);
  set_csr(sstatus, SSTATUS_SUM);
  write_csr(stvec, trap_entry+UMODE_TO_SMODE_CODE_OFFSET);
  write_csr(sscratch, read_csr(mscratch));
  // Exceptions we don't care about can be routed to the default machine-mode
  // handler so they are pretty-printed and trigger an abort.
  write_csr(medeleg,
    (1 << CAUSE_FETCH_PAGE_FAULT) |
    (1 << CAUSE_FETCH_ACCESS));

  // TODO: temp, try all the fences because things don't work
  asm volatile ("sfence.vma");
  asm volatile ("fence");

  // fence to ensure the instruction writes take effect (unrelated to SATP)
  asm volatile ("fence.i");
}

uint64_t execute_test_gadget(uint64_t gadget_address) {
  latest_fault_info = (const struct fault_info_t){ 0 };
  test_active = false;

  test_gadget_t gadget_fn = (test_gadget_t)gadget_address;

  bp_print_string("jumping to: ");
  bp_hprint_uint64((uint64_t)gadget_fn);
  bp_cprint('\n');

  test_active = true;
  asm volatile ("li a0, 0");
  uint64_t result = gadget_fn();
  test_active = false;

  return result;
}

void execute_and_expect_fault(uint64_t gadget_address, uint64_t expected_pc, uint64_t expected_cause, uint64_t expected_val) {
  uint64_t result = execute_test_gadget(gadget_address);

  // verify that fault was triggered -- trap handler returns magic value to
  // indicate it triggered a trap
  if (result != FAULT_MAGIC) {
    bp_print_string("result: ");
    bp_hprint_uint64(result);
    bp_print_string(" (fail)\n");
    terminate(-1);
  }

  if (!latest_fault_info.present) {
    // In theory, if magic return value check passes we must have seen a trap.
    // Confirm here that data was passed back to usermode correctly.
    bp_print_string("expected fault, but none was logged\n");
    terminate(-1);
  }

  if (latest_fault_info.pc != expected_pc || latest_fault_info.tval != expected_val || latest_fault_info.cause != expected_cause) {
    bp_print_string("Incorrect fault triggered!\n");

    bp_print_string("scause:\n");
    bp_print_string("\texpected ");
    bp_hprint_uint64(expected_cause);
    bp_cprint('\n');
    bp_print_string("\tgot      ");
    bp_hprint_uint64(latest_fault_info.cause);
    bp_cprint('\n');


    bp_print_string("stval:\n");
    bp_print_string("\texpected ");
    bp_hprint_uint64(expected_val);
    bp_cprint('\n');
    bp_print_string("\tgot      ");
    bp_hprint_uint64(latest_fault_info.tval);
    bp_cprint('\n');

    bp_print_string("sepc:\n");
    bp_print_string("\texpected ");
    bp_hprint_uint64(expected_pc);
    bp_cprint('\n');
    bp_print_string("\tgot      ");
    bp_hprint_uint64(latest_fault_info.pc);
    bp_cprint('\n');

    terminate(-1);
  }

  bp_print_string("fault pc: ");
  bp_hprint_uint64(latest_fault_info.pc);
  bp_print_string(" (pass)\n");
}

void execute_and_expect_success(uint64_t gadget_address) {
  uint64_t result = execute_test_gadget(gadget_address);

  bp_print_string("result: ");
  bp_hprint_uint64(result);

  // "end" instruction sequence returns 0x42
  if (result == 0x42) {
    bp_print_string(" (pass)\n");
  } else {
    bp_print_string(" (fail)\n");
    terminate(-1);
  }

  if (latest_fault_info.present) {
    bp_print_string("unexpected fault:\n\tepc: ");
    bp_hprint_uint64(latest_fault_info.pc);
    bp_print_string("\n\tcause: ");
    bp_hprint_uint64(latest_fault_info.cause);
    bp_cprint('\n');
    terminate(-1);
  }
}

void run_test() {
  // TODO: yet another mindless fencei 
  asm volatile ("fence.i");

  execute_and_expect_success(test_0_aligned_execution_across_page_boundary_gadget_address);
  execute_and_expect_success(test_1_misaligned_within_single_page_gadget_address);
  execute_and_expect_success(test_2_misaligned_execution_across_page_boundary_gadget_address);
  execute_and_expect_success(test_3_tlb_miss_both_halves_gadget_address);
  // Execute test 4 "secondary" gadget first to prime ITLB and I$
  execute_and_expect_success(test_4_tlb_miss_first_half_only_secondary_gadget_address);
  execute_and_expect_success(test_4_tlb_miss_first_half_only_primary_gadget_address);

  execute_and_expect_fault(test_5_access_fault_within_single_page, test_5_access_fault_within_single_page, CAUSE_FETCH_ACCESS, test_5_access_fault_within_single_page);

  // TODO
  terminate(0); // temp
}


void map_test_pair(int test_num, uint64_t first_page_perms, uint64_t second_page_perms) {
  uint64_t first_page_num = TEST_PAGE_NUM_FOR_TEST_NUM(test_num);
  map_test_page(first_page_num, first_page_perms);
  map_test_page(first_page_num+1, second_page_perms);
}

void place_instruction(uint64_t vaddr, uint32_t instruction) {
  if (vaddr % 2 == 0) {
    // Decompose into aligned writes
    volatile void* target = ((volatile void*)DATA_PAGE_VADDR_TO_PADDR(vaddr));
    volatile uint16_t* target_lower = target;
    volatile uint16_t* target_upper = target + 2;

    *target_lower = instruction & 0x0000FFFF;
    *target_upper = instruction >> 16;
  } else {
    bp_panic("misaligned instruction");
  }
}

void place_dummy_instruction(uint64_t vaddr) {
  place_instruction(vaddr, 0x00000013); // nop
}

void place_end_instructions(uint64_t vaddr) {
  place_instruction(vaddr,   0x04200513); // li a0,0x42
  place_instruction(vaddr+4, 0x00008067); // ret
}


int main(int argc, char** argv) {

  map_code_page();
  map_cfg_page(); // TODO: might not use

  map_test_pair(0, PAGE_PERMS_USER_ALL, PAGE_PERMS_USER_ALL);
  place_dummy_instruction(test_0_aligned_execution_across_page_boundary_gadget_address);
  place_end_instructions(test_0_aligned_execution_across_page_boundary_gadget_address+4);

  map_test_pair(1, PAGE_PERMS_USER_ALL, PAGE_PERMS_USER_ALL);
  place_dummy_instruction(test_1_misaligned_within_single_page_gadget_address);
  place_end_instructions(test_1_misaligned_within_single_page_gadget_address+4);

  map_test_pair(2, PAGE_PERMS_USER_ALL, PAGE_PERMS_USER_ALL);
  place_dummy_instruction(test_2_misaligned_execution_across_page_boundary_gadget_address);
  place_end_instructions(test_2_misaligned_execution_across_page_boundary_gadget_address+4);

  map_test_pair(3, PAGE_PERMS_USER_ALL, PAGE_PERMS_USER_ALL);
  place_dummy_instruction(test_3_tlb_miss_both_halves_gadget_address);
  place_end_instructions(test_3_tlb_miss_both_halves_gadget_address+4);

  map_test_pair(4, PAGE_PERMS_USER_ALL, PAGE_PERMS_USER_ALL);
  place_dummy_instruction(test_4_tlb_miss_first_half_only_primary_gadget_address);
  place_end_instructions(test_4_tlb_miss_first_half_only_primary_gadget_address+4);
  place_end_instructions(test_4_tlb_miss_first_half_only_secondary_gadget_address);

  map_test_pair(5, PAGE_PERMS_USER_NOEXEC, PAGE_PERMS_USER_ALL);
  place_dummy_instruction(test_5_access_fault_within_single_page);
  place_end_instructions(test_5_access_fault_within_single_page+4);

  init_vm();

  // TODO: run tests twice, once default and once with nonspec I$
  flush_tlb();
  *cfg_reg_icache_mode = 2;

  // x TLB miss both halves
  // x TLB miss second half only
  // x TLB miss first half only
  // miss and fault on both halves
  // miss and fault on first half only
  // miss and fault on second half only
  // faults but pre-loaded

  // nonspec I$ with misses involved in the above

  uint64_t stack_start = DRAM_BASE + MPGSIZE - 0x100;

  trapframe_t tf;
  memset(&tf, 0, sizeof(tf));
  tf.epc = (uint64_t)run_test;
  tf.gpr[2] = stack_start; // sp
  pop_tf(&tf);

  return 99;
}
