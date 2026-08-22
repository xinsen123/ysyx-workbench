`include "npc_const.sv"

import "DPI-C" function int pmem_read(int pc);

module ifu import npc_const::*; (
    input  [DATA_WIDTH-1:0] pc,
    output reg [DATA_WIDTH-1:0] inst
);
    
    always_comb begin
        inst = pmem_read(pc);
    end

endmodule
