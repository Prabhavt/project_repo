package int8_mul;

//----------------------------------------------Signed 8-bit Pipelined Multiplier---------------------------------------------------------------------------------------------------
  interface Ifc_Mult8;
       method Action put(Bit#(8) a, Bit#(8) b, Bit#(16) c);
       method Bit#(16) get;
  endinterface

  module mk_SignedMul8(Ifc_Mult8);
    
    Ifc_cla         obj_cla         <- mk_CLA;                     //Sub-modules Instantiation
    Ifc_ha          obj_ha          <- mk_HA;
    Ifc_fa          obj_fa          <- mk_FA;

    Reg#(Bit#(8))   rg_a            <- mkReg(?);
    Reg#(Bit#(8))   rg_b            <- mkReg(?);
    Reg#(Bit#(16))  rg_pp[8];
    Reg#(Bit#(16))  rg_stage1_op[5];
    for (Integer i=0; i<5; i=i+1)
      rg_stage1_op[i] <- mkReg(0);
    Reg#(Bit#(16)) rg_stage2_op[3];
    for (Integer i=0; i<3; i=i+1)
      rg_stage2_op[i] <- mkReg(0);
    Reg#(Bit#(16)) rg_stage3_op[2];
    for (Integer i=0; i<2; i=i+1)
      rg_stage3_op[i] <- mkReg(0);
    Reg#(Bit#(16)) rg_stage4_op <- mkReg(?);

    //-----------Test-------------
      Reg#(Bit#(16)) s1_c <-mkReg(?); Reg#(Bit#(8)) s1_a<-mkReg(?); Reg#(Bit#(8)) s1_b<-mkReg(?);
      Reg#(Bit#(16)) s2_c <-mkReg(?); Reg#(Bit#(8)) s2_a<-mkReg(?); Reg#(Bit#(8)) s2_b<-mkReg(?);
      Reg#(Bit#(16)) s3_c <-mkReg(?); Reg#(Bit#(8)) s3_a<-mkReg(?); Reg#(Bit#(8)) s3_b<-mkReg(?);
      Reg#(Bit#(16)) s4_c <-mkReg(?); Reg#(Bit#(8)) s4_a<-mkReg(?); Reg#(Bit#(8)) s4_b<-mkReg(?);
    //---------------------------

   //--------------------------------Stage1: Partial Product Generation-----------------------------------------------------------------------------------
   rule s1;
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
      rg_stage1_op[i] <= stage1_op[i];
    
    s2_a   <= rg_a;
    s2_b   <= rg_b;
    s2_c   <= s1_c;
    endrule

    //------------------------------------------Stage 2: Reduction using CLA------------------------------------------------------------------------------------------------
    rule s2;
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
       rg_stage2_op[i] <= stage2_op[i];

    s3_a <= s2_a;
    s3_b <= s2_b;
    s3_c <= s2_c;   
    endrule
    
    //-------------------------------------------Stage 3: Reduction using HA and FA------------------------------------------------------------------------------------------
    rule s3;
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
        rg_stage3_op[i] <= stage3_op[i];
    
    s4_a <= s3_a;
    s4_b <= s3_b;
    s4_c <= s3_c;
    endrule

    //-------------------------------------------Stage 4: Vector Merge Stage------------------------------------------------------------------------------------------
    rule s4;
    Bit#(16) stage4_op;
    Bit#(4) t = 4'd0;
    stage4_op[3:0]      =   obj_cla.get(rg_stage3_op[0][3:0],rg_stage3_op[1][3:0], 1'b0)[4:1];
    t[0]                =   obj_cla.get(rg_stage3_op[0][3:0],rg_stage3_op[1][3:0], 1'b0)[0];
    stage4_op[7:4]      =   obj_cla.get(rg_stage3_op[0][7:4],rg_stage3_op[1][7:4], t[0])[4:1];
    t[1]                =   obj_cla.get(rg_stage3_op[0][7:4],rg_stage3_op[1][7:4], t[0])[0];
    stage4_op[11:8]     =   obj_cla.get(rg_stage3_op[0][11:8],rg_stage3_op[1][11:8], t[1])[4:1];
    t[2]                =   obj_cla.get(rg_stage3_op[0][11:8],rg_stage3_op[1][11:8], t[1])[0];
    stage4_op[15:12]    =   obj_cla.get({1'b1,rg_stage3_op[0][14:12]},{1'b0,rg_stage3_op[1][14:12]}, t[2])[4:1];
    rg_stage4_op <= stage4_op;

    $display("%d x %d = %d", s4_a,s4_b,stage4_op);
    endrule

    method Action put(Bit#(8) a, Bit#(8) b, Bit#(16) c);
       rg_a <= a;
       rg_b <= b;
       s1_c <= c;
    endmethod

    method Bit#(16) get;
       return rg_stage4_op;
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


module mkTb(Empty);
    
    Reg#(Bit#(8)) a <- mkReg(pack({8'b00001010}));
    Reg#(Bit#(8)) b <- mkReg(pack({8'b00001110}));
    Reg#(int) i <- mkReg(0);

    Ifc_Mult8 obj_mul <- mk_SignedMul8;

    rule start;
        obj_mul.put(a,b,extend(a)*extend(b));
        a<= a+1; b<= b-1;
        i <= i+1;
    endrule

    rule comp (i > 5);
        //Bit#(16) out = obj_mul.get();
        //$display("%d * %d = %d", a, b, out);
    endrule

    rule fin (i==10);
        $finish(0);
    endrule

endmodule




endpackage : int8_mul