#!/usr/bin/env python3
from tc_utils import *
import random
import numpy as np
from math import ceil

random.seed(0)

I = 16
J = 768
K = 768
SA_L = SA_H = SA_W = 4

I_PRIME = I // SA_L
J_PRIME = J // SA_H
K_PRIME = K // SA_W

A = np.array([[random.randint(0, 0x7FFFFFFF) for _ in range(J)] for _ in range(I)], dtype=np.uint32)
W = np.array([[random.randint(0, 0x7FFFFFFF) for _ in range(K)] for _ in range(J)], dtype=np.uint32)
#A = np.arange(I * J, dtype=np.uint32).reshape(I, J)
#W = np.arange(J * K, dtype=np.uint32).reshape(J, K)

full_result = np.zeros((I, K), dtype=np.uint32)
#for i in range(I):
#    for k in range(K):
#        for j in range(J):
#            full_result[i][k] ^= A[i][j] ^ W[j][k]

A_rs = gemm_act_block_tensor(raw_act_data=A , array_height=SA_H , array_width=SA_W , accum_els=2*SA_L)
W_rs = gemm_wt_block_tensor(raw_wt_data=W, array_height=SA_H, array_width=SA_W, accum_els=2*SA_L)


block_result = np.zeros((I_PRIME, K_PRIME, SA_L, SA_W), dtype=np.uint32)

#for kp in range(K_PRIME): # sub-K
#    for jp in range(J_PRIME): # sub-J
#        for ip in range(I_PRIME):
#            A_blk = A_rs[ip,jp,:,:]
#            W_blk = W_rs[jp,kp,:,:]
#            R_blk = np.zeros((SA_L, SA_W), dtype=np.uint32)
#            for i_s in range(SA_L): # sub-I
#                for j_s in range(SA_H): # sub-J
#                    for k_s in range(SA_W): # sub-K
#                        R_blk[i_s][k_s] ^= A_blk[i_s][j_s] ^ W_blk[j_s][k_s]
#            block_result[ip,kp,:,:] ^= R_blk

unblock_result = gemm_act_unblock_tensor(raw_act_shape=full_result.shape, act_data=block_result, array_height=SA_H, array_width=SA_W, accum_els=2*SA_L)

assert np.array_equal(full_result, unblock_result)


#############################

attr = "__attribute__ ((aligned (64))) __attribute__ ((section (\".data\")))"

# USE ME FOR REAL DATA
#A_rs_str = ",".join(map(str,A_rs.flatten().tolist()))
#W_rs_str = ",".join(map(str,W_rs.flatten().tolist()))
#R_rs_str = ",".join(map(str,block_result.flatten().tolist()))

# USE ME FOR ZEROs DATA
A_rs_str = "0"
W_rs_str = "0"
R_rs_str = "0"

print(f"#ifndef __TC_DATA_H__")
print(f"#define __TC_DATA_H__")
print(f"")
print(f"#define I {I}")
print(f"#define J {J}")
print(f"#define K {K}")
print(f"")
print(f"#define I_PRIME {I_PRIME}")
print(f"#define J_PRIME {J_PRIME}")
print(f"#define K_PRIME {K_PRIME}")
print(f"")
print("volatile uint32_t A[%d] %s = {%s};" % (I * J, attr, A_rs_str))
print("volatile uint32_t W[%d] %s = {%s};" % (J * K, attr, W_rs_str))
print("volatile uint32_t expected[%d] %s = {%s};" % (I * K, attr, R_rs_str))
print("volatile uint32_t result[%d] %s;" % (I * K, attr, ))
print(f"")
print(f"#endif // __TC_DATA_H__")
