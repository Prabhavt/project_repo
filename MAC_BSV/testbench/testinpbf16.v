module testinpbf16(input CLK, input rst, input CLK_GATE, input EN, 
                input [10:0] cnt, 
                output reg [15:0]outA, 
                output reg [15:0]outB, 
                output reg [31:0]outC, 
                output reg [31:0]mac 
                );
    integer addr;
    reg [31:0] memA[0:999];
    reg [31:0] memB[0:999];
    reg [31:0] memC[0:999];
    reg [31:0] memmac[0:999];
    
    initial begin 
        addr=32'd0;
        $readmemb("A_bf16binary.txt", memA, 0, 999);
        $readmemb("B_bf16binary.txt", memB, 0, 999);
        $readmemb("C_bf16binary.txt", memC, 0, 999);
        $readmemb("MAC_bf16binary.txt", memmac, 0, 999);
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
