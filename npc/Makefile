CSRC = csrc/main.cpp
VSRC = $(wildcard vsrc/*.v)
OBJDIR = obj_dir
PREFIX = Vtop
TOPNAME = top
TARGET = $(OBJDIR)/$(PREFIX)
VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc  \
				-O3 --x-assign fast --x-initial fast --noassert
TRACE = 

.PHONY: all clean sim

include $(NVBOARD_HOME)/scripts/nvboard.mk

all: $(TARGET)

$(TARGET): $(VSRC) $(CSRC) | $(OBJDIR)
	$(VERILATOR) --cc $(TRACE) $(VSRC) --exe $(CSRC)
	$(MAKE) -C $(OBJDIR) -f $(PREFIX).mk $(PREFIX)

$(OBJDIR):
	mkdir -p $(OBJDIR)


sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	./obj_dir/$(PREFIX)

clean:
	rm -rf $(OBJDIR)

include ../Makefile
