# 源文件查找
# VSRC = $(shell find $(abspath ./vsrc) -name "*.v")
VSRC = ./vsrc/top.v
CSRC = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")
VPATH = -I./vsrc

TOPNAME = top
BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(OBJ_DIR)/V$(TOPNAME)  # 修正：可执行文件在 obj_dir 中

# 创建构建目录
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 主要构建规则
$(BIN): $(BUILD_DIR) $(VSRC) $(CSRC)
	@rm -rf $(OBJ_DIR)
	verilator -cc --Mdir $(OBJ_DIR) $(VPATH) --top-module $(TOPNAME) $(VSRC) --exe $(CSRC)
	$(MAKE) -C $(OBJ_DIR) -f V$(TOPNAME).mk V$(TOPNAME)

# 运行仿真
run: $(BIN)  # 修正：依赖 BIN
	@$(BIN)

# 清理
clean:
	rm -rf $(BUILD_DIR)

.PHONY: run clean all test