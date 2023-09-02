from typing import Tuple, Union
import numpy as np
from math import ceil


Dim2 = Tuple[int,int]
Dim3 = Tuple[int,int,int]
Dim4 = Tuple[int,int,int,int]


################################################################################
#
def _pad_to_multiple( value    : int
                    , multiple : int
) -> int:
    """
        Round up the value of "value" until it is a multiple of "multiple".

        Arguments:
            value: the value to round up.
            multiple: the multiple to round up to.
    """
    mod = value % multiple
    if mod == 0:
        return value
    return value + multiple - mod


##### GEMM UTILITY FUNCS #####


################################################################################
#
def gemm_act_get_blocked_shape( raw_act_shape : Dim2
                              , array_height  : int
                              , array_width   : int
                              , accum_els     : int
) -> Dim4:
    """
        Block the 2D activation tensor for a gemm operation and block it into a
        4D tensor for use in the systolic array.

        Arguments:
            raw_act_shape: dimension of the original activation tensor
            array_height: the height of the systolic array
            array_width: the width of the systolic array
            accum_els: the number of elements per accumulator channel
    """
    I     = raw_act_shape[0]
    J     = _pad_to_multiple(raw_act_shape[1], array_height)
    I_BAR = ceil( I / ceil(accum_els / 2) )
    J_BAR = ceil( J / array_height )
    S     = ceil( I / I_BAR )
    return ( I_BAR, J_BAR, S, array_height )


################################################################################
#
def gemm_wt_get_blocked_shape( raw_wt_shape : Dim2
                             , array_height : int
                             , array_width  : int
                             , accum_els    : int
) -> Dim4:
    """
        Block the 2D weight tensor for a gemm operation and block it into a 4D
        tensor for use in the systolic array.

        Arguments:
            raw_wt_shape: dimension of the original weight tensor
            array_height: the height of the systolic array
            array_width: the width of the systolic array
            accum_els: the number of elements per accumulator channel
    """
    J     = _pad_to_multiple(raw_wt_shape[0], array_height)
    K     = _pad_to_multiple(raw_wt_shape[1], array_width)
    J_BAR = ceil( J / array_height )
    K_BAR = ceil( K / array_width )
    return ( J_BAR, K_BAR, array_height, array_width )


################################################################################
#
def gemm_out_get_blocked_shape( raw_out_shape : Dim2
                              , array_height  : int
                              , array_width   : int
                              , accum_els     : int
) -> Dim4:
    """
        -- TODO --
    """
    I     = raw_out_shape[0]
    K     = _pad_to_multiple(raw_out_shape[1], array_width)
    I_BAR = ceil( I / ceil(accum_els / 2) )
    K_BAR = ceil( K / array_width )
    S     = ceil( I / I_BAR )
    return ( I_BAR, K_BAR, S, array_width )


################################################################################
#
def gemm_act_block_tensor( raw_act_data : np.ndarray
                         , array_height : int
                         , array_width  : int
                         , accum_els    : int
) -> np.ndarray:
    """
        -- TODO --
    """
    I_BAR,J_BAR,S,H = gemm_act_get_blocked_shape(raw_act_data.shape, array_height, array_width, accum_els)

    data_padded = np.pad(raw_act_data, ((0, I_BAR * S - raw_act_data.shape[0]), (0, J_BAR * H - raw_act_data.shape[1])))

    blocked_data = np.empty((I_BAR,J_BAR,S,H), dtype=raw_act_data.dtype)
    for i in range(I_BAR):
        for j in range(J_BAR):
            blocked_data[i,j] = data_padded[ i*S:i*S+S , j*H:j*H+H ]

    return blocked_data


################################################################################
#
def gemm_wt_block_tensor( raw_wt_data  : np.ndarray
                        , array_height : int
                        , array_width  : int
                        , accum_els    : int
) -> np.ndarray:
    """
        -- TODO --
    """
    J_BAR,K_BAR,H,W = gemm_wt_get_blocked_shape(raw_wt_data.shape, array_height, array_width, accum_els)

    data_padded = np.pad(raw_wt_data, ((0, J_BAR * H - raw_wt_data.shape[0]), (0, K_BAR * W - raw_wt_data.shape[1])))

    blocked_data = np.empty((J_BAR,K_BAR,H,W), dtype=raw_wt_data.dtype)
    for j in range(J_BAR):
        for k in range(K_BAR):
            blocked_data[j,k] = data_padded[ j*H:j*H+H , k*W:k*W+W ]

    return blocked_data


################################################################################
#
def gemm_act_unblock_tensor( raw_act_shape : Dim2
                           , act_data      : np.ndarray
                           , array_height  : int
                           , array_width   : int
                           , accum_els     : int
) -> np.ndarray:
    """
        -- TODO --
    """
    I_BAR, J_BAR, S, Hsa = act_data.shape
    result = np.transpose(act_data, (0,2,1,3)).reshape(I_BAR*S, J_BAR*Hsa)
    return result[:raw_act_shape[0],:raw_act_shape[1]]


################################################################################
#
def gemm_write_back_reblock_tensor( raw_out_shape : Dim2
                                  , out_data      : np.ndarray
                                  , array_height  : int
                                  , array_width   : int
                                  , accum_els     : int
) -> np.ndarray:
    """
        Reblock output data tensor to an activation tensor as it is written
        back form the accumulators to the global buffer.
    """
    I_BAR, J_BAR, S, Hsa = gemm_act_get_blocked_shape(raw_out_shape, array_height, array_width, accum_els)
    I_BAR, K_BAR, S, Wsa = out_data.shape

    result = np.empty((I_BAR,J_BAR,S,Hsa))
    for C in range(J_BAR * Hsa):
        if C >= K_BAR * Wsa:
            break
        result[:,C//Hsa,:,C%Hsa] = out_data[:,C//Wsa,:,C%Wsa]
    return result


##### CONV UTILITY FUNCS #####


################################################################################
#
def conv_act_get_blocked_shape( raw_act_shape : Dim4
                              , array_height  : int
                              , array_width   : int
                              , accum_els     : int
) -> Dim4:
    """
        -- TODO --
    """
    N = raw_act_shape[0]
    C = _pad_to_multiple(raw_act_shape[1], array_height)
    H = raw_act_shape[2]
    W = raw_act_shape[3]
    return N,H,W,C


################################################################################
#
def conv_wt_get_blocked_shape( raw_wt_shape : Dim4
                             , array_height  : int
                             , array_width   : int
                             , accum_els     : int
) -> Dim4:
    """
        -- TODO --
    """
    K = _pad_to_multiple(raw_wt_shape[0], array_width)
    C = _pad_to_multiple(raw_wt_shape[1], array_height)
    H = raw_wt_shape[2]
    W = raw_wt_shape[3]
    return H,W,C,K


################################################################################
#
def conv_act_block_tensor( raw_act_data : np.ndarray
                         , array_height : int
                         , array_width  : int
                         , accum_els    : int
) -> np.ndarray:
    """
        -- TODO --
    """
    N,H,W,C = conv_act_get_blocked_shape(raw_act_data.shape, array_height, array_width, accum_els)
    result = np.transpose(raw_act_data, (0,2,3,1))
    return np.pad(result, ((0,0),(0,0),(0,0),(0,C - raw_act_data.shape[1]))).reshape(-1,C)


################################################################################
#
def conv_wt_block_tensor( raw_wt_data  : np.ndarray
                        , array_height : int
                        , array_width  : int
                        , accum_els    : int
) -> np.ndarray:
    """
        -- TODO --
    """
    H,W,C,K = conv_wt_get_blocked_shape(raw_wt_data.shape, array_height, array_width, accum_els)
    result = np.transpose(raw_wt_data, (2,3,1,0))
    return np.pad(result, ((0,0),(0,0),(0,C - raw_wt_data.shape[1]),(0,K - raw_wt_data.shape[0])))
