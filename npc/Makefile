CSRC = csrc/main.cpp
VSRC = $(wildcard vsrc/*.v)
OBJDIR = obj_dir
PREFIX = Vtop
TARGET = $(OBJDIR)/$(PREFIX)
VERILATOR = verilator

.PHONY: all clean sim

all: $(TARGET)

$(TARGET): $(VSRC) $(CSRC) | $(OBJDIR)
	$(VERILATOR) --cc $(VSRC) --exe $(CSRC)
	$(MAKE) -C $(OBJDIR) -f $(PREFIX).mk $(PREFIX)

$(OBJDIR):
	mkdir -p $(OBJDIR)


sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	./obj_dir/$(PREFIX)

clean:
	rm -rf $(OBJDIR)

include ../Makefile
