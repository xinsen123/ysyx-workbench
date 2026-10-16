`ifndef NPC_CONST_SV
`define NPC_CONST_SV

package npc_const;
/* verilator lint_off UNUSEDPARAM */
    localparam int DATA_WIDTH       = 32;
    // RV32E: 只有 x0-x15, 因此寄存器堆为 16 项, 索引 4 bit
    // (与 NEMU 的 CONFIG_RVE 是同一个开关的两侧)
    localparam int REG_ADDR_WIDTH   = 4;
    // 复位入口, 与 csrc/include/npc_config.h 的 CONFIG_MBASE +
    // CONFIG_PC_RESET_OFFSET 保持一致
    localparam logic [DATA_WIDTH-1:0] RESET_VECTOR = 32'h8000_0000;
/* verilator lint_on UNUSEDPARAM */

    // 写回数据来源
    typedef enum logic[1:0] {REG_ALU, REG_PC, REG_MEM, REG_NONE} reg_in_e;

    // 访存类型
    //   load : 决定加载宽度以及是否符号扩展
    //   store: 只用到宽度 (SL_BYTE/SL_HALF/SL_WORD 恰好分别对应 sb/sh/sw)
    typedef enum logic[3:0] {SL_BYTE, SL_UBYTE, SL_HALF, SL_UHALF, SL_WORD, SL_NONE} sl_type_e;
endpackage

`endif
