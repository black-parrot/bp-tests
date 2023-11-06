volatile uint32_t A[64] __attribute__ ((aligned (64)));
volatile uint32_t W[64] __attribute__ ((aligned (64)));
volatile uint32_t R[64] __attribute__ ((aligned (64)));
#define mm(a_ptr,w_ptr,r_ptr) ({ \
	volatile uint32_t null = 0; \
	volatile uint32_t *null_ptr = &null; \
	volatile uint32_t *_a = a_ptr; \
	volatile uint32_t *_w = w_ptr; \
	volatile uint32_t *_r = r_ptr; \
	for(int i = 0; i < 2; i++) { \
	uint64_t a_offset = i * 32; \
	_w = w_ptr - 16; \
	for(int k = 0; k < 2; k++) { \
	_a = a_ptr - 16 + i * a_offset; \
	__asm__ __volatile__(".insn i 0x0b, 0b010, %0, 64(%1)" : "+r"(_a) : "r"(_a) ); \
	__asm__ __volatile__(".insn i 0x0b, 0b000, %0, 64(%1)" : "+r"(_w) : "r"(_w) ); \
	__asm__ __volatile__(".insn i 0x0b, 0b011, %0, 64(%1)" : "+r"(_a) : "r"(_a) ); \
	__asm__ __volatile__(".insn i 0x0b, 0b001, %0, 64(%1)" : "+r"(_w) : "r"(_w) ); \
	__asm__ __volatile__(".insn i 0x0b, 0b101, x0, %0, 0"::"rK"(_r)); \
	_r += 16; \
	} \
	} \
})
