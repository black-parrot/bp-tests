#!/usr/bin/env python3
from tc_utils import *
import random
import numpy as np

random.seed(0)

def create_rand_array(row, col):
    return np.array([[random.randint(0, 0x7FFFFFFF) for _ in range(col)] for _ in range(row)], dtype=np.uint32)

def create_pow2_array(row, col, start_val, transpose):
    data = [[-1 for _ in range(col)] for _ in range(row)]
    for r in range(row):
        for c in range(col):
            data[r][c] = start_val << (c if transpose else r)
    return np.array(data, dtype=np.uint32)





I = 40
J = 40
K = 40

SA_L = SA_H = SA_W = 4

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

A_rs_str     = ",".join(map(str,A_rs.flatten().tolist()))
W_rs_str     = ",".join(map(str,W_rs.flatten().tolist()))
R_rs_str     = ",".join(map(str,R_rs.flatten().tolist()))
E_rs_str     = ",".join(map(str,E_rs.flatten().tolist()))
E_blk_rs_str = ",".join(map(str,E_blk_rs.flatten().tolist()))
R_blk_rs_str = ",".join(map(str,R_blk_rs.flatten().tolist()))

#A_rs_str = "0"
#W_rs_str = "0"
#R_rs_str = "0"
#E_rs_str = "0"

print(f"#define I {I}")
print(f"#define J {J}")
print(f"#define K {K}")
#print(f"#define I_PRIME {I_PRIME}")
#print(f"#define J_PRIME {J_PRIME}")
#print(f"#define K_PRIME {K_PRIME}")
print("volatile uint32_t A[%d] %s = {%s};"     % (A_rs.size, attr, A_rs_str))
print("volatile uint32_t W[%d] %s = {%s};"     % (W_rs.size, attr, W_rs_str))
print("volatile uint32_t R[%d] %s = {%s};"     % (R_rs.size, attr, R_rs_str))
print("volatile uint32_t E[%d] %s = {%s};"     % (E_rs.size, attr, E_rs_str))
print("volatile uint32_t E_BLK[%d] %s = {%s};" % (E_blk_rs.size, attr, E_blk_rs_str))
print("volatile uint32_t R_BLK[%d] %s = {%s};" % (R_blk_rs.size, attr, R_blk_rs_str))
