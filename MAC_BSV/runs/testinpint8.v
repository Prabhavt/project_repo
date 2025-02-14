`define size 1048
module testinpint8(input CLK, input rst, input CLK_GATE, input EN, 
                input [10:0] cnt, 
                output reg [7:0]outA, 
                output reg [7:0]outB, 
                output reg [31:0]outC, 
                output reg [31:0]mac 
                );
    integer addr;
    reg [31:0] memA[0:`size];
    reg [31:0] memB[0:`size];
    reg [31:0] memC[0:`size];
    reg [31:0] memmac[0:`size];
    
    initial begin 
        addr=32'd0;
        $readmemb("A_binaryint8.txt", memA, 0, `size);
        $readmemb("B_binaryint8.txt", memB, 0, `size);
        $readmemb("C_binaryint8.txt", memC, 0, `size);
        $readmemb("MAC_binaryint8.txt", memmac, 0, `size);
    end
    always@(posedge CLK) begin
        if(addr <= cnt) begin
        outA <= memA[addr];
        outB <= memB[addr];
        outC <= memC[addr];
        mac  <= memmac[addr];
        addr= addr+1;
        end
    end
endmodule

// module tb;
//     reg rclk;
//     wire [15:0]outA;
//     wire [15:0]outB;
//     wire [31:0]outC;
//     wire [31:0]mac;

//     testinp dut(.CLK(rclk), .rst(1'd0), .CLK_GATE(1'd0), .cnt(11'd10), .outA(outA), .outB(outB), .outC(outC), .mac(mac));

//     initial begin 
//         rclk=1'b0;
//         $monitor("%b %b %b %b",outA, outB, outC, mac);
//         #1000 $finish(0);
//     end
//     always begin #5 rclk = ~rclk; end

// endmodule
