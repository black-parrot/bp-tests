import numpy as np
from math import ceil, floor
from typing import Tuple



# ==========================================================
#   FROM RASA
# ==========================================================
#       RESNET50-1: N=32, K=C=64, X=Y=56, R=S=1
#       RESNET50-2: N=32, K=C=64, X=Y=56, R=S=3
#       RESNET50-3: N=32, K=512, C=1024, X=Y=14, R=S=1
#       DLRM-1: N=512, NIN=1024, NON=1024
#       DLRM-2: N=512, NIN=1024, NON=64
#       DLRM-3: N=512, NIN=2048, NON=2048
#       BERT-1: N=256, NIN=768, NON=768
#       BERT-2: N=256, NIN=3072, NON=768
#       BERT-3: N=256, NIN=768, NON=3072
# ==========================================================



def get_block_shape_2d(shape, sa_h=4, sa_w=4) -> Tuple[int,int,int,int]:
    assert len(shape) == 2
    rows, cols = (
        round_up_to_multiple(shape[0], sa_h),
        round_up_to_multiple(shape[1], sa_w),
    )
    blocks_y = rows // sa_h
    blocks_x = cols // sa_w
    return (blocks_y, blocks_x, sa_h, sa_w)



def generate_matmul_kernel(A_shape, W_shape, sa_h=4, sa_w=4):
    result = []
    assert len(A_shape) == 2
    assert len(W_shape) == 2
    I_blk, J_blk, _, _ = get_block_shape_2d(A_shape, sa_h, sa_w)
    J_blk, K_blk, _, _ = get_block_shape_2d(W_shape, sa_h, sa_w)
    result.append(f"for(int i = 0; i < {I_blk}; i++) {'{'}")
    result.append(f'uint64_t a_offset = i * {J_blk * 16};')
    result.append('_w = w_ptr - 16;')
    result.append(f"for(int k = 0; k < {K_blk}; k++) {'{'}")
    result.append('_a = a_ptr - 16 + i * a_offset;')
    for j in range(J_blk):
        #result.append(asm_load("act", j==0, j==(J_blk-1)))
        #result.append(asm_load("wt",  j==0, j==(J_blk-1)))
        result.append(asm_load("act", False, j==(J_blk-1)))
        result.append(asm_load("wt",  False, j==(J_blk-1)))
    result.append(f'__asm__ __volatile__(".insn i 0x0b, 0b101, x0, %0, 0"::"rK"(_r));')
    result.append(f'_r += 16;')
    result.append("}")
    result.append("}")
    return result


def asm_load(dtype, first, last):
    block_size = 64
    op = {
        ( "act" , False ) : '0b010' ,
        ( "act" , True  ) : '0b011' ,
        ( "wt"  , False ) : '0b000' ,
        ( "wt"  , True  ) : '0b001' ,
    }[(dtype,last)]
    ptr = {
        "act" : '_a' ,
        "wt"  : '_w' ,
    }[dtype]
    if first:
        return f'__asm__ __volatile__(".insn i 0x0b, {op}, %0, 0(%1)" : "+r"({ptr}) : "r"({ptr}) );'
    else:
        return f'__asm__ __volatile__(".insn i 0x0b, {op}, %0, {block_size}(%1)" : "+r"({ptr}) : "r"({ptr}) );'


def asm_store(shape, blk_row, blk_col):
    block_size = 64 # bytes
    ptr = '_r'
    block_rows, block_cols, _, _ = shape
    offset = blk_row * (block_rows * block_size) + blk_col * block_size
    expr = f'((uint64_t){ptr} + {offset})'
    return f'__asm__ __volatile__(".insn i 0x0b, 0b101, x0, %0, 0"::"rK"({expr}));'


def round_up_to_multiple(val: int, multiple: int) -> int:
    mod = val % multiple
    if mod == 0:
        return val
    return val + (multiple - mod)


if __name__ == '__main__':
    I,J,K = 4,128,128
    print('#define mm(a_ptr,w_ptr,r_ptr) ({ \\')
    print('\tvolatile uint32_t null = 0; \\')
    print('\tvolatile uint32_t *null_ptr = &null; \\')
    print('\tvolatile uint32_t *_a = a_ptr; \\')
    print('\tvolatile uint32_t *_w = w_ptr; \\')
    print('\tvolatile uint32_t *_r = r_ptr; \\')
    print('\n'.join(map(lambda x: '\t' + x + ' \\', generate_matmul_kernel((I,J), (J,K), sa_h=4, sa_w=4))))
    print('})')
