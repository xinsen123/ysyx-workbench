`include "const.vh"

module lsu(
    input [`DATA_WIDTH-1:0] addr,
    input [`DATA_WIDTH-1:0] wdata,
    output reg [`DATA_WIDTH-1:0] rdata,

    input ren,
    input wen,
    input clk,
    input [3:0] wmask
);

    import "DPI-C" function int pmem_read(int addr);
    import "DPI-C" function void pmem_write(int addr, int data, byte mask);


    always_comb begin
        if(ren) begin
            rdata = pmem_read(addr) & {{8{wmask[3]}}, {8{wmask[2]}}, {8{wmask[1]}}, {8{wmask[0]}}};
            // $display("r:0x%x", rdata);
        end else begin
            rdata = `DATA_WIDTH'b0;
        end
    end
    
    always @(posedge clk) begin
        if (wen) begin
            pmem_write(addr, wdata, {{4'b0}, wmask}); 
        end
    end

endmodule
