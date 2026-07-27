
module RegisterFile #(parameter ADDR_WIDTH = 4, parameter DATA_WIDTH = 32) (
        input clk,
        input [DATA_WIDTH-1:0] wdata,
        input [ADDR_WIDTH-1:0] waddr,
        input wen,

        input [ADDR_WIDTH-1:0] radd1,
        input [ADDR_WIDTH-1:0] radd2,
        output [DATA_WIDTH-1:0] rs1,
        output [DATA_WIDTH-1:0] rs2
    );

    reg [DATA_WIDTH-1:0] rf [2**ADDR_WIDTH-1:0];
    always @(posedge clk) begin
        if (wen)
            rf[waddr] <= wdata;
    end

    assign rs1 = (radd1 == 0) ? 0 : rf[radd1];
    assign rs2 = (radd2 == 0) ? 0 : rf[radd2];

    export "DPI-C" task read_reg;
    task read_reg (
        input int idx,
        output int num
    );
        num = idx == 0 ? 0 : rf[idx];
    endtask //read_reg

endmodule
