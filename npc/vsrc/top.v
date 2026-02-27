`include "const.vh"

module top(
    input reg  [DATA_WIDTH-1:0] inst,
    output reg [DATA_WIDTH-1:0] pc,
    input clk,
    input rst
);

    reg [7:0] led;

    wire [DATA_WIDTH-1 : 0] regin_data[REGIN_LINE], rout_data1, rout_data2;
    wire [DATA_WIDTH-1 : 0] imm_I, imm_U, imm_S;
    
    wire [ADDR_WIDTH-1 : 0] rd, rs1, rs2, reg_waddr;
    
    wire [INST_TYPES-1 : 0] itype;
    wire wen_reg, wen_mem;
    
    RegisterFile #(ADDR_WIDTH, DATA_WIDTH) 
        Rvreg ( .wdata(regin_data[0]), 
                .waddr(reg_waddr),
                .raddr1(rs1),
                .raddr2(rs2),
                .rdata1(rout_data1),
                .rdata2(rout_data2),
                .clk(clk),
                .wen(wen_reg));
                
    IDU idu (
        .inst(inst),
        .itype(itype),
        .rd(rd),
        .rs1(rs1),
        .rs2(rs2),
        .imm_I(imm_I),
        .imm_U(imm_U),
        .imm_S(imm_S),
        .wen_reg(wen_reg),
        .wen_mem(wen_mem)
    );

    always @(posedge clk) begin
        pc++;
    end

endmodule

module RegisterFile #(ADDR_WIDTH = 1, DATA_WIDTH = 1) (
    input clk,
    input [DATA_WIDTH-1:0] wdata,
    input [ADDR_WIDTH-1:0] waddr,
    input [ADDR_WIDTH-1:0] raddr1,
    input [ADDR_WIDTH-1:0] raddr2,
    output [DATA_WIDTH-1:0] rdata1,
    output [DATA_WIDTH-1:0] rdata2,
    input wen
);
    reg [DATA_WIDTH-1:0] rf [2**ADDR_WIDTH-1:0];
    always @(posedge clk) begin
        if (wen) rf[waddr] <= wdata;
    end

    assign rdata1 = raddr1 == 0 ? 0 : rf[raddr1];
    assign rdata2 = raddr2 == 0 ? 0 : rf[raddr2];
endmodule