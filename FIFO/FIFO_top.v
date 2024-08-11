//`include "memory.v"
//`include "ptr_module.v"
//`include "twoflopsynchronizer.v"

module FIFO_asynch(
  input w_clk, w_rst,
  input r_clk, r_rst,
  input w_en, r_en,
  input [7:0] in,
  output [7:0] out,
  output full, empty
);
  
  wire [4:0] wptr_to_mem, rptr_to_mem;
  wire [4:0] b_wptr, b_rptr;
  wire [4:0] g_wptr, g_rptr;
  wire [3:0] w_addr, r_addr;  
  twoFlopSync sync_write(.clk(r_clk), .rst(r_rst), .in(g_wptr), .Q(wptr_to_mem)); 
  twoFlopSync sync_read(.clk(w_clk), .rst(w_rst), .in(g_rptr), .Q(rptr_to_mem));  
  write_ptr wptr_h(.w_clk(w_clk), .w_rst(w_rst), .w_en(w_en), .read_ptr(rptr_to_mem), .full(full), .writeptr_b(b_wptr), .writeptr_g(g_wptr));
  read_ptr rptr_h(.r_clk(r_clk), .r_rst(r_rst), .r_en(r_en), .write_ptr(wptr_to_mem), .empty(empty), .readptr_b(b_rptr), .readptr_g(g_rptr));
  memory fifom(w_clk, w_en, r_clk, r_en , b_wptr, b_rptr, full,empty, in, out);

endmodule