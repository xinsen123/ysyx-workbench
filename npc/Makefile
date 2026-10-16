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

# diff相关
# 访客 ISA: riscv32e (RV32E, 16 个寄存器) / minirv / riscv32 (RV32I, 32 个).
# 由 AM 的 scripts/platform/npc.mk 以 ISA=$(ISA) 传进来。
ISA ?= riscv32e

# 参考设计 so 的文件名只取决于 NEMU 的 CONFIG_ISA (riscv32),
# 但 CPU_state 的大小取决于 CONFIG_RVE (16 还是 32 个 GPR)。
# difftest_regcpy 直接按 sizeof(CPU_state) 做 memcpy, 没有长度协商,
# 两边不一致就会越界写内存 —— 所以显式检查, 而不是静默跑出错的结果。
DIFF_REF_SO   = $(NEMU_HOME)/build/riscv32-nemu-interpreter-so
NEMU_AUTOCONF = $(NEMU_HOME)/include/config/auto.conf
ARGS_DIFF     = --diff=$(DIFF_REF_SO)

override args ?= --log=$(BUILD_DIR)/nemu-log.txt
override args += $(ARGS_DIFF)

all: $(BIN)

# 创建构建目录
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 主要构建规则
$(BIN): $(BUILD_DIR) $(PKG_SRC) $(VSRC) $(CSRC) $(HSRC)
	verilator -cc --build --trace -j --Mdir $(OBJ_DIR) $(VPATH) --top-module $(TOPNAME) $(VSRC) --exe $(CSRC) --CFLAGS "$(CFLAGS) -g" --LDFLAGS "-lreadline"
	$(MAKE) -C $(OBJ_DIR) -f V$(TOPNAME).mk V$(TOPNAME)

# 检查 difftest 参考设计与 RTL 的寄存器个数是否匹配
# 注意 RTL 固定为 RV32E (REG_ADDR_WIDTH=4, 16 个寄存器), 与 ISA 变量无关,
# 所以参考设计的 NEMU 必须始终打开 CONFIG_RVE。
check-ref:
	@if ! grep -q "^CONFIG_RVE=y" $(NEMU_AUTOCONF) 2>/dev/null; then \
		echo "*** 参考设计 NEMU 没有打开 CONFIG_RVE, 与 RTL 的 16 个寄存器 (RV32E)不等"; \
		echo "*** 两边 CPU_state 大小不一致会让 difftest_regcpy 越界写内存"; \
		echo "*** 请执行: make -C $(NEMU_HOME) riscv32e_defconfig"; \
		echo "*** 然后:   make -C $(NEMU_HOME) SHARE=1   # 重编 difftest 参考 so"; \
		exit 1; \
	fi
	@echo "difftest ref: $(DIFF_REF_SO)  (ISA=$(ISA), NEMU CONFIG_RVE=y)"

# 运行仿真
run: check-ref $(BIN)
	@$(BIN) $(args) $(img) $(elf)

# gdb
gdb: check-ref $(BIN)
	gdb --args ./$(BIN) $(args) $(img) $(elf)

# 清理
clean:
	rm -rf $(BUILD_DIR)

show:
	yosys -p "read_verilog vsrc/$(SMOD).v; \
				hierarchy -top $(SMOD); \
				show $(SMOD);"

.PHONY: run gdb clean all show check-ref
