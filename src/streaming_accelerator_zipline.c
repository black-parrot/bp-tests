#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "bp_utils.h"
#include "vdp.h"

#define TO_HEX(i) (i <= 9 ? '0' + i : 'A' - 10 + i)

uint64_t main(uint64_t argc, char * argv[]) {

  struct Zipline_CSR zipline_csr;

  uint64_t core_id;
  __asm__ volatile("csrr %0, mhartid": "=r"(core_id): :);   
  uint64_t resp_data [20];
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  struct dma_cfg dma_tlv_header [5];
  uint64_t tlv_headers [14];
  //struct zipline_tlv tlv_headers [14];

  //rqe
  tlv_headers[0] = 0x400000000a000400;
  tlv_headers[1] = 0x0000000000000000;
  dma_tlv_header[0].data_ptr = tlv_headers;
  dma_tlv_header[0].length = 2;
  dma_tlv_header[0].type = 0;

  
  //cmd
  tlv_headers[2] = 0x800000000a000601;
  tlv_headers[3] = 0x000000000780b000;
  tlv_headers[4] = 0x3a00000000200654;
  dma_tlv_header[1].data_ptr = tlv_headers+2;
  dma_tlv_header[1].length = 3;
  dma_tlv_header[1].type = 1;
  

  //frmd 
  tlv_headers[5] = 0xc00000000a00020b;
  dma_tlv_header[2].data_ptr = tlv_headers+5;
  dma_tlv_header[2].length = 1;
  dma_tlv_header[2].type = 2;
  
  //data
  tlv_headers[6]  = 0xa00000000a000005;
  tlv_headers[7]  = 0x0706050403020100;
  tlv_headers[8]  = 0x0f0e0d0c0b0a0908;
  tlv_headers[9]  = 0x1716151413121110;
  tlv_headers[10] = 0x1f1e1d1c1b1a1918;
  tlv_headers[11] = 0x2726252423222120;
  dma_tlv_header[3].data_ptr = tlv_headers+6;
  dma_tlv_header[3].length = 6;
  dma_tlv_header[3].type = 3;
  

  //cqe
  tlv_headers[12] = 0x800000000a000409;
  tlv_headers[13] = 0x0000000000000000;
  dma_tlv_header[4].data_ptr = tlv_headers+12;
  dma_tlv_header[4].length = 2;
  dma_tlv_header[4].type = 4;
 
  
  if (core_id == 0) {
    //type:1, streaming
    zipline_csr.input_ptr = dma_tlv_header;
    zipline_csr.resp_ptr =  (uint64_t *) &resp_data;
    bp_call_zipline_accelerator(1, zipline_csr, 5);
    
    int i=0, j=0;
    for(j=0; j < 10; j++){
      for(i = 0; i < 16;++i){      
        bp_cprint(TO_HEX((uint8_t)((resp_data[j]>>i*4) & 0x0F)));
      }
      bp_cprint(10);
    }

  }


  bp_finish(0);
  
  return 0;
}
