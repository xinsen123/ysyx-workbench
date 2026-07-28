`ifndef CONST_VH
`define CONST_VH  
    //ifdef
    
`define DATA_WIDTH 32 
`define REG_ADDR_WIDTH 4 

`define REG_ALU  2'b00
`define REG_PC   2'b01
`define REG_MEM  2'b10
`define REG_NONE 2'b11

`define SL_UBYTE 4'b0001 
`define SL_BYTE  4'b0001
`define SL_UHALF 4'b0011
`define SL_HALF  4'b0011
`define SL_WORD  4'b1111
`define SL_NONE  4'b0000    

`endif

