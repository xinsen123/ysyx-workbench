# 源文件查找
VSRC = $(shell find $(abspath ./vsrc) -name "*.v")
CSRC = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")
VPATH = -I./vsrc

TOPNAME = top
BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(OBJ_DIR)/V$(TOPNAME)  # 修正：可执行文件在 obj_dir 中
ECODE = build/ecoder

CHISEL = chisel

SMOD = top

# 创建构建目录
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(ECODE):
	gcc ecoder/ecoder.c -o $(BUILD_DIR)/ecoder

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

show: cv
	yosys -p "read_verilog vsrc/top.v; \
				hierarchy -top $(SMOD); \
				show $(SMOD);"

ecode:$(ECODE)
	@$(ECODE)

cv:
	touch vsrc/$(TOPNAME).v
	./mill $(CHISEL).run > vsrc/$(TOPNAME).v

.PHONY: run clean all show cv
