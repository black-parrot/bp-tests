
include Makefile.frag

RISCV_GCC           = $(CROSS_COMPILE)gcc
RISCV_GPP           = $(CROSS_COMPILE)g++
RISCV_GCC_OPTS      = -march=rv64imafd -mabi=lp64 -mcmodel=medany -I$(BP_SDK_INCLUDE_DIR)
RISCV_LINK_OPTS     = -T$(BP_SDK_LINKER_DIR)/riscv.ld -L$(BP_SDK_LIB_DIR) -Wl,--whole-archive -lperchbm -Wl,--no-whole-archive

COPY_SOURCE = $(BP_SDK_DIR)/bp-tests/*.riscv
COPY_DEST   = $(BP_SDK_PROG_DIR)/bp-tests

.PHONY: all
.PHONY: copy

vpath %.c   ./src
vpath %.cpp ./src
vpath %.S   ./src

all: $(foreach x,$(subst -,_,$(BP_TESTS)),$(x).riscv)

%.riscv: %.c
	$(RISCV_GCC) -o $@ $^ $(RISCV_GCC_OPTS) $(RISCV_LINK_OPTS)

%.riscv: %.S
	$(RISCV_GCC) -o $@ $^ $(RISCV_GCC_OPTS) $(RISCV_LINK_OPTS)

%.riscv: %.cpp
	$(RISCV_GPP) -o $@ $^ $(RISCV_GCC_OPTS) $(RISCV_LINK_OPTS)

paging.riscv: vm_start.S paging.c
	$(RISCV_GCC) -o $@ $^ $(RISCV_GCC_OPTS) $(RISCV_LINK_OPTS) -nostartfiles

mapping.riscv: vm_start.S mapping.c
	$(RISCV_GCC) -o $@ $^ $(RISCV_GCC_OPTS) $(RISCV_LINK_OPTS) -nostartfiles

streaming_accelerator_loopback.riscv: streaming_accelerator_loopback.c vdp.c
	$(RISCV_GCC) -o $@ $^ $(RISCV_GCC_OPTS) $(RISCV_LINK_OPTS)

streaming_accelerator_vdp.riscv: streaming_accelerator_vdp.c vdp.c
	$(RISCV_GCC) -o $@ $^ $(RISCV_GCC_OPTS) $(RISCV_LINK_OPTS)

streaming_accelerator_zipline.riscv: streaming_accelerator_zipline.c vdp.c
	$(RISCV_GCC) -o $@ $^ $(RISCV_GCC_OPTS) $(RISCV_LINK_OPTS)

coherent_accelerator_vdp.riscv: coherent_accelerator_vdp.c vdp.c
	$(RISCV_GCC) -o $@ $^ $(RISCV_GCC_OPTS) $(RISCV_LINK_OPTS)

clean:
	rm -f *.riscv

copy: 
	cp $(COPY_SOURCE) $(COPY_DEST)
