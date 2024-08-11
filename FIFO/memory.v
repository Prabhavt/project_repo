module memory(input w_clk, input w_en, input r_clk, input r_en, 
            input [4:0]w_ptr, input [4:0]r_ptr,
            input full, input empty,
            input [7:0] inp,
            output [7:0] data_out
            );

    reg [7:0]mem[0:15];
    always@(posedge w_clk)
    begin
        if(w_en & ~full) begin
        mem[w_ptr[3:0]] <= inp; 
    end
    end
    assign data_out= mem[r_ptr[3:0]];
endmodule