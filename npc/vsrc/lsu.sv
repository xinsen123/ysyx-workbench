`include "npc_const.sv"

module lsu import npc_const::*;(
    input [DATA_WIDTH-1:0] addr,
    input [DATA_WIDTH-1:0] wdata,
    output reg [DATA_WIDTH-1:0] rdata,

    input ren,
    input wen,
    input clk,
    input sl_type_e wmask
);

    import "DPI-C" function int pmem_read(int addr);
    import "DPI-C" function void pmem_write(int addr, byte mask, int data);


    always_comb begin
        if(ren) begin
            unique case (wmask)
                SL_BYTE: begin
                    rdata = pmem_read(addr) & {24'h0, 8'hff};
                end 
                SL_UBYTE: begin
                    rdata = pmem_read(addr) & {24'h0, 8'hff};
                end
                SL_WORD: begin
                    rdata = pmem_read(addr);
                end
                default: begin
                    rdata = '0;
                end
            endcase
            // $display("r:0x%x", rdata);
        end else begin
            rdata = 'b0;
        end
    end
    
    always @(posedge clk) begin
        if (wen) begin
            case (wmask)
                SL_BYTE: begin
                    pmem_write(addr, 1, wdata);
                end
                SL_WORD: begin
                    pmem_write(addr, 4, wdata);
                end
                default: begin
                    
                end
            endcase
        end
    end

endmodule
