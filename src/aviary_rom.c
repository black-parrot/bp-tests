
#include "bp_utils.h"
#include "aviary.h"

// This test verifies that we can read from the aviary ROM, provided by the
//   BlackParrot environment. It is hardcoded to the default unicore configuration
//   values, so it will need to change if these configuration values change
int main(uint64_t argc, char * argv[]) {
    if (bp_param_get(PARAM_MULTICORE) != 0) return -1;
    if (bp_param_get(PARAM_CC_X_DIM)  != 1) return -1;
    if (bp_param_get(PARAM_CC_Y_DIM)  != 1) return -1;
    // Multicore has an I/O complex, unicore does not
    if (bp_param_get(PARAM_IC_Y_DIM)  != bp_param_get(PARAM_MULTICORE)) return -1;
    if (bp_param_get(PARAM_MC_Y_DIM)  != 0) return -1;
    if (bp_param_get(PARAM_CAC_X_DIM) != 0) return -1;
    if (bp_param_get(PARAM_SAC_X_DIM) != 0) return -1;
    if (bp_param_get(PARAM_CACC_TYPE) != 0) return -1;
    if (bp_param_get(PARAM_SACC_TYPE) != 0) return -1;

    if (bp_param_get(PARAM_NUM_CCE) != 1) return -1;
    if (bp_param_get(PARAM_NUM_LCE) != 2) return -1;

    if (bp_param_get(PARAM_VADDR_WIDTH) != 39) return -1;
    if (bp_param_get(PARAM_PADDR_WIDTH) != 40) return -1;
    if (bp_param_get(PARAM_DADDR_WIDTH) != 33) return -1;
    if (bp_param_get(PARAM_CADDR_WIDTH) != 32) return -1;
    if (bp_param_get(PARAM_ASID_WIDTH)  != 1 ) return -1;

    if (bp_param_get(PARAM_BOOT_PC)       != 0x80000000) return -1;
    if (bp_param_get(PARAM_BOOT_IN_DEBUG) != 0         ) return -1;

    if (bp_param_get(PARAM_BRANCH_METADATA_FWD_WIDTH) != 36) return -1;
    if (bp_param_get(PARAM_BTB_TAG_WIDTH)             != 9 ) return -1;
    if (bp_param_get(PARAM_BTB_IDX_WIDTH)             != 6 ) return -1;
    if (bp_param_get(PARAM_BHT_IDX_WIDTH)             != 8 ) return -1;
    if (bp_param_get(PARAM_BHT_ROW_ELS)               != 2 ) return -1;
    if (bp_param_get(PARAM_GHIST_WIDTH)               != 2 ) return -1;

    if (bp_param_get(PARAM_ITLB_ELS_4K) != 8) return -1;
    if (bp_param_get(PARAM_DTLB_ELS_4K) != 8) return -1;
    if (bp_param_get(PARAM_ITLB_ELS_1G) != 0) return -1;
    if (bp_param_get(PARAM_DTLB_ELS_1G) != 0) return -1;

    if (bp_param_get(PARAM_LR_SC)                != 1) return -1;
    if (bp_param_get(PARAM_AMO_SWAP)             != 0) return -1;
    if (bp_param_get(PARAM_AMO_FETCH_LOGIC)      != 0) return -1;
    if (bp_param_get(PARAM_AMO_FETCH_ARITHMETIC) != 0) return -1;

    if (bp_param_get(PARAM_L1_WRITETHROUGH)    != 0  ) return -1;
    if (bp_param_get(PARAM_L1_COHERENT)        != 0  ) return -1;
    if (bp_param_get(PARAM_DCACHE_SETS)        != 64 ) return -1;
    if (bp_param_get(PARAM_DCACHE_ASSOC)       != 8  ) return -1;
    if (bp_param_get(PARAM_DCACHE_BLOCK_WIDTH) != 512) return -1;
    if (bp_param_get(PARAM_DCACHE_FILL_WIDTH)  != 64) return -1;
    if (bp_param_get(PARAM_ICACHE_SETS)        != 64 ) return -1;
    if (bp_param_get(PARAM_ICACHE_ASSOC)       != 8  ) return -1;
    if (bp_param_get(PARAM_ICACHE_BLOCK_WIDTH) != 512) return -1;
    if (bp_param_get(PARAM_ICACHE_FILL_WIDTH)  != 64) return -1;
    if (bp_param_get(PARAM_ACACHE_SETS)        != 64 ) return -1;
    if (bp_param_get(PARAM_ACACHE_ASSOC)       != 8  ) return -1;
    if (bp_param_get(PARAM_ACACHE_BLOCK_WIDTH) != 512) return -1;
    if (bp_param_get(PARAM_ACACHE_FILL_WIDTH)  != 64) return -1;

    if (bp_param_get(PARAM_L2_EN)               != 1  ) return -1;
    if (bp_param_get(PARAM_L2_DATA_WIDTH)       != 64 ) return -1;
    if (bp_param_get(PARAM_L2_SETS)             != 128) return -1;
    if (bp_param_get(PARAM_L2_ASSOC)            != 8  ) return -1;
    if (bp_param_get(PARAM_L2_BLOCK_WIDTH)      != 512) return -1;
    if (bp_param_get(PARAM_L2_FILL_WIDTH)       != 64 ) return -1;
    if (bp_param_get(PARAM_L2_OUTSTANDING_REQS) != 8  ) return -1;

    if (bp_param_get(PARAM_FE_QUEUE_FIFO_ELS) != 8) return -1;
    if (bp_param_get(PARAM_FE_CMD_FIFO_ELS)   != 4) return -1;

    if (bp_param_get(PARAM_ASYNC_COH_CLK)       != 0  ) return -1;
    if (bp_param_get(PARAM_COH_NOC_FLIT_WIDTH)  != 128) return -1;
    if (bp_param_get(PARAM_COH_NOC_CID_WIDTH)   != 2  ) return -1;
    if (bp_param_get(PARAM_COH_NOC_LEN_WIDTH)   != 3  ) return -1;
    if (bp_param_get(PARAM_COH_NOC_MAX_CREDITS) != 8  ) return -1;

    if (bp_param_get(PARAM_ASYNC_MEM_CLK)       != 0  ) return -1;
    if (bp_param_get(PARAM_MEM_NOC_FLIT_WIDTH)  != 64 ) return -1;
    if (bp_param_get(PARAM_MEM_NOC_CID_WIDTH)   != 2  ) return -1;
    if (bp_param_get(PARAM_MEM_NOC_LEN_WIDTH)   != 4  ) return -1;
    if (bp_param_get(PARAM_MEM_NOC_MAX_CREDITS) != 8  ) return -1;

    if (bp_param_get(PARAM_ASYNC_IO_CLK)       != 0  ) return -1;
    if (bp_param_get(PARAM_IO_NOC_FLIT_WIDTH)  != 64 ) return -1;
    if (bp_param_get(PARAM_IO_NOC_CID_WIDTH)   != 2  ) return -1;
    if (bp_param_get(PARAM_IO_NOC_DID_WIDTH)   != 3  ) return -1;
    if (bp_param_get(PARAM_IO_NOC_LEN_WIDTH)   != 4  ) return -1;
    if (bp_param_get(PARAM_IO_NOC_MAX_CREDITS) != 16 ) return -1;

    return 0;
}
