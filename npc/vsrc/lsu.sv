`include "npc_const.sv"

module lsu import npc_const::*;(
    input [DATA_WIDTH-1:0] addr,
    input [DATA_WIDTH-1:0] wdata,
    output logic [DATA_WIDTH-1:0] rdata,

    input ren,
    input wen,
    input clk,
    input sl_type_e wmask
);

    import "DPI-C" function int pmem_read(int addr);
    import "DPI-C" function void pmem_write(int addr, byte mask, int data);

    /* 读: 每次总是从 addr 开始读 4 个字节, 再按访存宽度取出需要的低位字节
       并做符号/零扩展。由于 pmem_read 是从 addr 起按小端返回 4 字节,
       这样写对非对齐的 lb/lh/lbu/lhu 也同样正确。
       注意 pmem_read 是一次 DPI-C 调用, 只有 ren 有效时才发起, 否则可能
       用任意地址访问物理内存并触发越界断言。 */
    always_comb begin
        rdata = '0;

        if (ren) begin
            // automatic: 每次求值都重新声明并重新调用 pmem_read,
            // 否则会按静态变量处理, 初值只赋一次 (verilator IMPLICITSTATIC)
            automatic logic [DATA_WIDTH-1:0] mem_rdata = pmem_read(addr);

            case (wmask)
                SL_BYTE:  rdata = {{24{mem_rdata[7]}},  mem_rdata[7:0]};    // lb
                SL_UBYTE: rdata = {24'b0,               mem_rdata[7:0]};    // lbu
                SL_HALF:  rdata = {{16{mem_rdata[15]}}, mem_rdata[15:0]};   // lh
                SL_UHALF: rdata = {16'b0,               mem_rdata[15:0]};   // lhu
                SL_WORD:  rdata = mem_rdata;                                // lw
                default:  rdata = '0;
            endcase
        end
    end
    
    /* 写: pmem_write 的第二个参数是要写的字节数, 小端地从 wdata 取低位字节,
       所以 sb/sh/sw 分别传 1/2/4 即可 */
    always @(posedge clk) begin
        if (wen) begin
            case (wmask)
                SL_BYTE: begin
                    pmem_write(addr, 1, wdata);
                end
                SL_HALF: begin
                    pmem_write(addr, 2, wdata);
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
