#include <stdio.h>
#include <stdint.h>
#include "bp_utils.h"
#include "bp_asm.h"

#include "neo/neo_resnet50_2.h"
#include "neo.h"

int main(int argc, char** argv)
{
    cbo_flush_all();
    matmul_os(A, W, R);
    bp_printf("%s\n", "Test complete!");
    return 0;
}
