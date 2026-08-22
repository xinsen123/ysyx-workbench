`ifndef NPC_ISA
`define NPC_ISA 

package npc_isa;

typedef enum logic [6:0] {
     OPCODE_LUI    = 7'b0110111,
     OPCODE_AUIPC  = 7'b0010111,
     OPCODE_JAL    = 7'b1101111,
     OPCODE_JALR   = 7'b1100111,
     OPCODE_LOAD   = 7'b0000011,
     OPCODE_STORE  = 7'b0100011,
     OPCODE_OP_IMM = 7'b0010011,
     OPCODE_OP     = 7'b0110011,
     OPCODE_SYSTEM = 7'b1110011 
} opcode_e;

typedef enum logic [1:0] {
    ALU_1_USE_RS1,
    ALU_1_NULL
} alu_in_1;

typedef enum logic [1:0] {
    ALU_2_USE_IMM,
    ALU_2_USE_RS2,
    ALU_2_NULL
} alu_in_2;

endpackage

`endif
