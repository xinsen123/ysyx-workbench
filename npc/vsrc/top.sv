`include "npc_const.sv"
`include "npc_isa.sv"

import "DPI-C" function void ebreak();
import "DPI-C" function void invalid_inst(int pc);
module top import npc_const::*;(
        input clk,
        output reg [DATA_WIDTH-1:0] pc
    );

    import npc_isa::*;

    wire [DATA_WIDTH-1:0] rs1, rs2;
    wire [DATA_WIDTH-1:0] reg_in;

    RegisterFile #(
                     .ADDR_WIDTH(REG_ADDR_WIDTH),
                     .DATA_WIDTH(DATA_WIDTH)
                 ) GPR (
                     .clk(clk),
                     .waddr(rd[REG_ADDR_WIDTH-1:0]),
                     .wdata(reg_in),
                     .wen(en_reg),
                     .radd1(radd1[REG_ADDR_WIDTH-1:0]),
                     .radd2(radd2[REG_ADDR_WIDTH-1:0]),
                     .rs1(rs1),
                     .rs2(rs2)
                 );

    // output declaration of module ifu
    reg [DATA_WIDTH-1:0] inst /*verilator public*/;
    
    ifu IFU(
        .pc   	(pc    ),
        .inst 	(inst  )
    );
    

    // output declaration of module idu
    wire [REG_ADDR_WIDTH-1:0] rd;
    wire [REG_ADDR_WIDTH-1:0] radd1;
    wire [REG_ADDR_WIDTH-1:0] radd2;
    wire [DATA_WIDTH-1:0] imm;
    reg_in_e reg_input;
    sl_type_e sl_type;
    alu_in_1 alu_input1;
    alu_in_2 alu_input2;
    alu_op_e alu_op;
    br_type_e br_type;
    wire en_reg;
    wire en_pc;         // jal/jalr: 无条件跳转
    wire mem_r;
    wire mem_w;
    wire is_ebreak;
    wire is_illegal;
    
    idu IDU(
        .inst       	(inst        ),
        .rd         	(rd          ),
        .radd1      	(radd1       ),
        .radd2      	(radd2       ),
        .imm        	(imm         ),
        .reg_input  	(reg_input   ),
        .sl_type    	(sl_type     ),
        .alu_input1 	(alu_input1  ),
        .alu_input2 	(alu_input2  ),
        .alu_op     	(alu_op      ),
        .br_type    	(br_type     ),
        .en_reg     	(en_reg      ),
        .en_pc      	(en_pc       ),
        .mem_r      	(mem_r       ),
        .mem_w      	(mem_w       ),
        .is_ebreak     	(is_ebreak   ),
        .is_illegal    	(is_illegal  )
    );
    


    // output declaration of module exu
    wire [DATA_WIDTH-1:0] alu_out;
    wire br_taken;

    exu EXU(
            .rs1        	(rs1         ),
            .rs2        	(rs2         ),
            .imm        	(imm         ),
            .pc             (pc          ),
            .alu_input1 	(alu_input1  ),
            .alu_input2 	(alu_input2  ),
            .alu_op     	(alu_op      ),
            .br_type    	(br_type     ),
            .alu_out    	(alu_out     ),
            .br_taken   	(br_taken    )
        );

    // output declaration of module lsu
    reg [DATA_WIDTH-1:0] rdata;
    
    lsu LSU(
        .addr  	(alu_out),
        .wdata 	(rs2    ),
        .rdata 	(rdata  ),
        .ren   	(mem_r  ),
        .wen   	(mem_w  ),
        .clk   	(clk    ),
        .wmask 	(sl_type)
    );
    

    assign reg_in = (reg_input == REG_ALU) ? alu_out :
           (reg_input == REG_PC)           ? pc+4    :
           (reg_input == REG_MEM)          ? rdata   :  
           0;

    /* 下一条 PC 的选择
       - jal/jalr : en_pc 有效, 无条件跳到 alu_out (目标地址)
       - branch   : 由 exu 的 br_taken 决定是否跳到 alu_out (pc + imm)
       alu_out 的低位清零对应 jalr 规范里的 "最低位清零";
       所有指令 2 字节对齐(RV32E 无 C 扩展), 对其它跳转无影响 */
    wire pc_sel = en_pc | ((br_type != BR_NONE) & br_taken);

    initial begin
        pc = RESET_VECTOR;
    end

    always @(posedge clk) begin
        if (is_illegal) begin
            // 译码失败: 报出非法指令并停止推进 pc, 便于定位
            invalid_inst(pc);
        end else if (is_ebreak) begin
            ebreak();
        end
        pc <= (pc_sel & ~is_illegal) ? (alu_out & ~32'b1) : pc+4;
        // itrace 已在 C++ 侧实现 (csrc/cpu/cpu-exec.c), 此处不再 $display
    end

endmodule
