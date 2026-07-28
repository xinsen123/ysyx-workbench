`include "const.vh"
`include "RegisterFile.v"
`include "idu.v"

import "DPI-C" function void ebreak();
module top(
        input clk
    );


    wire [`DATA_WIDTH-1:0] rs1, rs2;
    wire [`DATA_WIDTH-1:0] reg_in;

    RegisterFile #(
                     .ADDR_WIDTH(`REG_ADDR_WIDTH),
                     .DATA_WIDTH(`DATA_WIDTH)
                 ) GPR (
                     .clk(clk),
                     .waddr(rd[`REG_ADDR_WIDTH-1:0]),
                     .wdata(reg_in),
                     .wen(en_reg),
                     .radd1(radd1[`REG_ADDR_WIDTH-1:0]),
                     .radd2(radd2[`REG_ADDR_WIDTH-1:0]),
                     .rs1(rs1),
                     .rs2(rs2)
                 );

    // output declaration of module ifu
    reg [`DATA_WIDTH-1:0] inst;
    
    ifu IFU(
        .pc   	(pc    ),
        .inst 	(inst  )
    );
    

    // output declaration of module idu
    wire [`REG_ADDR_WIDTH-1:0] rd;
    wire [`REG_ADDR_WIDTH-1:0] radd1;
    wire [`REG_ADDR_WIDTH-1:0] radd2;
    wire [`DATA_WIDTH-1:0] imm;
    wire [1:0] reg_input;
    wire [3:0] sl_type;
    wire alu_input1;
    wire alu_input2;
    wire en_reg;
    wire en_pc;
    wire mem_r;
    wire mem_w;
    wire is_ebreak;
    
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
        .en_reg     	(en_reg      ),
        .en_pc      	(en_pc       ),
        .mem_r      	(mem_r       ),
        .mem_w      	(mem_w       ),
        .is_ebreak     	(is_ebreak   )
    );
    


    // output declaration of module exu
    wire [`DATA_WIDTH-1:0] alu_out;

    exu EXU(
            .rs1        	(rs1         ),
            .rs2        	(rs2         ),
            .imm        	(imm         ),
            .alu_input1 	(alu_input1  ),
            .alu_input2 	(alu_input2  ),
            .alu_out    	(alu_out     )
        );

    // output declaration of module lsu
    reg [`DATA_WIDTH-1:0] rdata;
    
    lsu LSU(
        .addr  	(alu_out),
        .wdata 	(rs2    ),
        .rdata 	(rdata  ),
        .ren   	(mem_r  ),
        .wen   	(mem_w  ),
        .clk   	(clk    ),
        .wmask 	(sl_type)
    );
    

    assign reg_in = (reg_input == `REG_ALU) ? alu_out :
           (reg_input == `REG_PC)           ? pc+4    :
           (reg_input == `REG_MEM)          ? rdata   :  
           0;

    reg [`DATA_WIDTH-1:0] pc;

    initial begin
        pc = 32'h80000000;
    end

    always @(posedge clk) begin
        if(is_ebreak) begin
            ebreak();
        end
        pc <= en_pc ? alu_out : pc+4;
        // $display("0x%x: 0x%x", pc, inst);
    end

endmodule
