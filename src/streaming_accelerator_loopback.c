#include <stdint.h>
#include "bp_utils.h"
#include "vdp.h"

#define TO_HEX(i) (i <= 9 ? '0' + i : 'A' - 10 + i)

uint64_t main(uint64_t argc, char * argv[]) {
  
  uint64_t core_id;
  __asm__ volatile("csrr %0, mhartid": "=r"(core_id): :);   

  if (core_id == 0) {
  
    uint64_t input_array [10];
    uint64_t output_array [10];
    uint64_t wrod_count = 10;
    int i;
    for(i = 0; i < wrod_count; i++){
      input_array[i] = i*16;
      output_array[i] = 0;
    }

    uint64_t *input_ptr, *resp_ptr;
    input_ptr = (uint64_t *) &input_array;
    resp_ptr = (uint64_t *) &output_array;
  
    uint64_t accel_write_cnt = 0;
    accel_write_cnt = bp_call_loopback_accelerator (input_ptr, resp_ptr, wrod_count);

    int err_cnt = 0;
    for(i = 0; i < wrod_count; i++){
      if (input_array[i] != output_array[i])
        err_cnt++;
      else 
        for(int j = 15; j >= 0; j--)
          bp_cprint(TO_HEX((uint8_t)((output_array[i]>>j*4) & 0x0F)));
      bp_cprint('\n');
    }

    char errcnt[7] = "ERRCNT:";
    for(i = 0; i < 7; i++)
      bp_cprint(errcnt[i]);
    for(i = 15; i >= 0; i--){      
      bp_cprint(TO_HEX((uint8_t)((err_cnt>>i*4) & 0x0F)));
    }

    bp_cprint('\n');
    char wrcnt[6] = "WRCNT:";
    for(i = 0; i < 6; i++)
      bp_cprint(wrcnt[i]);
    for(i = 15; i >= 0; i--){      
      bp_cprint(TO_HEX((uint8_t)((accel_write_cnt>>i*4) & 0x0F)));
    }
  } 
    
  if ((BP_CONFIG > 3) && (core_id == 3)){
  
    uint8_t vlen;
    uint64_t input_array_a [8];
    uint64_t input_array_b [8];
    uint64_t resp_data = 0;
    struct VDP_CSR vdp_csr;

    vlen = 2;
    int i;
    for(i = 0; i < vlen; ++i){
      input_array_a[i] = (i+1)*16;
      input_array_b[i] = (i+1)*32;
    }

    vdp_csr.input_a_ptr = (uint64_t *) &input_array_a;
    vdp_csr.input_b_ptr = (uint64_t *) &input_array_b;
    vdp_csr.input_length = vlen;
    vdp_csr.resp_ptr =  (uint64_t *) &resp_data;

    //type:0, coherent
    bp_call_vector_dot_product_accelerator(0, vdp_csr);

    for(i = 0; i < 16;++i){
      bp_cprint(TO_HEX((uint8_t)((resp_data>>i*4) & 0x0F)));
    }
  }

  bp_finish(0);
  
  return 0;
}
