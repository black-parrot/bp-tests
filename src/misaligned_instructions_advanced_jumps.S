# Tests correct predictions of jumps to/from misaligned code, and chains of jumps.

# Comment out to produce a program which executes the same instructions but all instuctions are
# aligned.
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

    # warm up predictors and cache for perform_test
    # conditional jump will be learned taken
    li a1, 0
    jal     ra,perform_test
    jal     ra,perform_test

    # clear a0 to dummy value, should be overwritten in perform_test
    li a0, 0x58
    # configure the conditional jump to be taken
    li a1, 1

    rdcycle s0

    # Run twice. First time, the conditional should be mispredicted. Second, it should be correct.
    jal     ra,perform_test
    jal     ra,perform_test

    fence
    rdcycle s1
    sub a0, s1, s0
    jal     ra, bp_hprint_uint64

    li      a0,0x20 # \n
    jal     ra,bp_cprint

    li      a0,0

    ld      ra,24(sp)
    ld      s0,16(sp)
    addi    sp,sp,32
    ret

perform_test:
    addi    sp,sp,-16
    sd      ra,0(sp)

    # chain some jumps (back-to-back), both predictable from the BTB
    j .Lmisaligned_1
    NOP_PADDING
    MISALIGN
.Lmisaligned_1:
    j .Lmisaligned_2
    NOP_PADDING
.Lmisaligned_2:
    # do something after the jumps
    li      a0,0x31 # 1
    jal     ra,bp_cprint

    # call a function and return, chaining both the calls and the returns
    jal     ra,some_fn_1

    # do something after the return
    li      a0,0x32 # 2
    jal     ra,bp_cprint

    beqz    a1, .Lskip_3
    li      a0,0x33 # 3
    jal     ra,bp_cprint
.Lskip_3:

    li      a0,10 # \n
    jal     ra,bp_cprint

    ld      ra,0(sp)
    addi    sp,sp,16
    ret

# everything after perform_test above remains misaligned
some_fn_1:
    # use alternate link register so that we have call -> call -> ret -> ret with nothing in between
    jal     x5,some_fn_2
    ret

some_fn_2:
    jalr    x5, ra, 0	

REALIGN
