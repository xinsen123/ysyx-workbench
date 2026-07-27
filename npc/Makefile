# 源文件查找
VSRC = $(shell find $(abspath ./vsrc) -name "top.v")
CSRC = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")
VPATH = -I./vsrc

TOPNAME = top
BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(OBJ_DIR)/V$(TOPNAME)  # 修正：可执行文件在 obj_dir 中
ECODE = build/ecoder

img = 
SMOD = top

# 创建构建目录
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 主要构建规则
$(BIN): $(BUILD_DIR) $(VSRC) $(CSRC)
	@rm -rf $(OBJ_DIR)
	verilator -cc --build --trace -j --Mdir $(OBJ_DIR) $(VPATH) --top-module $(TOPNAME) $(VSRC) --exe $(CSRC)
	$(MAKE) -C $(OBJ_DIR) -f V$(TOPNAME).mk V$(TOPNAME)

# 运行仿真
run: $(BIN)  # 修正：依赖 BIN
	@$(BIN) $(img)

# gdb
gdb: $(BIN)
	gdb --args ./$(BIN) $(img)

# 清理
clean:
	rm -rf $(BUILD_DIR)

show: 
	yosys -p "read_verilog vsrc/$(SMOD).v; \
				hierarchy -top $(SMOD); \
				show $(SMOD);"

.PHONY: run clean all show
