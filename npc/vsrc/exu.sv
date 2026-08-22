`include "npc_const.sv"
`include "npc_isa.sv"

module exu import npc_const::*; import npc_isa::*;(
    input [DATA_WIDTH-1:0] rs1,
    input [DATA_WIDTH-1:0] rs2,
    input [DATA_WIDTH-1:0] imm,
    input alu_in_1 alu_input1,
    input alu_in_2 alu_input2,

    output logic [DATA_WIDTH-1:0] alu_out
);

    logic [DATA_WIDTH-1:0] input1, input2;
    
    always_comb begin

        input1 = 0;
        input2 = 0;
        alu_out = 0;

        case (alu_input1)
            ALU_1_USE_RS1   : input1 = rs1;
            ALU_1_NULL      : input1 = 0;    
            default: ;
        endcase

        case (alu_input2)
            ALU_2_USE_IMM   : input2 = imm;
            ALU_2_USE_RS2   : input2 = rs2;
            ALU_2_NULL      : input2 = 0;
            default: ;
        endcase

        alu_out = input1 + input2;
    end

endmodule
