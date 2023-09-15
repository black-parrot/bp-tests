#!/usr/bin/env python3
import sys
from tc_utils import *
import random
import numpy as np

random.seed(0)

skip_write = True if "-no-write" in sys.argv else False


def create_rand_array(row, col):
    return np.array([[random.randint(0, 0x7FFFFFFF) for _ in range(col)] for _ in range(row)], dtype=np.uint32)

def create_pow2_array(row, col, start_val, transpose):
    data = [[-1 for _ in range(col)] for _ in range(row)]
    for r in range(row):
        for c in range(col):
            data[r][c] = start_val << (c if transpose else r)
    return np.array(data, dtype=np.uint32)


SA_L = SA_H = SA_W = 4

for idx,s in enumerate([4, 8, 16, 32, 48, 64]):

    I = J = K = s

    I_PRIME = I // SA_L
    J_PRIME = J // SA_H
    K_PRIME = K // SA_W

    A = create_rand_array(I, J)
    W = create_rand_array(J, K)

    #A = create_pow2_array(I, J, 1, True)
    #W = create_pow2_array(J, K, 2**J, False)

    R = np.zeros((I, K), dtype=np.uint32)
    E = np.zeros((I, K), dtype=np.uint32)
    for i in range(I):
        for k in range(K):
            E[i,k] = np.bitwise_xor.reduce(A[i,:] ^ W[:,k])

    A_rs = gemm_act_block_tensor(raw_act_data=A, array_height=SA_H, array_width=SA_W, accum_els=2*SA_L)
    W_rs = gemm_wt_block_tensor (raw_wt_data=W,  array_height=SA_H, array_width=SA_W, accum_els=2*SA_L)
    R_rs = gemm_act_block_tensor(raw_act_data=R, array_height=SA_H, array_width=SA_W, accum_els=2*SA_L)
    E_rs = gemm_act_block_tensor(raw_act_data=E, array_height=SA_H, array_width=SA_W, accum_els=2*SA_L)

    R_blk_rs = np.zeros((J_PRIME, I_PRIME, K_PRIME, SA_L, SA_W), dtype=np.uint32)
    E_blk_rs = np.zeros((J_PRIME, I_PRIME, K_PRIME, SA_L, SA_W), dtype=np.uint32)

    for kp in range(K_PRIME):
        for jp in range(J_PRIME):
            for ip in range(I_PRIME):
                # tensor-core
                for i_s in range(SA_L):
                    for j_s in range(SA_H):
                        for k_s in range(SA_W):
                            E_blk_rs[jp,ip,kp,i_s,k_s] ^= A_rs[ip,jp,i_s,j_s] ^ W_rs[jp,kp,j_s,k_s]

    attr = "__attribute__ ((aligned (64))) __attribute__ ((section (\".data\")))"

    tile_row_major = False
    block_row_major = True

    if not tile_row_major:
        A_rs = A_rs.transpose((1, 0, 2, 3))
        W_rs = W_rs.transpose((1, 0, 2, 3))
        R_rs = R_rs.transpose((1, 0, 2, 3))
        E_rs = E_rs.transpose((1, 0, 2, 3))
        R_blk_rs = R_blk_rs.transpose((0, 2, 1, 3, 4))
        E_blk_rs = E_blk_rs.transpose((0, 2, 1, 3, 4))

    A_rs_str     = ",".join(map(str,A_rs.flatten().tolist()))
    W_rs_str     = ",".join(map(str,W_rs.flatten().tolist()))
    R_rs_str     = ",".join(map(str,R_rs.flatten().tolist()))
    E_rs_str     = ",".join(map(str,E_rs.flatten().tolist()))
    E_blk_rs_str = ",".join(map(str,E_blk_rs.flatten().tolist()))
    R_blk_rs_str = ",".join(map(str,R_blk_rs.flatten().tolist()))

    print(A_rs.shape)
    print(W_rs.shape)
    print(R_rs.shape)
    print(R_blk_rs.shape)

    if not skip_write:
        with open("tc_%d.h" % idx, "w") as f:
            fprint = lambda x: print(x, file=f)
            fprint(f"#define I {I}")
            fprint(f"#define J {J}")
            fprint(f"#define K {K}")
            fprint("volatile uint32_t A[%d] %s = {%s};"     % (A_rs.size, attr, A_rs_str))
            fprint("volatile uint32_t W[%d] %s = {%s};"     % (W_rs.size, attr, W_rs_str))
            fprint("volatile uint32_t R[%d] %s = {%s};"     % (R_rs.size, attr, R_rs_str))
            fprint("volatile uint32_t E[%d] %s = {%s};"     % (E_rs.size, attr, E_rs_str))
            fprint("volatile uint32_t E_BLK[%d] %s = {%s};" % (E_blk_rs.size, attr, E_blk_rs_str))
            fprint("volatile uint32_t R_BLK[%d] %s = {%s};" % (R_blk_rs.size, attr, R_blk_rs_str))
