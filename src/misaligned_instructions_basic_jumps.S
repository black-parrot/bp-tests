# Tests jumps to, from, and between misaligned 32-bit instructions with untrained predictors.

# Comment out to produce a program which executes the same instructions but all instuctions are
# aligned. Output cycle count should be almost exactly 19 cycles less.
#define ENABLE_MISALIGNMENT

# An arbitrary number of NOPs, not indended to be executed, used to separate fetches by far longer
# than could be linearly pre-fetched by a naive branch prediction. Without these, a jump to pc+2
# would incidentally already be performing the right fetches regardless of the jump.
#define NOP_PADDING \
    nop; \
    nop; \
    nop; \
    nop; \
    nop; \
    nop;

#ifdef ENABLE_MISALIGNMENT
#define MISALIGN .short 0x0000
#define REALIGN  .short 0x0000
#else
#define MISALIGN
#define REALIGN  .word  0x00000000
#endif

.globl main
main:
    addi    sp,sp,-32
    sd      ra,24(sp)
    sd      s0,16(sp)

    rdcycle t0

# unconditional jump aligned -> misaligned
    j .Lmisaligned_1
    NOP_PADDING
    MISALIGN

.Lmisaligned_1:
    # bp_cprint is aligned, so these are each jal to aligned and jalr (ret) back to misaligned
    li      a0,72 # H
    jal     ra,bp_cprint
    li      a0,101 # e
    jal     ra,bp_cprint
    li      a0,108 # l
    jal     ra,bp_cprint
    li      a0,108 # l
    jal     ra,bp_cprint
    li      a0,111 # o
    jal     ra,bp_cprint
    li      a0,32 # space
    jal     ra,bp_cprint

# unconditional jump misaligned -> misaligned
    j .Lmisaligned_2
    NOP_PADDING

.Lmisaligned_2:
    li      a0,77 # M
    jal     ra,bp_cprint
    li      a0,105 # i
    jal     ra,bp_cprint
    li      a0,115 # s
    jal     ra,bp_cprint
    li      a0,97 # a
    jal     ra,bp_cprint
    li      a0,108 # l
    jal     ra,bp_cprint
    li      a0,105 # i
    jal     ra,bp_cprint
    li      a0,103 # g
    jal     ra,bp_cprint
    li      a0,110 # n
    jal     ra,bp_cprint
    li      a0,101 # e
    jal     ra,bp_cprint
    li      a0,100 # d
    jal     ra,bp_cprint
    li      a0,32 # space
    jal     ra,bp_cprint

# unconditional jump misaligned -> aligned
    j .Laligned_1
    NOP_PADDING
    REALIGN

.Laligned_1:
    li      a0,87 # W
    jal     ra,bp_cprint
    li      a0,111 # o
    jal     ra,bp_cprint
    li      a0,114 # r
    jal     ra,bp_cprint
    li      a0,108 # l
    jal     ra,bp_cprint
    li      a0,100 # d
    jal     ra,bp_cprint
    li      a0,33 # !
    jal     ra,bp_cprint
    li      a0,10 # \n
    jal     ra,bp_cprint

    fence
    rdcycle t1
    sub a0, t1, t0
    jal     ra, bp_hprint_uint64

    li      a0,10 # \n
    jal     ra,bp_cprint

    li      a0,0

    ld      ra,24(sp)
    ld      s0,16(sp)
    addi    sp,sp,32
    ret
