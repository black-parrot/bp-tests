#include "bp_utils.h"

/* this tests FMA with a few regression cases */

void main(uint64_t argc, char *argv[]) {
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
        rf = f[i] / f[j];
        final_rf += rf;
        rf = f[i] * f[j] + f[k];
        final_rf += rf;
        rf = f[i] / f[j] + f[k];
        final_rf += rf;
  
        // float-float op into double
        df = f[i] * f[j];
        final_df += df;
        df = f[i] / f[j];
        final_df += df;
        df = f[i] * f[j] + f[k];
        final_df += df;
        df = f[i] / f[j] + f[k];
        final_df += df;
  
        // double-double op
        df = d[i] * d[j];
        final_df += df;
        df = d[i] / d[j];
        final_df += df;
        df = d[i] * d[j] + d[k];
        final_df += df;
        df = d[i] / d[j] + d[k];
        final_df += df;

        // double-float op
        df = d[i] * f[j];
        final_df += df;
        df = d[i] / f[j];
        final_df += df;
        df = d[i] * f[j] + d[k];
        final_df += df;
        df = d[i] / f[j] + d[k];
        final_df += df;

        // float-double op
        df = (double)f[i] * d[j];
        final_df += df;
        df = (double)f[i] / d[j];
        final_df += df;
        df = (double)f[i] * d[j] + d[k];
        final_df += df;
        df = (double)f[i] / d[j] + d[k];
        final_df += df;
      }
  printf("final_rf: %f, final_df: %f", final_rf, final_df);
}
