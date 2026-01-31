module top(
    input a,
    input b,
    input clk,
    input rst,
    output f,
    output [7:0] ld
);

    reg [7:0] led;

    always @(posedge clk, posedge rst) begin
        if(rst)begin
            led[7] <= 1'b1;    
        end else begin
            led[7:0] <= {led[0], led[7:1]};
        end
    end

    assign f = a ^ b;
    assign ld = led;
endmodule