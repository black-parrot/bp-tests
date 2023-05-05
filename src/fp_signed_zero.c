#include "bp_utils.h"
#include "math.h"

/* tests FMA with a few regression cases.
  use with Dromajo cosimulation for finding mismatches, if any.
  the assertions do not guarantee functional correctness */

void main(uint64_t argc, char *argv[]) {
  // this part doesn't wash out the accumulations
  float  f[4] = {0.0, -0.0, 0.5, -0.2};
  double d[4] = {0.0, -0.0, 0.4, -0.3};
  float rf, final_rf;
  double df, final_df;
  enum rounding_modes {RNE, RTZ=0x20, RDN=0x40, RUP=0x60, RMM=0x80, DYN=0xe0} RM;
  for(int rm = 0; rm < 2; rm++) { // rounding_modes
    switch (rm) {
      // RNE default
      case 0:
        asm volatile ( 
            "li    a5, %[rm];"
            "csrrw a5, fcsr, a5;"
            :: [rm] "i" (RTZ)
            );
        break;
      case 1:
        asm volatile ( 
            "li    a5, %[rm];"
            "csrrw a5, fcsr, a5;"
            :: [rm] "i" (RDN)
            );
        break;
//      case 2:
//        asm volatile ( 
//            "li    a5, %[rm];"
//            "csrrw a5, fcsr, a5;"
//            :: [rm] "i" (RUP)
//            );
//        break;
//      case 3:
//        asm volatile ( 
//            "li    a5, %[rm];"
//            "csrrw a5, fcsr, a5;"
//            :: [rm] "i" (RMM)
//            );
//        break;
//      case 4:
//        asm volatile ( 
//            "li    a5, %[rm];"
//            "csrrw a5, fcsr, a5;"
//            :: [rm] "i" (DYN)
//            );
//        break;
      default: 
        asm volatile ( 
            "li    a5, %[rm];"
            "csrrw a5, fcsr, a5;"
            :: [rm] "i" (RNE)
            );
        break;
    }
    for(int i = 0; i < 4; i++)
      for(int j = 0; j < 4; j++) {
        for(int prec = 0; prec<4; prec++) {
          // float <- float|double * float|double
          asm volatile (
             "fmul.s %[rf], %[a], %[b];"
             : [rf] "=f" (rf)
             : [a]  "f" (prec&1 ? f[i] : d[i]) ,
               [b]  "f" (prec&2 ? f[j] : d[j])
            );
          final_rf += rf;
        }
        // float <- float|double * float|double + float|double
        for(int k = 0; k < 4; k++)
          for(int prec = 0; prec<8; prec++) {
            asm volatile (
               "fmadd.s %[rf], %[a], %[b], %[c];"
               : [rf] "=f" (rf)
               : [a]  "f" (prec&1 ? f[i] : d[i]) ,
                 [b]  "f" (prec&2 ? f[j] : d[j]) ,
                 [c]  "f" (prec&4 ? f[k] : d[k])
              );
            final_rf += rf;
          } 
        // double <- float|double * float|double
        for(int prec = 0; prec<4; prec++) {
          asm volatile (
             "fmul.d %[df], %[a], %[b];"
             : [df] "=f" (df)
             : [a]  "f" (prec&1 ? f[i] : d[i]) ,
               [b]  "f" (prec&2 ? f[j] : d[j])
            );
          final_df += df;
        }
        for(int k = 0; k < 4; k++)
          for(int prec = 0; prec<8; prec++) {
            // double <- float|double * float|double + float|double
            asm volatile (
               "fmadd.d %[df], %[a], %[b], %[c];"
               : [df] "=f" (df)
               : [a]  "f" (prec&1 ? f[i] : d[i]) ,
                 [b]  "f" (prec&2 ? f[j] : d[j]) ,
                 [c]  "f" (prec&4 ? f[k] : d[k])
              );
            final_df += df;
          }
       } // j
  } // rounding modes

  if(
    final_df != 54.079999575613925344441668130457401275634765625
    && final_rf != NAN)
    bp_finish(1);
 
  /* because of crossing NaNs below, the accumulation is washed out (=NaN)
    final assertions do not guarantee correct execution,
    rely on cosimulation with a golden reference (Dromajo) */

  float  ff[7] = {0.0, -0.0, 0.5, -0.2, INFINITY, -INFINITY, NAN};
  double dd[7] = {0.0, -0.0, 0.4, -0.3, INFINITY, -INFINITY, NAN};
  float  final_ff;
  double final_dd;
  for(int i = 0; i < 7; i++)
    for(int j = 0; j < 7; j++)
      for(int k = 0; k < 7; k++) {
        // float <- float|double * float|double
        for(int prec = 0; prec<4; prec++) {
          asm volatile (
             "fmul.s %[rf], %[a], %[b];"
             : [rf] "=f" (rf)
             : [a]  "f" (prec&1 ? f[i] : d[i]) ,
               [b]  "f" (prec&2 ? f[j] : d[j])
            );
          final_rf += rf;
        }
        // float <- float|double * float|double + float|double
        for(int prec = 0; prec<8; prec++) {
          asm volatile (
             "fmadd.s %[rf], %[a], %[b], %[c];"
             : [rf] "=f" (rf)
             : [a]  "f" (prec&1 ? f[i] : d[i]) ,
               [b]  "f" (prec&2 ? f[j] : d[j]) ,
               [c]  "f" (prec&4 ? f[k] : d[k])
            );
          final_rf += rf;
        } 
        // double <- float|double * float|double
        for(int prec = 0; prec<4; prec++) {
          asm volatile (
             "fmul.d %[df], %[a], %[b];"
             : [df] "=f" (df)
             : [a]  "f" (prec&1 ? f[i] : d[i]) ,
               [b]  "f" (prec&2 ? f[j] : d[j])
            );
          final_df += df;
        }
        // double <- float|double * float|double + float|double
        for(int prec = 0; prec<8; prec++) {
          asm volatile (
             "fmadd.d %[df], %[a], %[b], %[c];"
             : [df] "=f" (df)
             : [a]  "f" (prec&1 ? f[i] : d[i]) ,
               [b]  "f" (prec&2 ? f[j] : d[j]) ,
               [c]  "f" (prec&4 ? f[k] : d[k])
            );
          final_df += df;
        }
      }

  if(final_dd == NAN && final_ff == NAN)
    bp_finish(0);
  else bp_finish(1);
}
