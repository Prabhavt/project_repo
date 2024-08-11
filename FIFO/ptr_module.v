module write_ptr(input w_clk, input w_rst, input w_en, input [4:0] read_ptr,
       output reg full, output reg [4:0]writeptr_b, output reg [4:0]writeptr_g);
    
       wire [4:0] next_writeptr_b;
       wire [4:0] next_writeptr_g;  
       wire w_full;
       reg fullcondition;

       assign next_writeptr_b = writeptr_b + (w_en & ~full);
       assign next_writeptr_g = next_writeptr_b ^ ( next_writeptr_b >> 1);
       assign w_full= (next_writeptr_g == {~read_ptr[4:3], read_ptr[2:0]});
       
       always@(posedge w_clk or posedge w_rst)
       begin  
              if(w_rst)
              begin
                     full <= 0;
                     writeptr_b <= 0;
                     writeptr_g <= 0;
              end
              else 
              begin
                     full <= w_full;
                     writeptr_b <= next_writeptr_b;
                     writeptr_g <= next_writeptr_g;
              end
       end    
endmodule

module read_ptr(input r_clk, input r_rst, input r_en, input [4:0] write_ptr,
       output reg empty, output reg [4:0]readptr_b, output reg [4:0]readptr_g);
    
       wire [4:0] next_readptr_b;
       wire [4:0] next_readptr_g;  
       wire w_empty;
       reg emptycondition;

       assign next_readptr_b = readptr_b + (r_en & ~empty);
       assign next_readptr_g = next_readptr_b ^ ( next_readptr_b >> 1);
       assign w_empty = (write_ptr == next_readptr_g);
       
       always@(posedge r_clk or posedge r_rst)
       begin  
              if(r_rst)
              begin
                     empty <= 1;
                     readptr_b <= 0;
                     readptr_g <= 0;
              end
              else 
              begin
                     empty <= w_empty;
                     readptr_b <= next_readptr_b;
                     readptr_g <= next_readptr_g;
              end
       end    
endmodule