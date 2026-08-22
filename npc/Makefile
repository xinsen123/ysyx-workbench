# 源文件查找
PKG_SRC = $(abspath ./vsrc/npc_const.sv)
VSRC    = $(PKG_SRC) $(filter-out $(PKG_SRC),$(shell find $(abspath ./vsrc) -name "*.v" -or -name "*.sv"))
CSRC 	= $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")
HSRC 	= $(shell find $(abspath ./csrc) -name "*.h")
# 编译 C/C++ 源码所需的头文件搜索路径 (sdb/expr 等从 NEMU 移植的代码使用 <xxx.h> 风格)
INCLUDE = $(abspath ./csrc/include)
CFLAGS =  -I$(INCLUDE)
CFLAGS +=  -I$(abspath ./tools/capstone/repo/include)
VPATH = -I./vsrc

TOPNAME = top
BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(OBJ_DIR)/V$(TOPNAME)  # 修正：可执行文件在 obj_dir 中
SMOD = top

# 运行参数 (传给 NPC 可执行文件)
img =        # 镜像文件, 如 am-kernels/tests/cpu-tests/build/add-minirv-npc.bin
elf =        # ELF 文件 (ftrace 需要), 如 add-minirv-npc.elf
override args ?= --log=$(BUILD_DIR)/nemu-log.txt
override args += $(ARGS_DIFF)


# diff相关
DIFF_REF_SO = $(NEMU_HOME)/build/riscv32-nemu-interpreter-so
ARGS_DIFF = --diff=$(DIFF_REF_SO)

all: $(BIN)
# 创建构建目录
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 主要构建规则
$(BIN): $(BUILD_DIR) $(PKG_SRC) $(VSRC) $(CSRC) $(HSRC)
	verilator -cc --build --trace -j --Mdir $(OBJ_DIR) $(VPATH) --top-module $(TOPNAME) $(VSRC) --exe $(CSRC) --CFLAGS "$(CFLAGS) -g" --LDFLAGS "-lreadline"
	$(MAKE) -C $(OBJ_DIR) -f V$(TOPNAME).mk V$(TOPNAME)

# 运行仿真
run: $(BIN)  # 修正：依赖 BIN
	@$(BIN) $(args) $(img) $(elf)

# gdb
gdb: $(BIN)
	gdb --args ./$(BIN) $(args) $(img) $(elf)

# 清理
clean:
	rm -rf $(BUILD_DIR)

show:
	yosys -p "read_verilog vsrc/$(SMOD).v; \
				hierarchy -top $(SMOD); \
				show $(SMOD);"

.PHONY: run clean all show
