`include "npc_const.sv"
`include "npc_isa.sv"

module exu import npc_const::*; import npc_isa::*;(
    input [DATA_WIDTH-1:0] rs1,
    input [DATA_WIDTH-1:0] rs2,
    input [DATA_WIDTH-1:0] imm,
    input [DATA_WIDTH-1:0] pc,
    input alu_in_1 alu_input1,
    input alu_in_2 alu_input2,
    input alu_op_e alu_op,
    input br_type_e br_type,

    output logic [DATA_WIDTH-1:0] alu_out,
    output logic br_taken
);

    logic [DATA_WIDTH-1:0] input1, input2;
    
    always_comb begin

        input1 = 0;
        input2 = 0;

        case (alu_input1)
            ALU_1_USE_RS1   : input1 = rs1;
            ALU_1_USE_PC    : input1 = pc;
            ALU_1_NULL      : input1 = 0;    
            default         : input1 = 0;
        endcase

        case (alu_input2)
            ALU_2_USE_IMM   : input2 = imm;
            ALU_2_USE_RS2   : input2 = rs2;
            ALU_2_NULL      : input2 = 0;
            default         : input2 = 0;
        endcase
    end

    // 运算结果
    // 注意: 除算术右移外都按无符号位模式处理, slt/sltu 用 $signed() 区分
    always_comb begin
        alu_out = 0;

        case (alu_op)
            ALU_ADD  : alu_out = input1 + input2;
            ALU_SUB  : alu_out = input1 - input2;
            ALU_SLL  : alu_out = input1 << input2[4:0];             // RV32 移位量只有低 5 位
            ALU_SLT  : alu_out = ($signed(input1) <  $signed(input2)) ? 32'd1 : 32'd0;
            ALU_SLTU : alu_out = (input1 <  input2)                   ? 32'd1 : 32'd0;
            ALU_XOR  : alu_out = input1 ^ input2;
            ALU_SRL  : alu_out = input1 >> input2[4:0];
            ALU_SRA  : alu_out = $signed(input1) >>> input2[4:0];
            ALU_OR   : alu_out = input1 | input2;
            ALU_AND  : alu_out = input1 & input2;
            default  : alu_out = 0;
        endcase
    end

    // 分支条件判断
    // 直接用原始的 rs1/rs2, 与 ALU 的操作数选择无关
    always_comb begin
        case (br_type)
            BR_EQ   : br_taken = (rs1 == rs2);
            BR_NE   : br_taken = (rs1 != rs2);
            BR_LT   : br_taken = ($signed(rs1) <  $signed(rs2));
            BR_GE   : br_taken = ($signed(rs1) >= $signed(rs2));
            BR_LTU  : br_taken = (rs1 <  rs2);
            BR_GEU  : br_taken = (rs1 >= rs2);
            default : br_taken = 1'b0;
        endcase
    end

endmodule
