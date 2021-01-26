
include Makefile.frag

RISCV_GCC       = $(CROSS_COMPILE)gcc 
RISCV_GCC_OPTS  = -march=rv64imafd -mabi=lp64 -mcmodel=medany -I $(BP_TEST_DIR)/include
RISCV_LINK_OPTS = -T $(BP_TEST_DIR)/src/perch/riscv.ld -L$(BP_TEST_DIR)/lib -static -nostartfiles -lperch

.PHONY: all bp-demo-riscv

vpath %.c ./src
vpath %.S ./src

all: bp-demo-riscv

bp-demo-riscv: $(foreach x,$(subst -,_,$(BP_DEMOS)),$(x).riscv)

%.riscv: %.c
	$(RISCV_GCC) -o $@ $^ $(RISCV_GCC_OPTS) $(RISCV_LINK_OPTS)

%.riscv: %.S
	$(RISCV_GCC) -o $@ $^ $(RISCV_GCC_OPTS) $(RISCV_LINK_OPTS)

paging.riscv: vm_start.S paging.c
	$(RISCV_GCC) -o $@ $^ $(RISCV_GCC_OPTS) $(RISCV_LINK_OPTS)

mapping.riscv: vm_start.S mapping.c
	$(RISCV_GCC) -o $@ $^ $(RISCV_GCC_OPTS) $(RISCV_LINK_OPTS)

mc_sanity_%.riscv: mc_sanity.c
	$(RISCV_GCC) -DNUM_CORES=$(notdir $*) -o $@ $^ $(RISCV_GCC_OPTS) $(RISCV_LINK_OPTS)

mc_template_%.riscv: mc_template.c
	$(RISCV_GCC) -DNUM_CORES=$(notdir $*) -o $@ $^ $(RISCV_GCC_OPTS) $(RISCV_LINK_OPTS)

mc_rand_walk_%.riscv: mc_rand_walk.c
	$(RISCV_GCC) -DNUM_CORES=$(notdir $*) -o $@ $^ $(RISCV_GCC_OPTS) $(RISCV_LINK_OPTS)

mc_work_share_sort_%.riscv: mc_work_share_sort.c
	$(RISCV_GCC) -DNUM_CORES=$(notdir $*) -o $@ $^ $(RISCV_GCC_OPTS) $(RISCV_LINK_OPTS)

clean:
	rm -f *.riscv
