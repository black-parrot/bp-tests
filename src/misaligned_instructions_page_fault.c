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
// Intentionally _not_ the same VA as PA
#define DATA_PAGE_VADDR(data_page_num) ((128 * MPGSIZE) + ((data_page_num) * PGSIZE))
#define DATA_PAGE_VADDR_TO_PADDR(addr) ((addr) - (128 * MPGSIZE) + DRAM_BASE + MPGSIZE)

#define TEST_PAGE_NUM_FOR_TEST_NUM(test_num) ((test_num)*2)
// The address of the start of a fresh "data page", which can be used along with the one before it for this test
#define TEST_BOUNDARY_VADDR(test_num) (DATA_PAGE_VADDR(TEST_PAGE_NUM_FOR_TEST_NUM(test_num) + 1))

#define PAGE_SIZE_KILOPAGE 1
#define PAGE_SIZE_MEGAPAGE 2

#define PAGE_PERMS_ALL PTE_U_LEAF

// #define pa2kva(pa) ((uint64_t)(pa) - DRAM_BASE - MPGSIZE)
// #define kva2pa(va) ((uint64_t)(va) + DRAM_BASE + MPGSIZE)

// typedef struct {
//   uint64_t address;
//   uint64_t cause;
//   bool present;
// } fault_history_t;

// #define MAX_FAULT_COUNT 10
// fault_history_t fault_history[MAX_FAULT_COUNT] = { 0 };

struct fault_info_t {
  uint64_t address;
  uint64_t cause;
  bool present;
} latest_fault_info;

void trap_entry();
void pop_tf(trapframe_t*);

typedef uint64_t (*test_gadget_t)();

static const uint64_t test_0_tlb_miss_both_halves_gadget_address = TEST_BOUNDARY_VADDR(0) - 2;

uint64_t pt[NPT][PTES_PER_PT] __attribute__((aligned(PGSIZE)));

static void map_cfg_page() {
  // The lowest virtual gigapage is mapped 1:1 to physical addresses for CFG registers
  // Only the first two megapages are required, but gigapage requires fewer page tables

    uint64_t aligned_va = 0;
    uint64_t aligned_pa = 0;

    l1pt[vpn2(aligned_va)] = ((uint64_t)aligned_pa >> PGSHIFT << PTE_PPN_SHIFT) | PAGE_PERMS_ALL;;
}

static void map_code_page() {
  // Megapage starting at DRAM_BASE is mapped 1:1 to physical addresses so that 

    uint64_t aligned_va = DRAM_BASE;
    uint64_t aligned_pa = DRAM_BASE;

    l1pt[vpn2(aligned_va)] = ((uint64_t)l2pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
    l2pt[vpn1(aligned_va)] = ((uint64_t)aligned_pa >> PGSHIFT << PTE_PPN_SHIFT) | PAGE_PERMS_ALL;
}


static void map_test_page(uint64_t test_page_num, uint64_t leaf_perm) {
    uint64_t aligned_va = DATA_PAGE_VADDR(test_page_num);
    uint64_t aligned_pa = DATA_PAGE_VADDR_TO_PADDR(aligned_va);

    l1pt[vpn2(aligned_va)] = ((uint64_t)l2pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
    l2pt[vpn1(aligned_va)] = ((uint64_t)l3pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
    l3pt[vpn0(aligned_va)] = ((uint64_t)aligned_pa >> PGSHIFT << PTE_PPN_SHIFT) | leaf_perm;
}


    // bp_hprint_uint64(vpn2(aligned_va));
    // bp_cprint('\n');
    // bp_hprint_uint64(vpn1(aligned_va));
    // bp_cprint('\n');
    // bp_hprint_uint64(vpn0(aligned_va));
    // bp_cprint('\n');
    // bp_cprint('\n');

// static void page_init(uint64_t addr) {
//   // First virtual 4k page is allocated to the first 4k physical page
//   // the rest are mapped to the vpn0'th page of the second megapage
//   addr = (addr/PGSIZE)*PGSIZE;
//   uint64_t page_pa;
//   if(addr == 0)
//     page_pa = DRAM_BASE;
//   else 
//     page_pa = DRAM_BASE + MPGSIZE + (PGSIZE * vpn0(addr));
//   l1pt[vpn2(addr)] = ((uint64_t)kva2pa(l2pt) >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
//   l2pt[vpn1(addr)] = ((uint64_t)kva2pa(l3pt) >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
//   l3pt[vpn0(addr)] = ((uint64_t)page_pa >> PGSHIFT << PTE_PPN_SHIFT) | PTE_U_LEAF; 
// }

// static void megapage_init(uint64_t addr) {
//   // Virtual megapages are mapped to the vpn1'th megapage
//   addr = (addr/MPGSIZE)*MPGSIZE;
//   uint64_t page_pa = DRAM_BASE + (MPGSIZE * vpn1(addr));
//   l1pt[vpn2(addr)] = ((uint64_t)kva2pa(l2pt) >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
//   l2pt[vpn1(addr)] = ((uint64_t)page_pa >> PGSHIFT << PTE_PPN_SHIFT) | PTE_U_LEAF;
// }

// static void gigapage_init(uint64_t addr) {
//   // All virtual gigapage are mapped to the first gigapage
//   addr = (addr/GPGSIZE)*GPGSIZE;
//   uint64_t page_pa = DRAM_BASE;
//   l1pt[vpn2(addr)] = ((uint64_t)page_pa >> PGSHIFT << PTE_PPN_SHIFT) | PTE_U_LEAF;
// }

void handle_fault(uint64_t addr, int cause) {
  if (latest_fault_info.present) {
    terminate(-1);
    while (1);
  }

  latest_fault_info.address = addr;
  latest_fault_info.cause = cause;
  latest_fault_info.present = true;

  // TODO: don't continue?
}

void handle_trap(trapframe_t* tf) {
  if (tf->cause == CAUSE_FETCH_PAGE_FAULT || tf->cause == CAUSE_FETCH_ACCESS)
    handle_fault(tf->badvaddr, tf->cause);
  else
    terminate(-1);

  pop_tf(tf);
}

// int main(int argc, char** argv) {

//   uint64_t satp_val = ((uint64_t)l1pt >> PGSHIFT) | ((uint64_t)SATP_MODE_SV39 * (SATP_MODE & ~(SATP_MODE<<1)));
//   write_csr(satp, satp_val);
//   set_csr(mstatus, MSTATUS_SUM);
//   write_csr(stvec, pa2kva(trap_entry));
//   write_csr(sscratch, pa2kva(read_csr(mscratch)));
//   write_csr(medeleg,
//     (1 << CAUSE_FETCH_PAGE_FAULT) |
//     (1 << CAUSE_LOAD_PAGE_FAULT) |
//     (1 << CAUSE_STORE_PAGE_FAULT));

//   // Allocate the last megapage to kernel
//   l1pt[PTES_PER_PT-1] = ((uint64_t)kpt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
//   kpt[PTES_PER_PT-1] = ((uint64_t)DRAM_BASE >> PGSHIFT << PTE_PPN_SHIFT) | PTE_S_LEAF;

//   trapframe_t tf;
//   memset(&tf, 0, sizeof(tf));
//   tf.epc = (uint64_t)userTest - DRAM_BASE;
//   tf.gpr[2] = PGSIZE - 0x100;
//   pop_tf(&tf);

//   return 0;  
// }

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

  // fence to ensure the instruction writes take effect (unrelated to SATP)
  asm volatile ("fence.i");
}

void execute_and_expect_fault(uint64_t gadget_address, uint64_t expected_address, uint64_t expected_cause) {
  // TODO
}

void execute_and_expect_success(uint64_t gadget_address) {

  test_gadget_t gadget_fn = (test_gadget_t)gadget_address;
  bp_hprint_uint64(gadget_address);
  asm volatile ("li a0, 0"); // TODO: introduce a fake param instead?
  uint64_t result = gadget_fn();
  // "end" instruction sequence returns 0x42
  if (result != 0x42) {
    terminate(-1);
  }

  if (latest_fault_info.present) {
    terminate(-1);
  }
}

void run_test() {
  latest_fault_info = (const struct fault_info_t){ 0 };
  execute_and_expect_success(test_0_tlb_miss_both_halves_gadget_address);

  terminate(0); // temp
}


void map_test_pair(int test_num, uint64_t first_page_perms, uint64_t second_page_perms) {
  uint64_t first_page_num = TEST_PAGE_NUM_FOR_TEST_NUM(test_num);
  map_test_page(first_page_num, first_page_perms);
  map_test_page(first_page_num+1, second_page_perms);
}

void place_instruction(uint64_t vaddr, uint32_t instruction) {
  // TODO: do we allow misaligned stores?
  *((uint64_t*)DATA_PAGE_VADDR_TO_PADDR(vaddr)) = instruction;
}

void place_dummy_instruction(uint64_t vaddr) {
  place_instruction(vaddr, 0x00000013); // nop
}

void place_end_instructions(uint64_t vaddr) {
  place_instruction(vaddr,   0x04200513); // li	a0,0x42
  place_instruction(vaddr+4, 0x00008067); // ret
}


int main(int argc, char** argv) {

  map_code_page();
  map_cfg_page(); // TODO: this might not be needed

  // // boot vector page stays where it is, to avoid managing a relocation
  // map_page(DRAM_BASE, DRAM_BASE, PAGE_SIZE_MEGAPAGE, PAGE_PERMS_ALL);
  // // first two megapages mapped 1:1 to make cfg regs available at original locations
  // // TODO: following two break things
  // map_page(0, 0, PAGE_SIZE_MEGAPAGE, PAGE_PERMS_ALL);
  // // map_page(MPGSIZE, MPGSIZE, PAGE_SIZE_MEGAPAGE, PAGE_PERMS_ALL);

  map_test_pair(0, PAGE_PERMS_ALL, PAGE_PERMS_ALL);
  place_dummy_instruction(test_0_tlb_miss_both_halves_gadget_address);
  place_end_instructions(test_0_tlb_miss_both_halves_gadget_address+4);

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
