`include "const.vh"

module idu (
    input [`DATA_WIDTH-1:0] inst,

    output [`REG_ADDR_WIDTH-1:0] rd,
    output [`REG_ADDR_WIDTH-1:0] radd1,
    output [`REG_ADDR_WIDTH-1:0] radd2,
    output [`DATA_WIDTH-1:0] imm,
    
    output [1:0] reg_input,
    output [2:0] sl_type,
    output alu_input1, alu_input2,

    output en_reg, en_pc,
    output mem_r, mem_w,
    output is_ebreak
);
    
    parameter OP_IMMI = 7'b0010011,
              OP_IMMU = 7'b0110111,
              OP_JALR = 7'b1100111,
              OP_RTOR = 7'b0110011,
              OP_SAVE = 7'b0100011,
              OP_LOAD = 7'b0000011,
              OP_EBRK = 7'b1110011;

    wire [6:0] opcode = inst[6:0];
    wire [2:0] funct3 = inst[14:12];
    wire [6:0] funct7 = inst[31:25];
    wire [31:0] immi = {{21{inst[31]}}, inst[30:20]};
    wire [31:0] imms = {{21{inst[31]}}, inst[30:25], inst[11:7]};
    wire [31:0] immu = {inst[31:12], 12'b0};

    wire is_add  = (opcode == OP_RTOR) && (funct3 == 3'b000) && (funct7 == 7'b0000000);
    wire is_addi = (opcode == OP_IMMI) && (funct3 == 3'b000);
    wire is_lui  = (opcode == OP_IMMU);
    wire is_lw   = (opcode == OP_LOAD) && (funct3 == 3'b010);
    wire is_lbu  = (opcode == OP_LOAD) && (funct3 == 3'b100);
    wire is_sw   = (opcode == OP_SAVE) && (funct3 == 3'b010);
    wire is_sb   = (opcode == OP_SAVE) && (funct3 == 3'b000);
    wire is_jalr = (opcode == OP_JALR);
    wire is_ebrk = (opcode == OP_EBRK);

    wire type_r = is_add;
    wire type_i = is_addi || is_jalr || is_lw || is_lbu;
    wire type_s = is_sw || is_sb;
    wire type_u = is_lui;



    assign alu_input1 = is_add || is_addi || is_lw || is_lbu || is_sw || is_sb || is_jalr;
    assign alu_input2 = is_add;
    assign en_reg = is_add || is_addi || is_lui || is_lw || is_lbu || is_jalr;
    assign en_pc = is_jalr;
    assign mem_w = is_sb  || is_sw;
    assign mem_r = is_lbu || is_lw;
    assign is_ebreak = is_ebrk;

    assign rd = (type_i || type_r || type_u) ? inst[7+`REG_ADDR_WIDTH-1:7] : 0;
    assign radd1 = (type_i || type_r || type_s) ? inst[15+`REG_ADDR_WIDTH-1:15] : 0;
    assign radd2 = (type_r || type_s) ? inst[20+`REG_ADDR_WIDTH-1:20] : 0;
    assign imm = (type_i) ? immi :
                 (type_s) ? imms : 
                 (type_u) ? immu :
                 0;
    assign reg_input = (is_add || is_addi || is_lui) ? `REG_ALU :
                       (is_lw || is_lbu) ? `REG_MEM :
                       (is_jalr) ? `REG_PC :
                       `REG_NONE;
    assign sl_type = (is_lw)  ? `SL_WORD  : 
                     (is_lbu) ? `SL_UBYTE :
                     (is_sb)  ? `SL_BYTE  :
                     `SL_NONE;

endmodule
