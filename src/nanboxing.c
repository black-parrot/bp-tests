
#include <bp_utils.h>
#include <stdint.h>

int main(uint64_t argc, char * argv[]) {
    long x0 = 0x12345678;
    long x1 = 0x12345678;
    long x2 = 0x12345678;

    __asm__ __volatile__ ("fmv.d.x f0, %0" : : "r"(x0));
    __asm__ __volatile__ ("fmv.d.x f1, %0" : : "r"(x1));
    __asm__ __volatile__ ("fmv.d.x f2, %0" : : "r"(x2));
    __asm__ __volatile__ ("fmadd.s f3, f0, f1, f2");
    __asm__ __volatile__ ("fadd.s f3, f0, f1");
    __asm__ __volatile__ ("fdiv.s f3, f0, f1");
    __asm__ __volatile__ ("fsgnj.s f3, f0, f1");

    return 0;
}

