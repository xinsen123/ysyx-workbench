`define ADDI 7'b0010011 //I
`define JALR 7'b1100111 //I
`define ADD  7'b0110011 //R
`define LUI  7'b0110111 //U
`define LW   7'b0000011 //I
`define LBU  7'b0000011 //I
`define SW   7'b0100011 //S
`define SB   7'b0100011 //S

module IDU #(
    parameter ADDR_WIDTH = 5,
    parameter DATA_WIDTH = 32,
    parameter INST_TYPES = 4
)(
    input  [DATA_WIDTH-1 : 0] inst,
    output [INST_TYPES-1 : 0] itype,
    output [ADDR_WIDTH-1 : 0] rd, rs1, rs2,
    output [DATA_WIDTH-1 : 0] imm_I, imm_U, imm_S,

    output wen_reg, wen_mem, wen_pc
);
    wire [6:0] opcode = inst[6:0];
    
    assign itype[0] = opcode == `ADDI || opcode == `JALR || opcode == `LW || opcode == `LBU;//I
    assign itype[1] = opcode == `ADD;//R
    assign itype[2] = opcode == `LUI;//U
    assign itype[3] = opcode == `SW || opcode == `SB;//S

    assign rd = inst[11:7];
    assign rs1 = inst[19:15];
    assign rs2 = inst[24:20];
    
    assign imm_I = {{(20){inst[31]}}, inst[31:20]};
    assign imm_U = {inst[31:12], 12'b0};
    assign imm_S = {{(20){inst[31]}}, inst[31:25], inst[11:7]};

    assign wen_pc = opcode == `JALR;

endmodule