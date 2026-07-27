`ifndef CONST_VH
`define CONST_VH  
    //ifdef
    
`define DATA_WIDTH 32 
`define REG_ADDR_WIDTH 4 

`define REG_ALU  2'b00
`define REG_PC   2'b01
`define REG_MEM  2'b10
`define REG_NONE 2'b11

`define SL_UBYTE 3'b000 
`define SL_BYTE  3'b001
`define SL_UHALF 3'b010
`define SL_HALF  3'b011
`define SL_WORD  3'b101
`define SL_NONE  3'b111    

`endif

