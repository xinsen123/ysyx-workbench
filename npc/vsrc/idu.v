`include "const.vh"

`define ADDI 7'b0010011 //I
`define JALR 7'b1100111 //I
`define ADD  7'b0110011 //R
`define LUI  7'b0110111 //U
`define LW   7'b0000011 //I
`define LBU  7'b0000011 //I
`define SW   7'b0100011 //S
`define SB   7'b0100011 //S

`define OPCODE inst[6:0]
`define FUNC3  inst[14:12]

module IDU(
    input  [DATA_WIDTH-1 : 0] inst,
    output [INST_TYPES-1 : 0] itype,
    output [ADDR_WIDTH-1 : 0] rd, rs1, rs2,
    output [DATA_WIDTH-1 : 0] imm_I, imm_U, imm_S,

    output wen_reg, wen_mem
);
    assign itype[0] = `OPCODE == `ADDI || `OPCODE == `JALR || `OPCODE == `LW || `OPCODE == `LBU;//I
    assign itype[1] = `OPCODE == `ADD;//R
    assign itype[2] = `OPCODE == `LUI;//U
    assign itype[3] = `OPCODE == `SW || `OPCODE == `SB;//S

    assign rd = inst[11:7];
    assign rs1 = inst[19:15];
    assign rs2 = inst[24:20];
    
    assign imm_I = {{(20){inst[31]}}, inst[31:20]};
    assign imm_U = {{(12){inst[31]}}, inst[31:12]};
    assign imm_S = {{(20){inst[31]}}, inst[31:25], inst[11:7]};

endmodule