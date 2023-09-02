#ifndef __BP_DMA_H__
#define __BP_DMA_H__

volatile uint64_t *const g_dma_device_ptr = 0x500000;

#define  BP_DMA_STA_OFFSET   0
#define  BP_DMA_CTL_OFFSET   1
#define  BP_DMA_INT_OFFSET   2
#define  BP_DMA_RBA_OFFSET   3
#define  BP_DMA_WBA_OFFSET   4
#define  BP_DMA_MS0_OFFSET   5
#define  BP_DMA_ME0_OFFSET   6
#define  BP_DMA_LS0_OFFSET   7

#define  BP_DMA_STA  (*(g_dma_device_ptr + BP_DMA_STA_OFFSET))
#define  BP_DMA_CTL  (*(g_dma_device_ptr + BP_DMA_CTL_OFFSET))
#define  BP_DMA_INT  (*(g_dma_device_ptr + BP_DMA_INT_OFFSET))
#define  BP_DMA_RBA  (*(g_dma_device_ptr + BP_DMA_RBA_OFFSET))
#define  BP_DMA_WBA  (*(g_dma_device_ptr + BP_DMA_WBA_OFFSET))
#define  BP_DMA_MS0  (*(g_dma_device_ptr + BP_DMA_MS0_OFFSET))
#define  BP_DMA_ME0  (*(g_dma_device_ptr + BP_DMA_ME0_OFFSET))
#define  BP_DMA_LS0  (*(g_dma_device_ptr + BP_DMA_LS0_OFFSET))

#define bp_dma_start() ({ \
    BP_DMA_CTL = 0b0000001; \
})

#define bp_dma_start_prefetch() ({ \
    BP_DMA_CTL = 0b1000001; \
})

#define bp_dma_wait() ({ \
    do { } while(BP_DMA_INT == 0); \
    BP_DMA_INT = 0; \
})

#define bp_dma_start_memcpy(src, dest, len) ({  \
    BP_DMA_RBA = src;                           \
    BP_DMA_WBA = dest;                          \
    BP_DMA_MS0 = 0xFFFFFFFF;                    \
    BP_DMA_ME0 = 0xFFFFFFFF;                    \
    BP_DMA_LS0 = ((len >> 4) << 16) | 0x0404;   \
    bp_dma_start();                             \
})

#define bp_dma_start_prefetch(src, len) ({ \
    BP_DMA_RBA = src;                      \
    BP_DMA_LS0 = (len << 16) | 0x0004;     \
    bp_dma_start_prefetch();               \
})

#endif // __BP_DMA_H__
