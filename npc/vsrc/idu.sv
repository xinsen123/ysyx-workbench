`include "npc_const.sv"
`include "npc_isa.sv"

module idu import npc_const::*;(
    input [DATA_WIDTH-1:0] inst,

    output logic [REG_ADDR_WIDTH-1:0]   rd,
    output logic [REG_ADDR_WIDTH-1:0]   radd1,
    output logic [REG_ADDR_WIDTH-1:0]   radd2,
    output logic [DATA_WIDTH-1:0]       imm,
    
    output reg_in_e     reg_input,
    output sl_type_e    sl_type,
    output alu_in_1     alu_input1,
    output alu_in_2     alu_input2,

    output logic        en_reg, en_pc,
    output logic        mem_r, mem_w,
    output logic        is_ebreak
);
    
    import npc_isa::*;


    wire [6:0]  opcode              = inst[6:0];
    wire [2:0]  funct3              = inst[14:12];
    // wire [6:0]  funct7              = inst[31:25];
    wire [DATA_WIDTH-1:0] immi      = {{21{inst[31]}}, inst[30:20]};
    wire [DATA_WIDTH-1:0] imms      = {{21{inst[31]}}, inst[30:25], inst[11:7]};
    wire [DATA_WIDTH-1:0] immu      = {inst[31:12], 12'b0};

    logic [REG_ADDR_WIDTH-1:0] rd0  = inst[7+REG_ADDR_WIDTH-1:7];
    logic [REG_ADDR_WIDTH-1:0] rs1  = inst[15+REG_ADDR_WIDTH-1:15];
    logic [REG_ADDR_WIDTH-1:0] rs2  = inst[20+REG_ADDR_WIDTH-1:20];


    always_comb begin
        
        rd          = 0;
        radd1       = 0;
        radd2       = 0;
        imm         = 0;
        reg_input   = REG_NONE;
        sl_type     = SL_NONE;
        alu_input1  = ALU_1_NULL;
        alu_input2  = ALU_2_NULL;
        en_reg      = 0;
        en_pc       = 0;
        mem_r       = 0;
        mem_w       = 0;
        is_ebreak   = 0;
        
        unique case (opcode)
            OPCODE_LUI: begin
                rd          = rd0;
                imm         = immu;
                reg_input   = REG_ALU;
                alu_input1  = ALU_1_NULL;
                alu_input2  = ALU_2_USE_IMM;
                en_reg      = 1;
            end

            OPCODE_JALR: begin
                rd          = rd0;
                radd1       = rs1;
                imm         = immi;
                reg_input   = REG_PC;
                alu_input1  = ALU_1_USE_RS1;
                alu_input2  = ALU_2_USE_IMM;
                en_pc       = 1;
                en_reg      = 1;
            end

            OPCODE_LOAD: begin
                rd          = rd0;
                radd1       = rs1;
                imm         = immi;
                reg_input   = REG_MEM;
                alu_input1  = ALU_1_USE_RS1;
                alu_input2  = ALU_2_USE_IMM;
                en_reg      = 1;
                mem_r       = 1;

                case (funct3) 
                    3'b010: sl_type = SL_WORD;  //lw
                    3'b100: sl_type = SL_UBYTE; //lbu
                    default: ;
                endcase
            end

            OPCODE_STORE: begin
                radd1       = rs1;
                radd2       = rs2;
                imm         = imms;
                alu_input1  = ALU_1_USE_RS1;
                alu_input2  = ALU_2_USE_IMM;
                mem_w       = 1;
                
                case (funct3) 
                    3'b000: sl_type = SL_BYTE; //sb
                    3'b010: sl_type = SL_WORD;  //sw
                    default: ;
                endcase
            end

            OPCODE_OP_IMM: begin
                rd          = rd0;
                radd1       = rs1;
                imm         = immi;
                reg_input   = REG_ALU;
                alu_input1  = ALU_1_USE_RS1;
                alu_input2  = ALU_2_USE_IMM;
                en_reg      = 1;
            end 

            OPCODE_OP: begin
                rd          = rd0;
                radd1       = rs1;
                radd2       = rs2;
                reg_input   = REG_ALU;
                alu_input1  = ALU_1_USE_RS1;
                alu_input2  = ALU_2_USE_RS2;
                en_reg      = 1;
            end

            OPCODE_SYSTEM: begin
                is_ebreak   = 1;
            end

            default: ;
        endcase
    end

endmodule
