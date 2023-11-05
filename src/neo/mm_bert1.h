#define mm(a_ptr,w_ptr,r_ptr) ({ \
	volatile uint32_t null = 0; \
	volatile uint32_t *null_ptr = &null; \
	volatile uint32_t *_a = a_ptr; \
	volatile uint32_t *_w = w_ptr; \
	volatile uint32_t *_r = r_ptr; \
	__asm__ __volatile__(".insn i 0x0b, 0b010, %0, 0(%1)" : "+r"(_a) : "r"(_a) ); \
	__asm__ __volatile__(".insn i 0x0b, 0b000, %0, 0(%1)" : "+r"(_w) : "r"(_w) ); \
	__asm__ __volatile__(".insn i 0x0b, 0b011, %0, 64(%1)" : "+r"(_a) : "r"(_a) ); \
	__asm__ __volatile__(".insn i 0x0b, 0b001, %0, 64(%1)" : "+r"(_w) : "r"(_w) ); \
	__asm__ __volatile__(".insn i 0x0b, 0b101, x0, %0, 0"::"rK"(((uint64_t)_r + 0))); \
		_a = a_ptr + (i * 128);\ \
	__asm__ __volatile__(".insn i 0x0b, 0b010, %0, 0(%1)" : "+r"(_a) : "r"(_a) ); \
	__asm__ __volatile__(".insn i 0x0b, 0b000, %0, 0(%1)" : "+r"(_w) : "r"(_w) ); \
	__asm__ __volatile__(".insn i 0x0b, 0b011, %0, 64(%1)" : "+r"(_a) : "r"(_a) ); \
	__asm__ __volatile__(".insn i 0x0b, 0b001, %0, 64(%1)" : "+r"(_w) : "r"(_w) ); \
	__asm__ __volatile__(".insn i 0x0b, 0b101, x0, %0, 0"::"rK"(((uint64_t)_r + 64))); \
		_a = a_ptr + (i * 128);\ \
		_w = w_ptr;\ \
	__asm__ __volatile__(".insn i 0x0b, 0b010, %0, 0(%1)" : "+r"(_a) : "r"(_a) ); \
	__asm__ __volatile__(".insn i 0x0b, 0b000, %0, 0(%1)" : "+r"(_w) : "r"(_w) ); \
	__asm__ __volatile__(".insn i 0x0b, 0b011, %0, 64(%1)" : "+r"(_a) : "r"(_a) ); \
	__asm__ __volatile__(".insn i 0x0b, 0b001, %0, 64(%1)" : "+r"(_w) : "r"(_w) ); \
	__asm__ __volatile__(".insn i 0x0b, 0b101, x0, %0, 0"::"rK"(((uint64_t)_r + 128))); \
		_a = a_ptr + (i * 128);\ \
	__asm__ __volatile__(".insn i 0x0b, 0b010, %0, 0(%1)" : "+r"(_a) : "r"(_a) ); \
	__asm__ __volatile__(".insn i 0x0b, 0b000, %0, 0(%1)" : "+r"(_w) : "r"(_w) ); \
	__asm__ __volatile__(".insn i 0x0b, 0b011, %0, 64(%1)" : "+r"(_a) : "r"(_a) ); \
	__asm__ __volatile__(".insn i 0x0b, 0b001, %0, 64(%1)" : "+r"(_w) : "r"(_w) ); \
	__asm__ __volatile__(".insn i 0x0b, 0b101, x0, %0, 0"::"rK"(((uint64_t)_r + 192))); \
		_a = a_ptr + (i * 128);\ \
		_w = w_ptr;\ \
})
