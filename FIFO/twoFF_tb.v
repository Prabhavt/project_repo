module Tb();
    reg [3:0]IN;
    wire [3:0]OUT;
    reg clk, rst;

    twoFlopSync ff(.clk(clk), .rst(rst), .in(IN), .Q(OUT));

    always begin #5 clk=~clk; end  

    initial 
    begin
        $dumpfile("ex.vcd");
        $dumpvars(0,Tb);
        $monitor($time, " %d in: %d  out: %d ",clk, IN,OUT);
        clk=0; rst=0;
        
        #2 rst = 1; #2 rst = 0;
        
        #6 IN= 4'd6;
        #10 IN= 4'd9;
        #10 IN= 4'd12;
        #10 IN= 4'd3;
        #10 IN= 4'd2;
        #10 IN= 4'd5;
        #10 IN= 4'd1;
        #10 IN= 4'd13;
        #10 $finish;
    end
endmodule