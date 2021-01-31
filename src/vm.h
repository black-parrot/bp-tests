#include <stdint.h>

#define CAUSE_MISALIGNED_FETCH    0x0
#define CAUSE_FETCH_ACCESS        0x1
#define CAUSE_ILLEGAL_INSTRUCTION 0x2
#define CAUSE_BREAKPOINT          0x3
#define CAUSE_MISALIGNED_LOAD     0x4
#define CAUSE_LOAD_ACCESS         0x5
#define CAUSE_MISALIGNED_STORE    0x6
#define CAUSE_STORE_ACCESS        0x7
#define CAUSE_USER_ECALL          0x8
#define CAUSE_SUPERVISOR_ECALL    0x9
#define CAUSE_HYPERVISOR_ECALL    0xa
#define CAUSE_MACHINE_ECALL       0xb
#define CAUSE_FETCH_PAGE_FAULT    0xc
#define CAUSE_LOAD_PAGE_FAULT     0xd
#define CAUSE_STORE_PAGE_FAULT    0xf

#define PTE_V      0x01
#define PTE_R      0x02
#define PTE_W      0x04
#define PTE_X      0x08
#define PTE_U      0x10
#define PTE_G      0x20
#define PTE_A      0x40
#define PTE_D      0x80
#define PTE_S_LEAF PTE_V | PTE_R | PTE_W | PTE_X | PTE_A | PTE_D
#define PTE_U_LEAF PTE_V | PTE_U | PTE_R | PTE_W | PTE_X | PTE_A | PTE_D

#define PGSHIFT       12
#define PTE_PPN_SHIFT 10
#define PGLEVELBITS   9
#define PTES_PER_PT   (1UL << PGLEVELBITS)
#define PGSIZE        (1UL << PGSHIFT)
#define MPGSIZE       (1UL << (PGSHIFT + PGLEVELBITS))
#define GPGSIZE       (1UL << (PGSHIFT + 2*PGLEVELBITS))

#define SATP_MODE_SV39 8
#define SATP_MODE      0xF000000000000000
#define SATP_ASID      0x0FFFF00000000000
#define SATP_PPN       0x00000FFFFFFFFFFF
#define MSTATUS_SUM    0x00040000
#define DRAM_BASE      0x80000000

#define vpn0(va) ((va >> PGSHIFT) % PTES_PER_PT)
#define vpn1(va) ((va >> (PGSHIFT + PGLEVELBITS)) % PTES_PER_PT)
#define vpn2(va) ((va >> (PGSHIFT + 2*PGLEVELBITS)) % PTES_PER_PT)

#define read_csr(reg) ({ uint64_t __tmp; \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })

#define write_csr(reg, val) ({ \
  asm volatile ("csrw " #reg ", %0" :: "rK"(val)); })

#define swap_csr(reg, val) ({ uint64_t __tmp; \
  asm volatile ("csrrw %0, " #reg ", %1" : "=r"(__tmp) : "rK"(val)); \
  __tmp; })

#define set_csr(reg, bit) ({ uint64_t __tmp; \
  asm volatile ("csrrs %0, " #reg ", %1" : "=r"(__tmp) : "rK"(bit)); \
  __tmp; })

#define clear_csr(reg, bit) ({ uint64_t __tmp; \
  asm volatile ("csrrc %0, " #reg ", %1" : "=r"(__tmp) : "rK"(bit)); \
  __tmp; })

#define flush_tlb() asm volatile ("sfence.vma")

#define terminate(code)             \
  asm volatile("mv a0, %0\n\t"      \
               "ecall\n\t"          \
               ::"r" ((int)(code)):)

typedef struct
{
  long gpr[32];
  long sr;
  long epc;
  long badvaddr;
  long cause;
  long hartid;
} trapframe_t;

