`ifndef NPC_CONST_SV
`define NPC_CONST_SV

package npc_const;
/* verilator lint_off UNUSEDPARAM */
    localparam int DATA_WIDTH       = 32;
    localparam int REG_ADDR_WIDTH   = 4;
/* verilator lint_on UNUSEDPARAM */

    typedef enum logic[1:0] {REG_ALU, REG_PC, REG_MEM, REG_NONE} reg_in_e;
    typedef enum logic[3:0] {SL_BYTE, SL_UBYTE, SL_HALF, SL_UHALF, SL_WORD, SL_NONE} sl_type_e;
endpackage

`endif
