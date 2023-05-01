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

  int i = 0;
  for(int i = 0; i < 4; i++)
    for(int j = 0; j < 4; j++)
      for(int k = 0; k < 4; k++) {
        // float-float op
        rf = f[i] * f[j];
        final_rf += rf;
        rf = f[i] * f[j] + f[k];
        final_rf += rf;
  
        // float-float op into double
        df = f[i] * f[j];
        final_df += df;
        df = f[i] * f[j] + f[k];
        final_df += df;
  
        // double-double op
        df = d[i] * d[j];
        final_df += df;
        df = d[i] * d[j] + d[k];
        final_df += df;

        // double-float op
        df = d[i] * f[j];
        final_df += df;
        df = d[i] * f[j] + d[k];
        final_df += df;

        // float-double op
        df = f[i] * d[j];
        final_df += df;
        df = f[i] * d[j] + d[k];
        final_df += df;
      }

  if(
    final_df == 10.879999968707554813818205730058252811431884765625
    && final_rf == 5.52000141143798828125)
    bp_finish(0);
 
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
        // float-float op
        rf = ff[i] * ff[j];
        final_ff += rf;
        rf = ff[i] * ff[j] + ff[k];
        final_ff += rf;
  
        // float-float op into double
        df = ff[i] * ff[j];
        final_dd += df;
        df = ff[i] * ff[j] + ff[k];
        final_dd += df;
  
        // double-double op
        df = dd[i] * dd[j];
        final_dd += df;
        df = dd[i] * dd[j] + dd[k];
        final_dd += df;

        // double-float op
        df = dd[i] * ff[j];
        final_dd += df;
        df = dd[i] * ff[j] + dd[k];
        final_dd += df;

        // float-double op
        df = ff[i] * dd[j];
        final_dd += df;
        df = ff[i] * dd[j] + dd[k];
        final_dd += df;
      }

  if(final_dd == NAN && final_ff == NAN)
    bp_finish(0);
  else bp_finish(1);
}
