`ifndef NPC_ISA
`define NPC_ISA 

package npc_isa;

// RV32E 的 opcode 与 RV32I 完全一致 (见 riscv-spec 的 RV32E 说明)
typedef enum logic [6:0] {
     OPCODE_LUI    = 7'b0110111,
     OPCODE_AUIPC  = 7'b0010111,
     OPCODE_JAL    = 7'b1101111,
     OPCODE_JALR   = 7'b1100111,
     OPCODE_BRANCH = 7'b1100011,
     OPCODE_LOAD   = 7'b0000011,
     OPCODE_STORE  = 7'b0100011,
     OPCODE_OP_IMM = 7'b0010011,
     OPCODE_OP     = 7'b0110011,
     OPCODE_FENCE  = 7'b0001111,
     OPCODE_SYSTEM = 7'b1110011 
} opcode_e;

// ALU 第一操作数来源
typedef enum logic [1:0] {
    ALU_1_USE_RS1,
    ALU_1_USE_PC,
    ALU_1_NULL
} alu_in_1;

// ALU 第二操作数来源
typedef enum logic [1:0] {
    ALU_2_USE_IMM,
    ALU_2_USE_RS2,
    ALU_2_NULL
} alu_in_2;

// ALU 运算类型 (由 funct3/funct7 译码得到)
typedef enum logic [3:0] {
    ALU_ADD,
    ALU_SUB,
    ALU_SLL,
    ALU_SLT,
    ALU_SLTU,
    ALU_XOR,
    ALU_SRL,
    ALU_SRA,
    ALU_OR,
    ALU_AND,
    ALU_NONE
} alu_op_e;

// 分支比较类型 (由 branch 的 funct3 译码得到)
typedef enum logic [2:0] {
    BR_NONE,
    BR_EQ,      // beq
    BR_NE,      // bne
    BR_LT,      // blt
    BR_GE,      // bge
    BR_LTU,     // bltu
    BR_GEU      // bgeu
} br_type_e;

endpackage

`endif
