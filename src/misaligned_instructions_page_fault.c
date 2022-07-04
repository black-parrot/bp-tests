#include <string.h>
#include "bp_utils.h"
#include "vm.h"

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

#define PAGE_SIZE_KILOPAGE 1
#define PAGE_SIZE_MEGAPAGE 2

#define PAGE_PERMS_ALL PTE_U_LEAF

struct fault_info_t {
  uint64_t address;
  uint64_t cause;
  bool present;
} latest_fault_info;

void trap_entry();
void pop_tf(trapframe_t*);

typedef uint64_t (*test_gadget_t)();

// Sanity check (fully aligned)
static const uint64_t test_0_aligned_execution_across_page_boundary_gadget_address = TEST_BOUNDARY_VADDR(0) - 4;
// TODO: below comment doesn't match test. Separate into two tests.
// Misaligned but entirely within a single page (confirm that misaligned execution with VM enabled works at all)
static const uint64_t test_1_misaligned_within_single_page_gadget_address = TEST_BOUNDARY_VADDR(1) - 10;
// Same as above, but now the first instruction is misaligned so it crosses the page boundary
static const uint64_t test_2_tlb_miss_both_halves_gadget_address = TEST_BOUNDARY_VADDR(2) - 2;

uint64_t pt[NPT][PTES_PER_PT] __attribute__((aligned(PGSIZE)));

static void map_cfg_page() {
  // The lowest virtual gigapage is mapped 1:1 to physical addresses for CFG registers
  // Only the first two megapages are required, but gigapage requires fewer page tables

    uint64_t aligned_va = 0;
    uint64_t aligned_pa = 0;

    l1pt[vpn2(aligned_va)] = ((uint64_t)aligned_pa >> PGSHIFT << PTE_PPN_SHIFT) | PAGE_PERMS_ALL;;
}

static void map_code_page() {
  // Megapage starting at DRAM_BASE is mapped 1:1 to physical addresses so that we don't have to
  // handle a relocation

    uint64_t aligned_va = DRAM_BASE;
    uint64_t aligned_pa = DRAM_BASE;

    l1pt[vpn2(aligned_va)] = ((uint64_t)l2pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
    l2pt[vpn1(aligned_va)] = ((uint64_t)aligned_pa >> PGSHIFT << PTE_PPN_SHIFT) | PAGE_PERMS_ALL;
}


static void map_test_page(uint64_t test_page_num, uint64_t leaf_perm) {
    uint64_t aligned_va = DATA_PAGE_VADDR(test_page_num);
    uint64_t aligned_pa = DATA_PAGE_VADDR_TO_PADDR(aligned_va);

    bp_hprint_uint64(aligned_va);
    bp_cprint('\n');

    bp_hprint_uint64(aligned_pa);
    bp_cprint('\n');
    bp_cprint('\n');

    l1pt[vpn2(aligned_va)] = ((uint64_t)l2pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
    l2pt[vpn1(aligned_va)] = ((uint64_t)l3pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
    l3pt[vpn0(aligned_va)] = ((uint64_t)aligned_pa >> PGSHIFT << PTE_PPN_SHIFT) | leaf_perm;
}

void handle_fault(uint64_t addr, int cause) {
  if (latest_fault_info.present) {
    terminate(-1);
    while (1);
  }

  latest_fault_info.address = addr;
  latest_fault_info.cause = cause;
  latest_fault_info.present = true;

  // TODO: modify target pc to return to the "test finished" handler
  // Temp
  terminate(55);
}

void handle_trap(trapframe_t* tf) {
  if (tf->cause == CAUSE_FETCH_PAGE_FAULT || tf->cause == CAUSE_FETCH_ACCESS)
    handle_fault(tf->badvaddr, tf->cause);
  else
    terminate(-1);

  pop_tf(tf);
}

void init_vm() {
  // enable memory mappings
  uint64_t satp_val = ((uint64_t)l1pt >> PGSHIFT) | ((uint64_t)SATP_MODE_SV39 * (SATP_MODE & ~(SATP_MODE<<1)));
  write_csr(satp, satp_val);
  set_csr(mstatus, MSTATUS_SUM);
  write_csr(stvec, trap_entry);
  write_csr(sscratch, read_csr(mscratch)); // TODO: is this right? removed a mapping
  write_csr(medeleg,
    (1 << CAUSE_FETCH_PAGE_FAULT) |
    (1 << CAUSE_FETCH_ACCESS));

  // TODO: temp, try all the fences because things don't work
  asm volatile ("sfence.vma");
  asm volatile ("fence");

  // fence to ensure the instruction writes take effect (unrelated to SATP)
  asm volatile ("fence.i");
}

void execute_and_expect_fault(uint64_t gadget_address, uint64_t expected_address, uint64_t expected_cause) {
  // TODO
}

void execute_and_expect_success(uint64_t gadget_address) {
  latest_fault_info = (const struct fault_info_t){ 0 };

  test_gadget_t gadget_fn = (test_gadget_t)gadget_address;

  bp_print_string("jumping to: ");
  bp_hprint_uint64((uint64_t)gadget_fn);
  bp_cprint('\n');

  asm volatile ("li a0, 0"); // TODO: introduce a fake param instead?
  uint64_t result = gadget_fn();
  // "end" instruction sequence returns 0x42
  bp_print_string("result: ");
  bp_hprint_uint64(result);

  if (result == 0x42) {
    bp_print_string(" (pass)\n");
  } else {
    bp_print_string(" (fail)\n");
    terminate(-1);
  }

  if (latest_fault_info.present) {
    bp_print_string("unexpected fault:\n\taddress: ");
    bp_hprint_uint64(latest_fault_info.address);
    bp_print_string("\n\tcause: ");
    bp_hprint_uint64(latest_fault_info.cause);
    bp_cprint('\n');
    terminate(-1);
  }
}

void run_test() {
  // bp_print_string("gadget address:");
  // bp_hprint_uint64(test_0_aligned_execution_across_page_boundary_gadget_address);
  // bp_cprint('\n');
  // bp_print_string("insn at address: ");
  // bp_hprint_uint64(*((volatile uint64_t*)test_0_aligned_execution_across_page_boundary_gadget_address));
  // bp_cprint('\n');

  // TODO: yet another mindless fencei 
  asm volatile ("fence.i");

  execute_and_expect_success(test_0_aligned_execution_across_page_boundary_gadget_address);
  execute_and_expect_success(test_1_misaligned_within_single_page_gadget_address);
  // execute_and_expect_success(test_2_tlb_miss_both_halves_gadget_address);

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

  map_test_pair(0, PAGE_PERMS_ALL, PAGE_PERMS_ALL);
  place_dummy_instruction(test_0_aligned_execution_across_page_boundary_gadget_address);
  place_end_instructions(test_0_aligned_execution_across_page_boundary_gadget_address+4);

  map_test_pair(1, PAGE_PERMS_ALL, PAGE_PERMS_ALL);
  place_dummy_instruction(test_1_misaligned_within_single_page_gadget_address);
  place_end_instructions(test_1_misaligned_within_single_page_gadget_address+4);

  // TODO: jump directly to misaligned instruction on the page boundary

  // TODO: remove
  // asm volatile ("fence.i");

  // map_test_pair(2, PAGE_PERMS_ALL, PAGE_PERMS_ALL);
  // place_dummy_instruction(test_2_tlb_miss_both_halves_gadget_address);
  // place_end_instructions(test_2_tlb_miss_both_halves_gadget_address+4);

  // bp_hprint_uint64(test_0_aligned_execution_across_page_boundary_gadget_address);
  // bp_cprint('\n');

  init_vm();

  // TLB miss both halves
  // TLB miss second half only
  // TLB miss first half only
  // miss and fault on both halves
  // miss and fault on first half only
  // miss and fault on second half only
  // faults but pre-loaded

  uint64_t stack_start = DRAM_BASE + MPGSIZE - 0x100;

  trapframe_t tf;
  memset(&tf, 0, sizeof(tf));
  tf.epc = (uint64_t)run_test;
  tf.gpr[2] = stack_start; // sp
  pop_tf(&tf);

  return 99;
}
