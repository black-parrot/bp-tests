#include <string.h>
#include "bp_utils.h"
#include "vm.h"

/*
Test suite which exercises misaligned instruction support in a virtual memory context.

This application tests:
    - Execution of misaligned instructions with translation enabled
    - Execution of misaligned instructions when one or both halves fault or raise an exception
    - Handling of exception data (*tval, *epc) when misaligned instructions trigger a trap

It runs a sequence of scenarios first in the default chip configuration, then again with the
insruction cache configured for two alternative modes:
- "non-speculative" mode: every miss travels through the backend and is committed before the fill is
  serviced from memory.
- uncached mode: every request round-trips to memory

ITLB and I$ fills are likely sources of bugs when handling misaligned instructions because these
instructions require two fetches to be issued. One or both halves may trigger an exception and in
such a case the frontend must resume part-way through an instruction fetch.

The application configures a virtual memory environment in which the necessary code and data are
mapped in a sensible default location. For each test case to be run, a pair of pages is allocated
and configured as needed for the test.

Tests allocate pairs of pages because they test behavior when only part of an instruction faults.
This means the instruction spans two consecutive pages. When mapping a test pair, the two pages are
configured with potentially different permissions to enable provoking a partially successful fetch.

Test cases are two or three 32-bit instructions written on and around the page boundary.
Instructions are written into memory dynamically after mapping the appropriate pages. Each
instruction sequence is an optional nop followed by a return of a magic value to the caller. Some
tests "expect success", meaning the test passes if the instruction sequence is executed and the
magic value is returned. Other tests "expect a fault", i.e. correct processor operation implies that
a fault is raised before the return is retired.

If a fault is raised during a test, its information is logged in a shared memory space and control
is returned to the test harness, skipping any remaining portions of the dynamic instruction sequence
in the test pages. The test harness inspects the relevant flags to identify whether a fault was
encountered during execution and passes/fails the test as appropriate.

Mappings are configured in machine mode before lowering into usermode. Once the address space is
prepared, the program jumps into usermode and invokes the preconfigured test sequences, checking
the result of each as it goes. Faults trap into supervisor mode to be logged; control flow does not
return to machine mode.

Function names are prefixed with the privilege level in which they are intended to be used.
*/

volatile uint32_t *cfg_reg_icache_mode = (volatile uint32_t *) 0x200208;

#define NPT 3
#define l1pt pt[0]
#define l2pt pt[1]
#define l3pt pt[2]

// Address of start of the Nth "data page", an arbitrary range of 4k pages for testing use
// Intentionally _not_ the same VA as PA -- we apply an arbitrary 127-megapage offset.
#define DATA_PAGE_VADDR(data_page_num) ((128 * MPGSIZE) + DRAM_BASE + ((data_page_num) * PGSIZE))
#define DATA_PAGE_VADDR_TO_PADDR(addr) ((addr) - (128 * MPGSIZE) + MPGSIZE)

#define TEST_PAGE_NUM_FOR_TEST_NUM(test_num) ((test_num)*2)
// The address of the start of a fresh "data page", which can be used along with the one before it for this test
#define TEST_BOUNDARY_VADDR(test_num) (DATA_PAGE_VADDR(TEST_PAGE_NUM_FOR_TEST_NUM(test_num) + 1))

#define UMODE_TO_SMODE_CODE_OFFSET MPGSIZE

#define PAGE_PERMS_USER_ALL (PTE_U_LEAF)
#define PAGE_PERMS_SUPERVISOR_ALL (PTE_S_LEAF)
#define PAGE_PERMS_USER_NOEXEC (PAGE_PERMS_USER_ALL & ~(uint64_t)PTE_X)

#define FAULT_MAGIC 0x8BADF00D

#define s_set_icache_uncached() \
    asm volatile(      \
        "li a0, 0\n\t" \
        "li a1, 1\n\t" \
        "ecall\n\t"    \
    )

#define s_set_icache_default() \
    asm volatile(      \
        "li a0, 1\n\t" \
        "li a1, 1\n\t" \
        "ecall\n\t"    \
    )

#define s_set_icache_nonspec() \
    asm volatile(      \
        "li a0, 2\n\t" \
        "li a1, 1\n\t" \
        "ecall\n\t"    \
    )

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
// Misaligned instruction spanning page boundary, second half is in non-executable page
static const uint64_t test_6_access_fault_second_half_only = TEST_BOUNDARY_VADDR(6) - 6;
// Misaligned instruction spanning page boundary, first half is in non-executable page
static const uint64_t test_7_access_fault_first_half_only = TEST_BOUNDARY_VADDR(7) - 2;

uint64_t pt[NPT][PTES_PER_PT] __attribute__((aligned(PGSIZE))) = {0};

/*
 * Dummy function which returns to the caller. Used as the sepc target for test
 * -ending traps.
 */
uint64_t userspace_trap_return_trampoline() {
    return FAULT_MAGIC;
}


void s_handle_vm_fault(trapframe_t* tf) {
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

// Trap handler invoked by stvec target in vm_start.S
void handle_trap(trapframe_t* tf) {
    if (tf->cause == CAUSE_FETCH_PAGE_FAULT || tf->cause == CAUSE_FETCH_ACCESS)
        s_handle_vm_fault(tf);
    else
        terminate(-1);

    pop_tf(tf);
}

void m_init_vm() {
    // enable memory mappings
    uint64_t satp_val = ((uint64_t)l1pt >> PGSHIFT) | ((uint64_t)SATP_MODE_SV39 * (SATP_MODE & ~(SATP_MODE<<1)));
    write_csr(satp, satp_val);
    // Allow machine and supervisor modes to read/write user memory
    set_csr(mstatus, MSTATUS_SUM);
    set_csr(sstatus, SSTATUS_SUM);
    write_csr(stvec, trap_entry + UMODE_TO_SMODE_CODE_OFFSET);
    write_csr(sscratch, read_csr(mscratch));
    // Exceptions we don't care about can be routed to the default machine-mode
    // handler so they are pretty-printed and trigger an abort.
    write_csr(medeleg,
        (1 << CAUSE_FETCH_PAGE_FAULT) |
        (1 << CAUSE_FETCH_ACCESS));

    // fence to ensure the instruction writes take effect
    asm volatile ("fence.i");
}

uint64_t u_execute_test_gadget(uint64_t gadget_address) {
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

void u_execute_and_expect_fault(uint64_t gadget_address, uint64_t expected_pc, uint64_t expected_cause, uint64_t expected_val) {
    uint64_t result = u_execute_test_gadget(gadget_address);

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

void u_execute_and_expect_success(uint64_t gadget_address) {
    uint64_t result = u_execute_test_gadget(gadget_address);

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

void u_run_test_pass() {
    u_execute_and_expect_success(test_0_aligned_execution_across_page_boundary_gadget_address);
    u_execute_and_expect_success(test_1_misaligned_within_single_page_gadget_address);
    u_execute_and_expect_success(test_2_misaligned_execution_across_page_boundary_gadget_address);
    u_execute_and_expect_success(test_3_tlb_miss_both_halves_gadget_address);

    // Execute test 4 "secondary" gadget first to prime ITLB and I$
    u_execute_and_expect_success(test_4_tlb_miss_first_half_only_secondary_gadget_address);
    u_execute_and_expect_success(test_4_tlb_miss_first_half_only_primary_gadget_address);

    u_execute_and_expect_fault(
        test_5_access_fault_within_single_page,
        test_5_access_fault_within_single_page,
        CAUSE_FETCH_PAGE_FAULT,
        test_5_access_fault_within_single_page
    );

    u_execute_and_expect_fault(
        test_6_access_fault_second_half_only,
        test_6_access_fault_second_half_only+4,
        CAUSE_FETCH_PAGE_FAULT,
        test_6_access_fault_second_half_only+6
    );

    u_execute_and_expect_fault(
        test_7_access_fault_first_half_only,
        test_7_access_fault_first_half_only,
        CAUSE_FETCH_PAGE_FAULT,
        test_7_access_fault_first_half_only
    );
}

void u_test_main() {
    s_set_icache_default();
    u_run_test_pass();

    bp_print_string("Re-running test suite in nonspec mode...\n");
    s_set_icache_nonspec();
    u_run_test_pass();

    bp_print_string("Re-running test suite in uncached mode...\n");
    s_set_icache_uncached();
    u_run_test_pass();

    s_set_icache_default();

    bp_print_string("All tests passed\n");
    terminate(0);
}

// Trap handler invoked by vm_start.S in response to an ECALL from U-mode
void handle_u_ecall(uint64_t arg) {
    if (arg == 0) {
        bp_print_string("Setting I$ to UNcached mode\n");
        // Set I$ mode to uncached
        *cfg_reg_icache_mode = 0;
        flush_tlb();
        flush_icache();
    } else if (arg == 1) {
        bp_print_string("Setting I$ to cached mode (default)\n");
        // Set I$ mode to cached (default)
        *cfg_reg_icache_mode = 1;
        flush_tlb();
        flush_icache();
    } else if (arg == 2) {
        bp_print_string("Setting I$ to nonspec mode\n");
        // Set I$ mode to nonspeculative
        *cfg_reg_icache_mode = 2;
        flush_tlb();
        flush_icache();
    } else {
        bp_hprint_uint64(arg);
        bp_panic("Unknown ECALL arg");
    }
}

static void m_map_cfg_page() {
    // The lowest virtual gigapage is mapped 1:1 to physical addresses for CFG registers
    // Only the first two megapages are required, but gigapage requires fewer page tables

    uint64_t aligned_va = 0;
    uint64_t aligned_pa = 0;

    l1pt[vpn2(aligned_va)] = ((uint64_t)aligned_pa >> PGSHIFT << PTE_PPN_SHIFT) | PAGE_PERMS_USER_ALL;;
}

static void m_map_code_page() {
    // Megapage starting at DRAM_BASE is mapped 1:1 to physical addresses so that we don't have to
    // handle a relocation

    uint64_t aligned_va = DRAM_BASE;
    uint64_t aligned_pa = DRAM_BASE;

    l1pt[vpn2(aligned_va)] = ((uint64_t)l2pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
    l2pt[vpn1(aligned_va)] = ((uint64_t)aligned_pa >> PGSHIFT << PTE_PPN_SHIFT) | PAGE_PERMS_USER_ALL;
    // Trap handler executes in S-mode which isn't allowed to execute instructions from U-mode pages.
    // Duplicate the code/data page for S-mode to use purely for trap handler code.
    l2pt[vpn1(aligned_va+UMODE_TO_SMODE_CODE_OFFSET)] = ((uint64_t)aligned_pa >> PGSHIFT << PTE_PPN_SHIFT) | PAGE_PERMS_SUPERVISOR_ALL;
}

static void m_map_test_page(uint64_t test_page_num, uint64_t leaf_perm) {
    uint64_t aligned_va = DATA_PAGE_VADDR(test_page_num);
    uint64_t aligned_pa = DATA_PAGE_VADDR_TO_PADDR(aligned_va);

    l1pt[vpn2(aligned_va)] = ((uint64_t)l2pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
    l2pt[vpn1(aligned_va)] = ((uint64_t)l3pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
    l3pt[vpn0(aligned_va)] = ((uint64_t)aligned_pa >> PGSHIFT << PTE_PPN_SHIFT) | leaf_perm;
}

/*
 * Maps a pair of test pages, indexed by the ID of the test being prepared.
 */
void m_map_test_pair(int test_num, uint64_t first_page_perms, uint64_t second_page_perms) {
    uint64_t first_page_num = TEST_PAGE_NUM_FOR_TEST_NUM(test_num);
    m_map_test_page(first_page_num, first_page_perms);
    m_map_test_page(first_page_num+1, second_page_perms);
}

/*
 * Places the given 32-bit instruction and the specified address, requiring only
 * 16-bit alignment.
 */
void m_place_instruction(uint64_t vaddr, uint32_t instruction) {
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

void m_place_dummy_instruction(uint64_t vaddr) {
    m_place_instruction(vaddr, 0x00000013); // nop
}

/*
 * Uses m_place_instruction to place a two-instruction sequence which returns
 * 0x42 when invoked.
 */
void m_place_end_instructions(uint64_t vaddr) {
    m_place_instruction(vaddr,   0x04200513); // li a0,0x42
    m_place_instruction(vaddr+4, 0x00008067); // ret
}

int main() {
    m_map_code_page();
    m_map_cfg_page();

    m_map_test_pair(0, PAGE_PERMS_USER_ALL, PAGE_PERMS_USER_ALL);
    m_place_dummy_instruction(test_0_aligned_execution_across_page_boundary_gadget_address);
    m_place_end_instructions(test_0_aligned_execution_across_page_boundary_gadget_address+4);

    m_map_test_pair(1, PAGE_PERMS_USER_ALL, PAGE_PERMS_USER_ALL);
    m_place_dummy_instruction(test_1_misaligned_within_single_page_gadget_address);
    m_place_end_instructions(test_1_misaligned_within_single_page_gadget_address+4);

    m_map_test_pair(2, PAGE_PERMS_USER_ALL, PAGE_PERMS_USER_ALL);
    m_place_dummy_instruction(test_2_misaligned_execution_across_page_boundary_gadget_address);
    m_place_end_instructions(test_2_misaligned_execution_across_page_boundary_gadget_address+4);

    m_map_test_pair(3, PAGE_PERMS_USER_ALL, PAGE_PERMS_USER_ALL);
    m_place_dummy_instruction(test_3_tlb_miss_both_halves_gadget_address);
    m_place_end_instructions(test_3_tlb_miss_both_halves_gadget_address+4);

    m_map_test_pair(4, PAGE_PERMS_USER_ALL, PAGE_PERMS_USER_ALL);
    m_place_dummy_instruction(test_4_tlb_miss_first_half_only_primary_gadget_address);
    m_place_end_instructions(test_4_tlb_miss_first_half_only_primary_gadget_address+4);
    m_place_end_instructions(test_4_tlb_miss_first_half_only_secondary_gadget_address);

    m_map_test_pair(5, PAGE_PERMS_USER_NOEXEC, PAGE_PERMS_USER_ALL);
    m_place_dummy_instruction(test_5_access_fault_within_single_page);
    m_place_end_instructions(test_5_access_fault_within_single_page+4);

    m_map_test_pair(6, PAGE_PERMS_USER_ALL, PAGE_PERMS_USER_NOEXEC);
    m_place_dummy_instruction(test_6_access_fault_second_half_only);
    m_place_end_instructions(test_6_access_fault_second_half_only+4);

    m_map_test_pair(7, PAGE_PERMS_USER_NOEXEC, PAGE_PERMS_USER_ALL);
    m_place_end_instructions(test_7_access_fault_first_half_only);

    m_init_vm();

    uint64_t stack_start = DRAM_BASE + MPGSIZE - 0x100;

    trapframe_t tf;
    memset(&tf, 0, sizeof(tf));
    tf.epc = (uint64_t)u_test_main;
    tf.gpr[2] = stack_start; // sp
    pop_tf(&tf);

    bp_panic("Control returned to machine mode entry point");
    return -1;
}
