
   #include <stdint.h>
   #include <bp_utils.h>
   #include <stdio.h>

   void main(uint64_t argc, char * argv[]) {
    int fflags;
    double one = 4.0;
    double zero = 2.0;
    double res;
    double x = 3.0;
    double y = 1.5;
    double z;

    // Floating-Point (F) Extension Instructions
    __asm__ __volatile__ ("fadd.d %0, %1, %2" : "=f"(res) : "f"(one), "f"(zero));               //Adds two double-precision floating-point values.
    __asm__ __volatile__ ("fsub.d %0, %1, %2" : "=f"(res) : "f"(one), "f"(zero));               //Subtracts one double-precision floating-point value from another.
    __asm__ __volatile__ ("fmul.d %0, %1, %2" : "=f"(res) : "f"(one), "f"(zero));               //Multiplies two double-precision floating-point values.
    __asm__ __volatile__ ("fdiv.d %0, %1, %2" : "=f"(res) : "f"(one), "f"(zero));               //Divides one double-precision floating-point value by another.
    __asm__ __volatile__ ("fsqrt.d %0, %1" : "=f"(res) : "f"(one));                             //Computes the square root of a double-precision floating-point value.
    __asm__ __volatile__ ("fsgnj.d %0, %1, %2" : "=f"(res) : "f"(one), "f"(zero));              //Applies a sign operation to a double-precision floating-point value.

    // Integer Multiply/Divide (M) Extension Instructions
    int32_t a = 5;
    int32_t b = 3;
    int32_t c;

    __asm__ __volatile__ ("mul %0, %1, %2" : "=r"(c) : "r"(a), "r"(b));                         //Multiplies two integers.
    __asm__ __volatile__ ("div %0, %1, %2" : "=r"(c) : "r"(a), "r"(b));                         //Divides one integer by another and returns the quotient.
    __asm__ __volatile__ ("rem %0, %1, %2" : "=r"(c) : "r"(a), "r"(b));                         //Computes the remainder of dividing one integer by another.
    __asm__ __volatile__ ("remu %0, %1, %2" : "=r"(c) : "r"(a), "r"(b));                        //Computes the remainder of unsigned division of one integer by another.

    // Double-Precision Floating-Point (D) Extension Instructions
    __asm__ __volatile__ ("fcvt.w.d %0, %1" : "=r"(c) : "f"(one));                              //Converts a double-precision floating-point value to a signed integer.
    __asm__ __volatile__ ("fcvt.d.w %0, %1" : "=f"(res) : "r"(c));                              //Converts a signed integer to a double-precision floating-point value.
    __asm__ __volatile__ ("feq.d %0, %1, %2" : "=r"(c) : "f"(one), "f"(zero));                  //Checks if two double-precision floating-point values are equal.
    __asm__ __volatile__ ("flt.d %0, %1, %2" : "=r"(c) : "f"(one), "f"(zero));                  //Checks if one double-precision floating-point value is less than another.
    __asm__ __volatile__ ("fle.d %0, %1, %2" : "=r"(c) : "f"(one), "f"(zero));                  //Checks if one double-precision floating-point value is less than or equal to another.
    __asm__ __volatile__ ("fcvt.d.l %0, %1" : "=f"(res) : "r"(c));                              //Converts a signed integer to a double-precision floating-point value.

    // Additional Instructions
    __asm__ __volatile__ ("fmax.d %0, %1, %2" : "=f"(res) : "f"(one), "f"(zero));               //Finds the maximum value between two double-precision floating-point values.
    __asm__ __volatile__ ("fmin.d %0, %1, %2" : "=f"(res) : "f"(one), "f"(zero));               //Finds the minimum value between two double-precision floating-point values.
    __asm__ __volatile__ ("fmv.d.x %0, %1" : "=f"(res) : "r"(c));                               //Moves a double-precision floating-point value to a general-purpose register.
    __asm__ __volatile__ ("fcvt.l.d %0, %1" : "=r"(c) : "f"(one));                              //Converts a double-precision floating-point value to a signed long integer.
    __asm__ __volatile__ ("fcvt.d.l %0, %1" : "=f"(res) : "r"(c));                              //Converts a signed long integer to a double-precision floating-point value.
    __asm__ __volatile__ ("flw f0, 0(x0)");                                                     //Loads a single-precision floating-point value from memory.
    __asm__ __volatile__ ("fsw f0, 0(x0)");                                                     //Stores a single-precision floating-point value to memory.

    __asm__ __volatile__ ("fadd.s %0, %1, %2" : "=f"(res) : "f"(one), "f"(zero));               //Performs addition on two single-precision floating-point values.
    __asm__ __volatile__ ("fmul.s %0, %1, %2" : "=f"(res) : "f"(one), "f"(zero));               //Performs multiplication on two single-precision floating-point values.
    __asm__ __volatile__ ("fdiv.s %0, %1, %2" : "=f"(res) : "f"(one), "f"(zero));               //Performs division on two single-precision floating-point values.

    __asm__ __volatile__ ("fsqrt.s %0, %1" : "=f"(res) : "f"(one));                             //Computes the square root of a single-precision floating-point value.
    __asm__ __volatile__ ("fsgnj.s %0, %1, %2" : "=f"(res) : "f"(one), "f"(zero));              //Applies a sign operation to a single-precision floating-point value.
    __asm__ __volatile__ ("csrr %0, fflags" : "=r"(fflags) : );                                 //Reads the value of a Control and Status Register (CSR) into a general-purpose register.

      bp_finish(0);
   }
