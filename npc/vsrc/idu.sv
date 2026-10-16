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
    output alu_op_e     alu_op,
    output br_type_e    br_type,

    output logic        en_reg,
    output logic        en_pc,
    output logic        mem_r, mem_w,
    output logic        is_ebreak,
    output logic        is_illegal
);
    
    import npc_isa::*;

    wire [6:0]  opcode              = inst[6:0];
    wire [2:0]  funct3              = inst[14:12];
    wire [6:0]  funct7              = inst[31:25];

    /* 立即数的位域位置不受 RV32E 影响, 必须按 RV32I 的完整位置取。
       各条指令的立即数含义见 riscv-spec Vol.I 的指令格式图:
         I 型: imm[11:0] = inst[31:20]
         S 型: imm[11:5] = inst[31:25], imm[4:0] = inst[11:7]
         B 型: imm[12] = inst[31], imm[11] = inst[7], imm[10:5] = inst[30:25],
               imm[4:1] = inst[11:8], imm[0] = 0
         U 型: imm[31:12] = inst[31:12]
         J 型: imm[20] = inst[31], imm[19:12] = inst[19:12], imm[11] = inst[20],
               imm[10:1] = inst[30:21], imm[0] = 0 */
    wire [DATA_WIDTH-1:0] immi      = {{20{inst[31]}}, inst[31:20]};
    wire [DATA_WIDTH-1:0] imms      = {{20{inst[31]}}, inst[31:25], inst[11:7]};
    wire [DATA_WIDTH-1:0] immb      = {{19{inst[31]}}, inst[31], inst[7], inst[30:25], inst[11:8], 1'b0};
    wire [DATA_WIDTH-1:0] immu      = {inst[31:12], 12'b0};
    wire [DATA_WIDTH-1:0] immj      = {{11{inst[31]}}, inst[31], inst[19:12], inst[20], inst[30:21], 1'b0};

    /* 寄存器说明符: RV32E 沿用 RV32I 的编码, 只是 5 bit 字段的最高位
       (rd 的 inst[11], rs1 的 inst[19], rs2 的 inst[24]) 恒为 0,
       所以取字段的低 REG_ADDR_WIDTH 位就等于寄存器编号。
       实测: `addi a0,a1,5` = 0x00558513, rd = inst[10:7] = 10 = a0,
       rs1 = inst[18:15] = 11 = a1, 与工具链输出一致。 */
    wire [REG_ADDR_WIDTH-1:0] rd0  = inst[7+REG_ADDR_WIDTH-1:7];
    wire [REG_ADDR_WIDTH-1:0] rs1  = inst[15+REG_ADDR_WIDTH-1:15];
    wire [REG_ADDR_WIDTH-1:0] rs2  = inst[20+REG_ADDR_WIDTH-1:20];


    always_comb begin
        
        rd          = 0;
        radd1       = 0;
        radd2       = 0;
        imm         = 0;
        reg_input   = REG_NONE;
        sl_type     = SL_NONE;
        alu_input1  = ALU_1_NULL;
        alu_input2  = ALU_2_NULL;
        alu_op      = ALU_NONE;
        br_type     = BR_NONE;
        en_reg      = 0;
        en_pc       = 0;
        mem_r       = 0;
        mem_w       = 0;
        is_ebreak   = 0;
        is_illegal  = 1;    // 不变量: 只有"完整译码成功"的分支才把它清零

        unique case (opcode)
            OPCODE_LUI: begin
                rd          = rd0;
                imm         = immu;
                reg_input   = REG_ALU;
                alu_input1  = ALU_1_NULL;
                alu_input2  = ALU_2_USE_IMM;
                alu_op      = ALU_ADD;
                en_reg      = 1;
                is_illegal  = 0;
            end

            OPCODE_AUIPC: begin
                rd          = rd0;
                imm         = immu;
                reg_input   = REG_ALU;
                alu_input1  = ALU_1_USE_PC;
                alu_input2  = ALU_2_USE_IMM;
                alu_op      = ALU_ADD;
                en_reg      = 1;
                is_illegal  = 0;
            end

            OPCODE_JAL: begin
                rd          = rd0;
                imm         = immj;
                reg_input   = REG_PC;           // 写回 pc+4
                alu_input1  = ALU_1_USE_PC;
                alu_input2  = ALU_2_USE_IMM;
                alu_op      = ALU_ADD;          // 目标地址 = pc + imm
                en_pc       = 1;
                en_reg      = 1;
                is_illegal  = 0;
            end

            OPCODE_JALR: begin
                rd          = rd0;
                radd1       = rs1;
                imm         = immi;
                reg_input   = REG_PC;
                alu_input1  = ALU_1_USE_RS1;
                alu_input2  = ALU_2_USE_IMM;
                alu_op      = ALU_ADD;          // 目标地址 = rs1 + imm
                en_pc       = 1;
                en_reg      = 1;
                is_illegal  = 0;
            end

            OPCODE_BRANCH: begin
                radd1       = rs1;
                radd2       = rs2;
                imm         = immb;
                alu_input1  = ALU_1_USE_PC;
                alu_input2  = ALU_2_USE_IMM;
                alu_op      = ALU_ADD;          // 目标地址 = pc + imm
                // 是否真的跳转由 exu 的 br_taken 决定, 所以 en_pc 保持 0
                case (funct3)
                    3'b000: begin br_type = BR_EQ;  is_illegal = 0; end
                    3'b001: begin br_type = BR_NE;  is_illegal = 0; end
                    3'b100: begin br_type = BR_LT;  is_illegal = 0; end
                    3'b101: begin br_type = BR_GE;  is_illegal = 0; end
                    3'b110: begin br_type = BR_LTU; is_illegal = 0; end
                    3'b111: begin br_type = BR_GEU; is_illegal = 0; end
                    default: ;
                endcase
            end

            OPCODE_LOAD: begin
                rd          = rd0;
                radd1       = rs1;
                imm         = immi;
                reg_input   = REG_MEM;
                alu_input1  = ALU_1_USE_RS1;
                alu_input2  = ALU_2_USE_IMM;
                alu_op      = ALU_ADD;          // 访存地址 = rs1 + imm

                case (funct3) 
                    3'b000: begin sl_type = SL_BYTE;  mem_r = 1; is_illegal = 0; end  // lb
                    3'b001: begin sl_type = SL_HALF;  mem_r = 1; is_illegal = 0; end  // lh
                    3'b010: begin sl_type = SL_WORD;  mem_r = 1; is_illegal = 0; end  // lw
                    3'b100: begin sl_type = SL_UBYTE; mem_r = 1; is_illegal = 0; end  // lbu
                    3'b101: begin sl_type = SL_UHALF; mem_r = 1; is_illegal = 0; end  // lhu
                    default: ;
                endcase

                if (mem_r) en_reg = 1;
            end

            OPCODE_STORE: begin
                radd1       = rs1;
                radd2       = rs2;
                imm         = imms;
                alu_input1  = ALU_1_USE_RS1;
                alu_input2  = ALU_2_USE_IMM;
                alu_op      = ALU_ADD;          // 访存地址 = rs1 + imm
                // 写回使能保持 0: store 不写寄存器
                case (funct3) 
                    3'b000: begin sl_type = SL_BYTE; mem_w = 1; is_illegal = 0; end  // sb
                    3'b001: begin sl_type = SL_HALF; mem_w = 1; is_illegal = 0; end  // sh
                    3'b010: begin sl_type = SL_WORD; mem_w = 1; is_illegal = 0; end  // sw
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

                case (funct3)
                    // 算术/逻辑
                    3'b000: begin alu_op = ALU_ADD;  is_illegal = 0; end  // addi
                    3'b010: begin alu_op = ALU_SLT;  is_illegal = 0; end  // slti
                    3'b011: begin alu_op = ALU_SLTU; is_illegal = 0; end  // sltiu
                    3'b100: begin alu_op = ALU_XOR;  is_illegal = 0; end  // xori
                    3'b110: begin alu_op = ALU_OR;   is_illegal = 0; end  // ori
                    3'b111: begin alu_op = ALU_AND;  is_illegal = 0; end  // andi
                    // 移位: RV32 的 shamt 只有低 5 位, funct7 用来区分 srl/sra
                    3'b001: begin
                        if (funct7 == 7'b0000000) begin alu_op = ALU_SLL; is_illegal = 0; end  // slli
                    end
                    3'b101: begin
                        if      (funct7 == 7'b0000000) begin alu_op = ALU_SRL; is_illegal = 0; end  // srli
                        else if (funct7 == 7'b0100000) begin alu_op = ALU_SRA; is_illegal = 0; end  // srai
                    end
                    default: ;
                endcase

                if (!is_illegal) en_reg = 1;
            end 

            OPCODE_OP: begin
                rd          = rd0;
                radd1       = rs1;
                radd2       = rs2;
                reg_input   = REG_ALU;
                alu_input1  = ALU_1_USE_RS1;
                alu_input2  = ALU_2_USE_RS2;

                case (funct3)
                    // funct7[5] = 1 时是减法 / 算术右移; RV32E 没有 M 扩展,
                    // 因此只有 0000000 和 0100000 两个合法 funct7
                    3'b000: begin
                        if      (funct7 == 7'b0000000) begin alu_op = ALU_ADD; is_illegal = 0; end  // add
                        else if (funct7 == 7'b0100000) begin alu_op = ALU_SUB; is_illegal = 0; end  // sub
                    end
                    3'b001: begin
                        if (funct7 == 7'b0000000) begin alu_op = ALU_SLL; is_illegal = 0; end       // sll
                    end
                    3'b010: begin
                        if (funct7 == 7'b0000000) begin alu_op = ALU_SLT; is_illegal = 0; end       // slt
                    end
                    3'b011: begin
                        if (funct7 == 7'b0000000) begin alu_op = ALU_SLTU; is_illegal = 0; end      // sltu
                    end
                    3'b100: begin
                        if (funct7 == 7'b0000000) begin alu_op = ALU_XOR; is_illegal = 0; end       // xor
                    end
                    3'b101: begin
                        if      (funct7 == 7'b0000000) begin alu_op = ALU_SRL; is_illegal = 0; end  // srl
                        else if (funct7 == 7'b0100000) begin alu_op = ALU_SRA; is_illegal = 0; end  // sra
                    end
                    3'b110: begin
                        if (funct7 == 7'b0000000) begin alu_op = ALU_OR; is_illegal = 0; end        // or
                    end
                    3'b111: begin
                        if (funct7 == 7'b0000000) begin alu_op = ALU_AND; is_illegal = 0; end       // and
                    end
                    default: ;
                endcase

                if (!is_illegal) en_reg = 1;
            end

            OPCODE_FENCE: begin
                // 单周期无 cache 的处理器里 fence 是空操作
                if (funct3 == 3'b000) is_illegal = 0;
            end

            OPCODE_SYSTEM: begin
                if (funct3 == 3'b000) begin
                    unique case (inst[31:20])
                        12'h000: ;                            // ecall: 需要 CSR/trap 支持, 暂作非法指令报出
                        12'h001: begin is_ebreak = 1; is_illegal = 0; end  // ebreak: 停机
                        default: ;
                    endcase
                end
                // funct3 != 0 即 CSR 指令, 属于 Zicsr, 暂未实现
            end

            default: ;
        endcase
    end

endmodule
