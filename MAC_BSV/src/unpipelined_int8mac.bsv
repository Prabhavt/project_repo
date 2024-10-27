package unpipelined_int8mac;

//----------------------------------------------Signed 8-bit UnPipelined Multiplier---------------------------------------------------------------------------------------------------
  interface Ifc_Mult8;
       method Action put(Bit#(8) a, Bit#(8) b, Bit#(32) c);
       method Bit#(32) get;
  endinterface

  module mk_SignedMul8(Ifc_Mult8);
    
    Ifc_cla         obj_cla         <- mk_CLA;                     //Sub-modules Instantiation
    Ifc_ha          obj_ha          <- mk_HA;
    Ifc_fa          obj_fa          <- mk_FA;

    Reg#(Bit#(8))   rg_a            <- mkReg(?);
    Reg#(Bit#(8))   rg_b            <- mkReg(?);
    Bit#(16)        rg_pp[8];
    Bit#(16)        rg_stage1_op[5];
    for (Integer i=0; i<5; i=i+1)
      rg_stage1_op[i] =0;
    Bit#(16) rg_stage2_op[3];
    for (Integer i=0; i<3; i=i+1)
      rg_stage2_op[i] =0;
    Bit#(16) rg_stage3_op[2];
    for (Integer i=0; i<2; i=i+1)
      rg_stage3_op[i] =0;
    Bit#(16) rg_stage4_op =0;
    Bit#(32) rg_stage5_op =0;

    //-----------Test-------------
      Reg#(Bit#(32)) s1_c <-mkReg(?); Bit#(8) s1_a =0; Bit#(8) s1_b =0;
      Bit#(32) s2_c =0; Bit#(8) s2_a =0; Bit#(8) s2_b =0;
      Bit#(32) s3_c =0; Bit#(8) s3_a =0; Bit#(8) s3_b =0;
      Bit#(32) s4_c =0; Bit#(8) s4_a =0; Bit#(8) s4_b =0;
      Bit#(32) s5_c =0; Bit#(8) s5_a =0; Bit#(8) s5_b =0;
    //---------------------------

   //--------------------------------Stage1: Partial Product Generation-----------------------------------------------------------------------------------
   Bit#(16) pp[8];
   pp[0] = {8'd0, ~(rg_a[7]&rg_b[0]) ,  rg_a[6:0]& signExtend(rg_b[0])};                        
   pp[1] = {7'd0, ~(rg_a[7]&rg_b[1]) ,  rg_a[6:0]& signExtend(rg_b[1])  ,1'b0};             
   pp[2] = {6'd0, ~(rg_a[7]&rg_b[2]) ,  rg_a[6:0]& signExtend(rg_b[2])  ,2'd0};             
   pp[3] = {5'd0, ~(rg_a[7]&rg_b[3]) ,  rg_a[6:0]& signExtend(rg_b[3])  ,3'd0};            
   pp[4] = {4'd0, ~(rg_a[7]&rg_b[4]) ,  rg_a[6:0]& signExtend(rg_b[4])  ,4'd0};          
   pp[5] = {3'd0, ~(rg_a[7]&rg_b[5]) ,  rg_a[6:0]& signExtend(rg_b[5])  ,5'd0};
   pp[6] = {2'd0, ~(rg_a[7]&rg_b[6]) ,  rg_a[6:0]& signExtend(rg_b[6])  ,6'd0};          
   pp[7] = {1'd0,  (rg_a[7]&rg_b[7]) ,  ~(rg_a[6:0]&signExtend(rg_b[7])),7'd0}; 
   
   //----------------------------------Reduction using CLA----------------------------------------------------------------------------------------------------------
    Bit#(16) stage1_op[5];
    stage1_op[0][5:2]   = obj_cla.get(pp[0][5:2], pp[1][5:2], pp[2][2])[4:1];
    stage1_op[0][6]     = obj_cla.get(pp[0][5:2], pp[1][5:2], pp[2][2])[0];
    stage1_op[1][7:4]   = obj_cla.get(pp[2][7:4], pp[3][7:4], pp[4][4])[4:1];
    stage1_op[1][8]     = obj_cla.get(pp[2][7:4], pp[3][7:4], pp[4][4])[0];
    stage1_op[2][9:6]   = obj_cla.get(pp[4][9:6], pp[5][9:6], pp[0][6])[4:1];
    stage1_op[2][10]    = obj_cla.get(pp[4][9:6], pp[5][9:6], pp[0][6])[0];
    stage1_op[1][13:10] = obj_cla.get(pp[6][13:10], pp[7][13:10], pp[5][10])[4:1];
    stage1_op[1][14]    = obj_cla.get(pp[6][13:10], pp[7][13:10], pp[5][10])[0];
    stage1_op[0][11:8]  = obj_cla.get({pp[4][11],pp[3][10],pp[2][9:8]}, {pp[5][11],pp[4][10],pp[3][9:8]}, pp[1][8])[4:1];
    stage1_op[0][12]    = obj_cla.get({pp[4][11],pp[3][10],pp[2][9:8]}, {pp[5][11],pp[4][10],pp[3][9:8]}, pp[1][8])[0];
    stage1_op[3][9:6]   = obj_cla.get(pp[6][9:6], {pp[7][9:7],pp[1][6]}, 1'b0)[4:1];
    stage1_op[3][10]    = obj_cla.get(pp[6][9:6], {pp[7][9:7],pp[1][6]}, 1'b0)[0];
    stage1_op[0][15:13] = {1'b0, pp[7][14],1'b0};
    stage1_op[0][1:0]   = pp[0][1:0];
    stage1_op[0][7]     = pp[0][7];
    stage1_op[1][3:0]   = {pp[2][3],1'b0, pp[1][1], {1'b0}};
    stage1_op[1][9]     = 1'b0;
    stage1_op[1][15]    = 1'b0;                                                                 
    stage1_op[2][15:11] = {3'b0,pp[5][12],1'b0};
    stage1_op[2][5:0]   = {pp[4][5],1'b0,pp[3][3], 3'b0};
    stage1_op[3][15:11] = 5'b0;
    stage1_op[3][5:0]   = {pp[5][5],5'b0};
    stage1_op[4][15:0]  = {7'b0,1'b1,pp[1][7],7'b0};    // 1'b1 for signed multiplication
    for (Integer i=0; i<5; i=i+1)
      rg_stage1_op[i] = stage1_op[i];
    
    s2_a   = rg_a;
    s2_b   = rg_b;
    s2_c   = s1_c;

    //------------------------------------------Stage 2: Reduction using CLA------------------------------------------------------------------------------------------------
    Bit#(16) stage2_op[3];
    stage2_op[0][6:3]   =   obj_cla.get(rg_stage1_op[0][6:3],rg_stage1_op[1][6:3],rg_stage1_op[2][3])[4:1];
    stage2_op[0][7]     =   obj_cla.get(rg_stage1_op[0][6:3],rg_stage1_op[1][6:3],rg_stage1_op[2][3])[0];
    stage2_op[1][8:5]   =   obj_cla.get({rg_stage1_op[0][8:7],rg_stage1_op[2][6:5]}, {rg_stage1_op[1][8:7], rg_stage1_op[3][6:5]}, 1'b0)[4:1];
    stage2_op[1][9]     =   obj_cla.get({rg_stage1_op[0][8:7],rg_stage1_op[2][6:5]}, {rg_stage1_op[1][8:7], rg_stage1_op[3][6:5]}, 1'b0)[0];
    stage2_op[2][10:7]  =   obj_cla.get(rg_stage1_op[2][10:7], rg_stage1_op[3][10:7], rg_stage1_op[4][7])[4:1];
    stage2_op[2][11]    =   obj_cla.get(rg_stage1_op[2][10:7], rg_stage1_op[3][10:7], rg_stage1_op[4][7])[0];
    stage2_op[0][15:8]  =   {1'b0,rg_stage1_op[0][14],rg_stage1_op[1][13],rg_stage1_op[0][12:9],rg_stage1_op[4][8]};        //rg_stage1_op[4][8] carrying the 1 forward 
    stage2_op[0][2:0]   =   rg_stage1_op[0][2:0];                                                                           // for signed multiplication
    stage2_op[1][4:0]   =   {3'b0, rg_stage1_op[1][1], 1'b0};
    stage2_op[1][15:10] =   {1'b0,rg_stage1_op[1][14],1'b0,rg_stage1_op[1][12:10]};
    stage2_op[2][6:0]   =   7'd0;
    stage2_op[2][15:12] =   {1'b0,2'b0,rg_stage1_op[2][12]};
    for (Integer i=0; i<3; i=i+1)
       rg_stage2_op[i] = stage2_op[i];

    s3_a = s2_a;
    s3_b = s2_b;
    s3_c = s2_c;   
    
    //-------------------------------------------Stage 3: Reduction using HA and FA------------------------------------------------------------------------------------------
    Bit#(16) stage3_op[2];
    stage3_op[0][7]     =   obj_ha.get1(rg_stage2_op[0][7], rg_stage2_op[1][7])[1];
    stage3_op[1][8]     =   obj_ha.get1(rg_stage2_op[0][7], rg_stage2_op[1][7])[0];
    stage3_op[0][8]     =   obj_fa.get2(rg_stage2_op[0][8],rg_stage2_op[1][8],rg_stage2_op[2][8])[1];
    stage3_op[1][9]     =   obj_fa.get2(rg_stage2_op[0][8],rg_stage2_op[1][8],rg_stage2_op[2][8])[0];
    stage3_op[0][9]     =   obj_fa.get2(rg_stage2_op[0][9],rg_stage2_op[1][9],rg_stage2_op[2][9])[1];
    stage3_op[1][10]    =   obj_fa.get2(rg_stage2_op[0][9],rg_stage2_op[1][9],rg_stage2_op[2][9])[0];
    stage3_op[0][10]    =   obj_fa.get2(rg_stage2_op[0][10],rg_stage2_op[1][10],rg_stage2_op[2][10])[1];
    stage3_op[1][11]    =   obj_fa.get2(rg_stage2_op[0][10],rg_stage2_op[1][10],rg_stage2_op[2][10])[0];
    stage3_op[0][11]    =   obj_fa.get2(rg_stage2_op[0][11],rg_stage2_op[1][11],rg_stage2_op[2][11])[1];
    stage3_op[1][12]    =   obj_fa.get2(rg_stage2_op[0][11],rg_stage2_op[1][11],rg_stage2_op[2][11])[0];
    stage3_op[0][12]    =   obj_fa.get2(rg_stage2_op[0][12],rg_stage2_op[1][12],rg_stage2_op[2][12])[1];
    stage3_op[1][13]    =   obj_fa.get2(rg_stage2_op[0][12],rg_stage2_op[1][12],rg_stage2_op[2][12])[0];
    stage3_op[0][15:13] =   {1'b0,rg_stage2_op[0][14:13]};
    stage3_op[0][6:0]   =   rg_stage2_op[0][6:0];
    stage3_op[1][15:14] =   {1'b1,rg_stage2_op[1][14]};   // 1'b1 for signed Mul
    stage3_op[1][7:0]   =   {rg_stage2_op[2][7], rg_stage2_op[1][6:5], 3'b0, rg_stage2_op[1][1], 1'b0};
    for (Integer i=0; i<2; i=i+1)
        rg_stage3_op[i] = stage3_op[i];
    
    s4_a = s3_a;
    s4_b = s3_b;
    s4_c = s3_c;

    //-------------------------------------------Stage 4: Vector Merge Stage------------------------------------------------------------------------------------------

    Bit#(16) stage4_op;
    Bit#(4) t = 4'd0;
    stage4_op[3:0]      =   obj_cla.get(rg_stage3_op[0][3:0],rg_stage3_op[1][3:0], 1'b0)[4:1];
    t[0]                =   obj_cla.get(rg_stage3_op[0][3:0],rg_stage3_op[1][3:0], 1'b0)[0];
    stage4_op[7:4]      =   obj_cla.get(rg_stage3_op[0][7:4],rg_stage3_op[1][7:4], t[0])[4:1];
    t[1]                =   obj_cla.get(rg_stage3_op[0][7:4],rg_stage3_op[1][7:4], t[0])[0];
    stage4_op[11:8]     =   obj_cla.get(rg_stage3_op[0][11:8],rg_stage3_op[1][11:8], t[1])[4:1];
    t[2]                =   obj_cla.get(rg_stage3_op[0][11:8],rg_stage3_op[1][11:8], t[1])[0];
    stage4_op[15:12]    =   obj_cla.get({1'b1,rg_stage3_op[0][14:12]},{1'b0,rg_stage3_op[1][14:12]}, t[2])[4:1];
    rg_stage4_op = stage4_op;
    
    s5_c = s4_c;
    s5_a = s4_a;
    s5_b = s4_b;
    //----------------------------------------------------------------------------------------------------------------------------------
    Bit#(32) axb = signExtend(rg_stage4_op);
    Bit#(32) stage5_op;
    Bit#(8) carry = 8'd0;
    stage5_op[3:0]          =   obj_cla.get(  s5_c[3:0],   axb[3:0],     1'b0)[4:1];
    carry[0]                =   obj_cla.get(  s5_c[3:0],   axb[3:0],     1'b0)[0];
    stage5_op[7:4]          =   obj_cla.get(  s5_c[7:4],   axb[7:4], carry[0])[4:1];
    carry[1]                =   obj_cla.get(  s5_c[7:4],   axb[7:4], carry[0])[0];
    stage5_op[11:8]         =   obj_cla.get( s5_c[11:8],  axb[11:8], carry[1])[4:1];
    carry[2]                =   obj_cla.get( s5_c[11:8],  axb[11:8], carry[1])[0];
    stage5_op[15:12]        =   obj_cla.get(s5_c[15:12], axb[15:12], carry[2])[4:1];
    carry[3]                =   obj_cla.get(s5_c[15:12], axb[15:12], carry[2])[0];
    stage5_op[19:16]        =   obj_cla.get(s5_c[19:16], axb[19:16], carry[3])[4:1];
    carry[4]                =   obj_cla.get(s5_c[19:16], axb[19:16], carry[3])[0];
    stage5_op[23:20]        =   obj_cla.get(s5_c[23:20], axb[23:20], carry[4])[4:1];
    carry[5]                =   obj_cla.get(s5_c[23:20], axb[23:20], carry[4])[0];
    stage5_op[27:24]        =   obj_cla.get(s5_c[27:24], axb[27:24], carry[5])[4:1];
    carry[6]                =   obj_cla.get(s5_c[27:24], axb[27:24], carry[5])[0];
    stage5_op[31:28]        =   obj_cla.get(s5_c[31:28], axb[31:28], carry[6])[4:1];
    carry[7]                =   obj_cla.get(s5_c[31:28], axb[31:28], carry[6])[0];
        
    rg_stage5_op = stage5_op; 

    method Action put(Bit#(8) a, Bit#(8) b, Bit#(32) c);
       rg_a <= a;
       rg_b <= b;
       s1_c <= c;
    endmethod

    method Bit#(32) get;
       return rg_stage5_op;
    endmethod
  endmodule
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------Sub-Modules and Interfaces----------------------------------------------------------------------------------------------------------
  interface Ifc_cla;
   method Bit#(5) get(Bit#(4) rg_a, Bit#(4) rg_b, bit cin);
  endinterface

//-----------------------------------------Module: Carry Lookahead Adder 4-bit-------------------------------------------------------------------------------------------------
  module mk_CLA(Ifc_cla);
  method Bit#(5) get(Bit#(4) rg_a, Bit#(4) rg_b, bit cin);
    Bit#(4) g,p,sum,carrrg_b;
    bit cout;
    g= rg_a&rg_b;
    p= rg_a^rg_b;
    carrrg_b[0]= cin;
    carrrg_b[1]= g[0] | p[0]&cin;
    carrrg_b[2]= g[1] | p[1]&g[0] | p[1]&p[0]&cin;
    carrrg_b[3]= g[2] | p[2]&g[1] | p[2]&p[1]&g[0] | p[2]&p[1]&p[0]&cin;
    cout= g[3] | p[3]&g[2] | p[3]&p[2]&g[1] | p[3]&p[2]&p[1]&g[0] | p[3]&p[2]&p[1]&p[0]&cin; 
    sum = p ^ carrrg_b;
    return {sum, cout};
  endmethod
  endmodule
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  interface Ifc_ha;
   method  Bit#(2) get1(bit a, bit b);
  endinterface
  
//-----------------------------------------Module: Half Adder---------------------------------------------------------------------------------------------------------------------  
  module mk_HA(Ifc_ha);
  method  Bit#(2) get1(bit a, bit b);
    Bit#(2) sum;
    sum[1] = a^b;
    sum[0] = a&b;
    return sum;
  endmethod
  endmodule
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  interface Ifc_fa;
   method Bit#(2) get2(bit a, bit b, bit cin);
  endinterface

//------------------------------------------ Module: Full Adder-------------------------------------------------------------------------------------------------------------------
  module mk_FA(Ifc_fa);
  method Bit#(2) get2(bit a, bit b, bit cin);
   Bit#(2) sum;
   sum[1] = a^b^cin;
   sum[0] = (a&b) | (a^b)&cin;
   return sum;
  endmethod
  endmodule



//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
(*always_ready*)
interface Ifc_testinpint8;
method Action putcnt( Bit#(11) cnt );       //Number of Test Cases
method Bit#(8)  getA;
method Bit#(8)  getB;
method Bit#(32) getC;
method Bit#(32) getmac;
endinterface

import "BVI" testinpint8 =
module mk_testinpint8(Ifc_testinpint8);
    method putcnt( cnt ) enable(EN);
    method outA getA;
    method outB getB;
    method outC getC;
    method mac  getmac;

    default_reset rst(rst);
    default_clock clk(CLK, (*unused*) CLK_GATE);
endmodule

module mkTb(Empty);
        
    Reg#(int) crg_bc        <- mkReg(0);
    Reg#(Bit#(8)) rg_A      <- mkReg(?);
    Reg#(Bit#(8)) rg_B      <- mkReg(?);
    Reg#(Bit#(32)) rg_C     <- mkReg(?);
    Reg#(Bit#(32)) rg_MAC   <- mkReg(?);

    Reg#(Bit#(11)) fail  <- mkReg(0);
    Bit#(11) no_test_case = 11'd1048;

    Ifc_Mult8           obj_mac        <- mk_SignedMul8;
    
    Ifc_testinpint8     inp            <- mk_testinpint8;          
    
    Reg#(Bit#(8))   as1                 <- mkReg(?);
    Reg#(Bit#(8))   bs1                 <- mkReg(?);
    Reg#(Bit#(32))  cs1                 <- mkReg(?);
    Reg#(Bit#(32))  s1_MAC              <- mkReg(?);

    rule start;
        inp.putcnt(no_test_case);
        rg_A      <= inp.getA;
        rg_B      <= inp.getB;
        rg_C      <= inp.getC;
        rg_MAC    <= inp.getmac;
        obj_mac.put(rg_A,rg_B,rg_C);
        crg_bc    <= crg_bc + 1 ;  
    endrule

    rule latency;
    as1     <= rg_A;           
    bs1     <= rg_B;
    cs1     <= rg_C;
    s1_MAC  <= rg_MAC;
    endrule

    rule comp (crg_bc > 1);
        Bit#(32) out = obj_mac.get();
        if(out!=s1_MAC) begin
          $display("%d * %d + %d = %d  expected: %d", as1, bs1, cs1, out, s1_MAC);
          fail <= fail+1;
        end
    endrule

    rule fin (crg_bc == unpack({'0,pack(no_test_case)}));
        $display("PASS: %d  FAIL: %d", pack(no_test_case)-fail, fail);
        $finish(0);
    endrule

endmodule


endpackage 