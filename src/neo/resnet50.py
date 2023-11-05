import torch
from torch.nn import functional as F


def block_conv_weights(arr, sa_h=4, sa_w=4):
    K,C,H,W = arr.shape
    K_PAD, C_PAD = (
        round_up_to_multiple(K, sa_w),
        round_up_to_multiple(C, sa_h),
    )
    C_BLOCKS = C_PAD // sa_h
    K_BLOCKS = K_PAD // sa_w

    arr_padded = torch.zeros((H,W,C_PAD,K_PAD))
    for h in range(H):
        for w in range(W):
            for c in range(C):
                for k in range(K):
                    arr_padded[h,w,c,k] = arr[k,c,h,w]

    result = torch.zeros((H, W, C_BLOCKS, K_BLOCKS, sa_h, sa_w))
    for h in range(H):
        for w in range(W):
            for c in range(C_BLOCKS):
                for k in range(K_BLOCKS):
                    result[h,w,c,k] = arr_padded[h, w, c*sa_h:(c+1)*sa_h, k*sa_w:(k+1)*sa_w]

    return result


def block_conv_activations(arr, sa_h=4, sa_w=4):
    N,C,H,W = arr.shape
    C_PAD = round_up_to_multiple(C, sa_h)
    C_BLOCKS = C_PAD // sa_h

    arr_padded = torch.zeros((N,H,W,C_PAD))
    for n in range(N):
        for h in range(H):
            for w in range(W):
                for c in range(C):
                        arr_padded[n,h,w,c] = arr[n,c,h,w]


    result = torch.zeros((N,H,W,C_BLOCKS,sa_h))
    for h in range(H):
        for w in range(W):
            for c in range(C_BLOCKS):
                for k in range(K_BLOCKS):
                    result[h,w,c,k] = arr_padded[h, w, c*sa_h:(c+1)*sa_h, k*sa_w:(k+1)*sa_w]

    return result


def round_up_to_multiple(val, multiple):
    mod = val % multiple
    if mod == 0:
        return val
    return val + (multiple - mod)





#A = torch.rand((1,3,224,224))
#W = torch.rand((16,3,7,7))


i = torch.arange(1*4*4*4).reshape(1,4,4,4) ;# N C H W
j = torch.arange(1*4*4*4).reshape(1,4,4,4) ;# K C H W
k = F.conv2d(i,j)
print(k)

i_block = block_conv_activations(i) ;# N H W C
j_block = block_conv_weights(j)     ;# H W C K
