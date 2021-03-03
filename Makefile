
include Makefile.frag

RISCV_GCC       = $(CROSS_COMPILE)gcc 
RISCV_GCC_OPTS  = -march=rv64imafd -mabi=lp64 -mcmodel=medany -I $(BP_TEST_DIR)/include
RISCV_LINK_OPTS = -T $(BP_TEST_DIR)/src/perch/riscv.ld -L$(BP_TEST_DIR)/lib -static -nostartfiles -lperch

.PHONY: all bp-demo-riscv

vpath %.c ./src
vpath %.S ./src

all: bp-demo-riscv

bp-demo-riscv: $(foreach x,$(subst -,_,$(BP_DEMOS)),$(x).riscv)

%.riscv: %.o
	$(RISCV_GCC) -o $@ $^ $(RISCV_LINK_OPTS)

paging.riscv: vm_start.o paging.o
	$(RISCV_GCC) -o $@ $^ $(RISCV_LINK_OPTS)

mapping.riscv: vm_start.o mapping.o
	$(RISCV_GCC) -o $@ $^ $(RISCV_LINK_OPTS)

mc_sanity_%.riscv: mc_sanity.o
	$(RISCV_GCC) -DNUM_CORES=$(notdir $*) -o $@ $^ $(RISCV_LINK_OPTS)

mc_template_%.riscv: mc_template.o
	$(RISCV_GCC) -DNUM_CORES=$(notdir $*) -o $@ $^ $(RISCV_LINK_OPTS)

mc_rand_walk_%.riscv: mc_rand_walk.o
	$(RISCV_GCC) -DNUM_CORES=$(notdir $*) -o $@ $^ $(RISCV_LINK_OPTS)

mc_work_share_sort_%.riscv: mc_work_share_sort.o
	$(RISCV_GCC) -DNUM_CORES=$(notdir $*) -o $@ $^ $(RISCV_LINK_OPTS)

%.o: %.c
	$(RISCV_GCC) -c -o $@ $< $(RISCV_GCC_OPTS)

%.o: %.S
	$(RISCV_GCC) -c -o $@ $< $(RISCV_GCC_OPTS)

clean:
	rm -f *.o
	rm -f *.riscv
