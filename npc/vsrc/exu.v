`include "npc_const.sv"

module exu import npc_const::*;(
    input [DATA_WIDTH-1:0] rs1,
    input [DATA_WIDTH-1:0] rs2,
    input [DATA_WIDTH-1:0] imm,
    input alu_input1, alu_input2,

    output [DATA_WIDTH-1:0] alu_out
);

    wire [DATA_WIDTH-1:0] input1, input2;
    assign input1 = alu_input1 ? rs1 : 0;
    assign input2 = alu_input2 ? rs2 : imm;
    assign alu_out = input1 + input2;

endmodule
