
module twoFlopSync(input clk, input rst, input [4:0]in, output reg [4:0]Q);
    reg [4:0] q1;
    //reg [4:0] Q;

    always @(posedge clk or posedge rst)
    begin
        if(rst)
        begin
            q1 <= 0;
            Q  <= 0; 
        end
        else
        begin
            q1 <= in;
            Q  <= q1;
        end
    end
endmodule

