                                //Module: Bfloat16 Multiplier and FP32 Adder

package unpipelined_BF16MAC;
import FloatingPoint::*;

//----------------------------------------------------------Bfloat16 Multiplier Main---------------------------------------------------------------------------------------------------------------------------------

interface Ifc_bfloat16_unpipe_main;
    method Bit#(32)     get();
    method Action       put(Bit#(16) a, Bit#(16) b, Bit#(32) c);
endinterface

module mk_bfloat16_unpipe_main(Ifc_bfloat16_unpipe_main);

    Reg#(Bit#(16))      rg_a        <-    mkReg(?);     //rg_a, rg_b contains the two input BFloat16 numbers 
    Reg#(Bit#(16))      rg_b        <-    mkReg(?);     //rg_c is a 32 bit number
    Reg#(Bit#(32))      rg_c        <-    mkReg(?);
    
  //------------------------------------------------------Register Declarations-------------------------------------------------------------------------------------------------- 
  
  //---------------------------------------------Block 1 Registers-------------------------------------------------------------------------------------------------- 
    Bit#(1)       sgn_stage1      ;
    Bit#(8)       mulexpadd_stage1;
    Bit#(2)       exception_s1    ;
    Bit#(32)      rg_c_stage1     ;

  //---------------------------------------------Block 2 Registers-------------------------------------------------------------------------------------------------- 
    Bit#(1)      sgn_stage2       ;
    Bit#(8)      mulexpadd_stage2 ;
    Bit#(2)      exception_s2     ;
    Bit#(32)      rg_c_stage2     ;

    //-------------------------------------------Block 3 Registers-------------------------------------------------------------------------------------------------- 
    Bit#(1)      sgn_stage3       ;
    Bit#(8)      mulexpadd_stage3 ;
    Bit#(2)      exception_s3     ;
    Bit#(32)      rg_c_stage3     ;

    //-------------------------------------------Block 4 Registers-------------------------------------------------------------------------------------------------- 
    Bit#(1)       sgn_stage4      ;
    Bit#(8)       mulexpadd_stage4;
    Bit#(2)       exception_s4    ;
    Bit#(32)      rg_c_stage4     ;

   //--------------------------------------------Block 5 Registers-----------------------------------------------------------------------------------------------------------
    Bit#(32)      rg_greaterOperand;
    Bit#(32)      rg_smallerOperand;

    //----------------------------------Addition Module--Block 6 Registers-----------------------------------------------------------------------------------------------------------
    Bit#(8)      rg_exp_diff_s6;
    Bit#(32)      rg_greaterOperand_stage6;
    Bit#(32)      rg_smallerOperand_stage6;
    Bit#(2)      exception_s6;

    //-------------------------------------------Block 7 Registers------------------------------------------------------------------------------------------------------------
    Bit#(32)      rg_greaterOperand_stage7; 
    Bit#(32)      rg_smallerOperand_stage7; 
    Bit#(8)       rg_exp_diff_s7          ; 
    Bit#(29)      rg_shiftedmantissa      ; 
    Bit#(2)       exception_s7            ; 

    //--------------------------------------------Block 8 Registers-----------------------------------------------------------------------------------------------------------
    Bit#(25)      rg_addedmantissa_stage8;
    Bit#(8)       rg_exponent_stage8     ;
    Bit#(1)       rg_sign_stage8         ;
    Bit#(2)       exception_s8           ;
    Bit#(3)       rg_GRS                 ;
  
    //--------------------------------------------Block 9 Registers-----------------------------------------------------------------------------------------------------------
    Bit#(32)       out;

    //--------------------------------------------Interfaces------------------------------------------------------------------------------------------------------------------
    Ifc_Mult8 obj_unsignedmul                        <-    mk_UnsignedMul8;
    Ifc_lzcounter obj_lzcounter                      <-    mk_lzcounter;


   //---------------------------------------------------------------------------------------------------------------------------------------------------------- 

        Bit#(1) w_sgn_stage1;
        Bit#(8) w_mulexpadd_stage1;

        //Checking corner cases:
        //If any of the operands are (0 x inf) or (inf x 0) or ( NaN x 0 ) or ( 0 x NaN ), 
        //Result will be NaN where exp={'1} and mantissa={1'b1,'0}  
        //Raising exception = 0 for inputs as inf and zero resulting in NaN output.
        if(((rg_a[14:7] == {'1}) && (rg_b[14:7] == {'0} || rg_a[6:0]!={'0})) || (rg_a[14:7] == {'0} && (rg_b[14:7] == {'1} || rg_a[6:0]!={'0} )))
        begin    
            w_sgn_stage1          =   mk_mulsign(rg_a[15],rg_b[15]);
            w_mulexpadd_stage1    =   {'1};
            exception_s1          =   2'd0;
        end
        //If input are 0 x any valid number, result will be true zero
        //Raising exception = 1 for output resulting to true zero
        else if( rg_a[14:7]=={'0} || rg_b[14:7]=={'0} )
        begin
            w_sgn_stage1          =   mk_mulsign(rg_a[15],rg_b[15]);
            w_mulexpadd_stage1    =   {'0};
            exception_s1          =   2'd1;
        end
        //If input are (inf x any valid number), result will be inf
        //Raising excetion = 2 for output resulting to inf
        else if(rg_a[14:7]=={'1} || rg_b[14:7]=={'1})
        begin
            w_sgn_stage1          =   mk_mulsign(rg_a[15],rg_b[15]);
            w_mulexpadd_stage1    =   {'1};
            exception_s1          =   2'd2;
        end
        //If inputs are not satisfying any of the above corner cases then proceed to the next step
        else
        begin
            w_sgn_stage1          =   mk_mulsign(rg_a[15],rg_b[15]);
            w_mulexpadd_stage1    =   mk_mulexpadder(rg_a[14:7],rg_b[14:7]);
            exception_s1          =   2'd3;
        end

        rg_c_stage1               =    rg_c;
        sgn_stage1                =    w_sgn_stage1;
        mulexpadd_stage1          =    w_mulexpadd_stage1;


  //--------------------------------------------------------------------------------------------------------------------------------------------------------- 

        mulexpadd_stage2          =    mulexpadd_stage1;            
        sgn_stage2                =    sgn_stage1;
        exception_s2              =    exception_s1;
        rg_c_stage2               =    rg_c_stage1;


  //--------------------------------------------------------------------------------------------------------------------------------------------------------- 

        mulexpadd_stage3           =    mulexpadd_stage2;
        sgn_stage3                 =    sgn_stage2;
        exception_s3               =    exception_s2;
        rg_c_stage3                =    rg_c_stage2;

  //-------------------------------------------------------------------------------------------------------------------------------------------------------- 
        mulexpadd_stage4           =    mulexpadd_stage3;
        sgn_stage4                 =    sgn_stage3;
        exception_s4               =    exception_s3;
        rg_c_stage4                =    rg_c_stage3;

//----------------------------------------------------------Block 5-------------------------------------------------------------------------------------------------- 

        Bit#(8) w_exp;
        Bit#(23) mantissa;
        Bit#(3) w_GRS_bf16;
        case(exception_s4)        
        2'd3: begin 
                  Bit#(23) w_mantissa_intermediate;
                  Bit#(9) rounded_mantissa_bf16;
                  Bit#(16) usignedmul               = obj_unsignedmul.get();
                  w_exp                             = mulexpadd_stage4;
                  w_GRS_bf16                        = (usignedmul[15]==1'b1) ? {usignedmul[7:6], |usignedmul[5:0]} : { usignedmul[6:5], |usignedmul[4:0] };
                  
                  if(usignedmul[15]==1)
                  begin 
                      w_exp                         = w_exp+1; 
                      rounded_mantissa_bf16         = round_bf16(w_GRS_bf16, {1'b1,usignedmul[14:8]});
                      w_mantissa_intermediate       = {rounded_mantissa_bf16[6:0],'0};
                      w_exp                         = (rounded_mantissa_bf16[8]==1) ? w_exp + 1 : w_exp; 
                  end 
                  else
                  begin  
                      rounded_mantissa_bf16         = round_bf16(w_GRS_bf16, {1'b1,usignedmul[13:7]});
                      w_mantissa_intermediate       = {rounded_mantissa_bf16[6:0],'0};
                      w_exp                         = (rounded_mantissa_bf16[8]==1) ? w_exp + 1 : w_exp; 
                  end
             //If after multiplication result comes out to have exponent={'1}, then we saturate the number to inf 
                  mantissa    =  w_exp != {'1} ? w_mantissa_intermediate : {'0}; 
              end
        2'd2: begin
                  w_exp       =   {'1};
                  mantissa    =   {'0};
              end
        2'd1: begin
             //Result is true zero
                  w_exp       =   {'0};
                  mantissa    =   {'0};
              end
        default: begin
              //Result is NaN 
                    w_exp       =   {'1};
                    mantissa    =   {1'b1,'0}; 
                 end
        endcase

        Bit#(32) mul_result = {sgn_stage4, w_exp, mantissa};
        //Assigning greater of two number wrt exponent to rg_greaterOperand and smaller to rg_smallerOperand 
        rg_greaterOperand             =    mul_result[30:23] >= rg_c_stage4[30:23] ? mul_result : rg_c_stage4;
        rg_smallerOperand             =    mul_result[30:23] >= rg_c_stage4[30:23] ? rg_c_stage4 : mul_result;

//----------------------------------------------------------Block 6--Adder Block-------------------------------------------------------------------------------------------------------------
// Description: Find difference of exponent
// Exceptions : Denormal number is converted to True Zero (sgn,exp,man)={0,0,0} and calculations are done with operand as true zero.
//              NaN and Infinity is preserved. If the number is infinity or NaN, result is infinity. i.e (sgn,exp,man) == {sgn,255,0}.
//              Raise Exception==1 for NaN and inf so that result can be evaluated accordingly in the last stage.
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

        if ( (rg_greaterOperand[30:23] != 8'd255) && (rg_smallerOperand[30:23] != 8'd255) ) 
        begin
            Bit#(8)     w_exp_greaterOperand_stage6      =   rg_greaterOperand[30:23];
            Bit#(8)     w_exp_smallerOperand_stage6      =   rg_smallerOperand[30:23];
            Bit#(8)     w_diff_exp_stage6                =   exponentsub(w_exp_greaterOperand_stage6,w_exp_smallerOperand_stage6); 

            if( (rg_greaterOperand[30:23] != 8'd0) || (rg_smallerOperand[30:23] != 8'd0) ) 
            begin
                rg_greaterOperand_stage6                 =   rg_greaterOperand;
                rg_smallerOperand_stage6                 =   rg_smallerOperand;
            end
            else 
            begin
                //exponent_rg_a == 0   or   exponent_rg_b == 0
                //Converting Denormal numbers to True zero
                rg_greaterOperand_stage6                 =    rg_greaterOperand[22:0] != 23'd0 ? {rg_greaterOperand[31:23], '0} : {rg_greaterOperand[31], '0};
                rg_smallerOperand_stage6                 =    rg_smallerOperand[22:0] != 23'd0 ? {rg_smallerOperand[31:23], '0} : {rg_smallerOperand[31], '0}; 
            end
            rg_exp_diff_s6                               = w_diff_exp_stage6;
            exception_s6                                 = 2'd0;
        end
        else 
        begin
            //Preserving sign
            rg_greaterOperand_stage6                    = (rg_greaterOperand[22:0] != 23'd0) ? {rg_greaterOperand[31], 8'd255, rg_greaterOperand[22:0]} : {rg_greaterOperand[31], 8'd255, 23'd0} ;    
            rg_smallerOperand_stage6                    = (rg_smallerOperand[22:0] != 23'd0) ? {rg_smallerOperand[31], 8'd255, rg_smallerOperand[22:0]} : {rg_smallerOperand[31], 8'd255, 23'd0} ;
            rg_exp_diff_s6                              = 8'd255;

            //exception=1 if number is NaN          
            //exception=2 if number is inf          
            exception_s6                                = (rg_smallerOperand[22:0] != 23'd0) || (rg_greaterOperand[22:0] != 23'd0) ? 2'd1 : 2'd2;
        end

//-------------------------------------------------------------Block 7--------------------------------------------------------------------------------------------------------------
// Description: Shift {1.mantissa} of operand with lower exponent to align decimal point
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

        Bit#(32) w_greaterOperand            =   rg_greaterOperand_stage6;
        Bit#(32) w_smallerOperand            =   rg_smallerOperand_stage6;
        Bit#(8) rightshiftcount              =   rg_exp_diff_s6;
// Shifting mantissa of operand with smaller exponent, taking 24 bit output, as we need information of MSB for mantissa addition
// for example if we shift 1.1001 by 1 place. We will have 0.11001, here we need to know the MSB before it gets added to other mantissa
// 3 extra bit at LSB of mantissa to account for Guard Round Sticky bit        
        Bit#(29) shiftedmantissa             =   rightshift({w_smallerOperand[22:0],5'd0}, rightshiftcount);

        rg_greaterOperand_stage7             =   w_greaterOperand;
        rg_smallerOperand_stage7             =   w_smallerOperand;
        rg_exp_diff_s7                       =   rightshiftcount;
        rg_shiftedmantissa                   =   shiftedmantissa;
        exception_s7                         =   exception_s6;

//-------------------------------------------------------------Block 8--------------------------------------------------------------------------------------------------------------
// Description: Mantissa Add/Subtract on the basis of sign of A and B
//For operands with both sign (+) or (-)           --> {1.mantissa} of greater exp and shifted {1.mantissa} of smaller exp need to be added.
//For one operand with (+) sign and other with (-) --> we have to subtract the significands
//                                                     Before subtraction we assign bigger (on the basis of mantissa) 
//                                                     number as operand 1 and smaller number as operand 2              
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        Bit#(32) w_greaterOp          =   rg_greaterOperand_stage7;
        Bit#(32) w_smallerOp          =   rg_smallerOperand_stage7;
        Bit#(1)  w_mode               =   w_greaterOp[31]^w_smallerOp[31];
        Bit#(29) w_shiftedmantissa    =   rg_shiftedmantissa; 
        Bit#(31) w_MantiAddSub        =   mantiAddSub(w_mode,w_greaterOp[31],{1'b1,w_greaterOp[22:0],5'd0},w_smallerOp[31],w_shiftedmantissa);
        Bit#(25) w_addedmantissa      =   w_MantiAddSub[29:5];
        Bit#(1)  w_sign               =   w_MantiAddSub[30];
        
        //------Guard Round Sticky--------
        rg_GRS                        =   (w_MantiAddSub[29]==1'b1) ? {w_MantiAddSub[4:3], |w_MantiAddSub[2:0]} : {w_MantiAddSub[5:4],|w_MantiAddSub[3:0]};
        
        rg_sign_stage8                =   w_sign;           //sign calcualte       
        rg_exponent_stage8            =   w_greaterOperand[30:23];
        rg_addedmantissa_stage8       =   w_addedmantissa;
        exception_s8                  =   exception_s7;
    
//-------------------------------------------------------------Block 9--------------------------------------------------------------------------------------------------------------
// Description: Aligning mantissa and incrementing exponent if required - normalize
//              If exceptions are raised, we throw out as NaN or inf as per the raised exception.
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

        Bit#(3) w_GRS                   = rg_GRS;

        case (exception_s8) matches
        2'd0 :  
        begin
            if(rg_addedmantissa_stage8[24:23]==0)
            begin 
                Bit#(5) leftshiftcount  =   obj_lzcounter.get(rg_addedmantissa_stage8[22:0]);
                Bit#(8) exponent        =   expIncDec(1'b1, rg_exponent_stage8, leftshiftcount);
                Bit#(23) mantissa       =   leftshift(rg_addedmantissa_stage8[22:0], leftshiftcount);
                mantissa[0]             =   w_GRS[2];
                w_GRS[2]                =   w_GRS[1];
                w_GRS[1]                =   1'b0;
                Bit#(25) rounded_value  =   round_fp32(w_GRS, {1'b1,mantissa});
                mantissa                =   rounded_value[22:0];
                out                     =   {rg_sign_stage8, (rounded_value[24]==1) ? exponent + 1 : exponent, mantissa};
            end
            else if(rg_addedmantissa_stage8[24]==1)
            begin
                Bit#(8) exponent        =   expIncDec(1'b0, rg_exponent_stage8, 5'd1);
                Bit#(23) mantissa       =   rg_addedmantissa_stage8[23:1];
                w_GRS[0]                =   |w_GRS[1:0];
                w_GRS[1]                =   w_GRS[2];
                w_GRS[2]                =   rg_addedmantissa_stage8[0];
                Bit#(25) rounded_value  =   round_fp32(w_GRS, {1'b1,mantissa});
                mantissa                =   rounded_value[24]==1 ? rounded_value[23:1] : rounded_value[22:0];
                out                     =   {rg_sign_stage8, (rounded_value[24]==1) ? exponent + 1 : exponent, mantissa};
            end
            else 
            begin 
                Bit#(8) exponent        =   rg_exponent_stage8;
                Bit#(23) mantissa       =   rg_addedmantissa_stage8[22:0];
                Bit#(25) rounded_value  =   round_fp32(w_GRS, {1'b1,mantissa});
                mantissa                =   rounded_value[24]==1 ? rounded_value[23:1] : rounded_value[22:0];
                out                     =   {rg_sign_stage8, (rounded_value[24]==1) ? exponent + 1 : exponent, mantissa};
            end
        end
        2'd1: 
        begin 
            //Result is NaN
            out       =    {rg_sign_stage8, 8'd255, 1'b1, '0};     
        end
        default: 
        begin
            //Result is inf
            out       =    {rg_sign_stage8, 8'd255, '0}; 
        end 
        endcase

//-------------------------------------------------------GET--------------------------------------------------------------------------------------------------
    method Bit#(32) get();
        return    out;
    endmethod 


//--------------------------------------------------------PUT-------------------------------------------------------------------------------------------------- 
  
    method Action put(Bit#(16) a, Bit#(16) b, Bit#(32) c);
        rg_a              <=    a;
        rg_b              <=    b;
        rg_c              <=    c;
        obj_unsignedmul.put({1'b1,a[6:0]},{1'b1,b[6:0]});
    endmethod

endmodule

function bit mk_mulsign(bit a,bit b);
    return a^b;
endfunction

function Bit#(8) mk_mulexpadder(Bit#(8) a, Bit#(8) b);
    Bit#(8) e= a + b - 8'd127;                  //Bias Subtraction
    if(e>1 && e<254)
        return e[7:0];
    else 
        return 8'd0;                           
endfunction



interface Ifc_Mult8;
    method Action       put(Bit#(8) a, Bit#(8) b);
    method Bit#(16)     get;
endinterface

module mk_UnsignedMul8(Ifc_Mult8) provisos(Bitwise#(Bit#(8)));

    Reg#(Bit#(8))       rg_a <- mkReg(?);
    Reg#(Bit#(8))       rg_b <- mkReg(?);

    //----------------------------------------------------------Block 1 Registers--------------------------------------------------------------------------------------------
    
    Bit#(16) rg_stage1_op[5];
    for (Integer i=0; i<5; i=i+1)
                        rg_stage1_op[i] = 16'd0;

    //-----------------------------------------------------------Block 2 Registers--------------------------------------------------------------------------------------------
    Bit#(16) rg_stage2_op[3];
    for (Integer i=0; i<3; i=i+1)
                        rg_stage2_op[i] = 16'd0;

    //------------------------------------------------------------Block 3 Registers---------------------------------------------------------------------------------------------
    Bit#(16) rg_stage3_op[2];
    for (Integer i=0; i<2; i=i+1)
                        rg_stage3_op[i] = 16'd0;

    //------------------------------------------------------------Block 4 Registers--------------------------------------------------------------------------------------------
    Bit#(16)      rg_stage4_op    = 16'd0;

   //-----------------------------------------------------------Stage1: Partial Product Generation-----------------------------------------------------------------------------------
        Bit#(16)        pp[8];
        Bit#(16)        stage1_op[5];
        Bit#(5)         r1,r2,r3,r4,r5,r6;

        pp[0]   =   {8'd0,rg_a[7:0]& signExtend(rg_b[0])};                        
        pp[1]   =   {7'd0,rg_a[7:0]& signExtend(rg_b[1]),1'b0};             
        pp[2]   =   {6'd0,rg_a[7:0]& signExtend(rg_b[2]),2'd0};             
        pp[3]   =   {5'd0,rg_a[7:0]& signExtend(rg_b[3]),3'd0};            
        pp[4]   =   {4'd0,rg_a[7:0]& signExtend(rg_b[4]),4'd0};          
        pp[5]   =   {3'd0,rg_a[7:0]& signExtend(rg_b[5]),5'd0};
        pp[5]   =   {3'd0,rg_a[7:0]& signExtend(rg_b[5]),5'd0};
        pp[6]   =   {2'd0,rg_a[7:0]& signExtend(rg_b[6]),6'd0};          
        pp[7]   =   {1'd0,rg_a[7:0]& signExtend(rg_b[7]),7'd0}; 

      //------------------------------------------------------------Reduction using CLA----------------------------------------------------------------------------------------------------------
        r1                  =   mk_CLA(pp[0][5:2], pp[1][5:2], pp[2][2]);
        stage1_op[0][5:2]   =   r1[4:1];
        stage1_op[0][6]     =   r1[0];

        r2                  =   mk_CLA(pp[2][7:4], pp[3][7:4], pp[4][4]);
        stage1_op[1][7:4]   =   r2[4:1];
        stage1_op[1][8]     =   r2[0];

        r3                  =   mk_CLA(pp[4][9:6], pp[5][9:6], pp[0][6]);
        stage1_op[2][9:6]   =   r3[4:1];
        stage1_op[2][10]    =   r3[0];

        r4                  =   mk_CLA(pp[6][13:10], pp[7][13:10], pp[5][10]);
      stage1_op[1][13:10]   =   r4[4:1];
      stage1_op[1][14]      =   r4[0];

        r5                  =   mk_CLA({pp[4][11],pp[3][10],pp[2][9:8]}, {pp[5][11],pp[4][10],pp[3][9:8]}, pp[1][8]);
        stage1_op[0][11:8]  =   r5[4:1];
        stage1_op[0][12]    =   r5[0];

        r6                  =   mk_CLA(pp[6][9:6], {pp[7][9:7],pp[1][6]}, 1'b0);
        stage1_op[3][9:6]   =   r6[4:1];
        stage1_op[3][10]    =   r6[0];

        stage1_op[0][15:13] =   {1'b0, pp[7][14],1'b0};
        stage1_op[0][1:0]   =   pp[0][1:0];
        stage1_op[0][7]     =   pp[0][7];
        stage1_op[1][3:0]   =   {pp[2][3],1'b0, pp[1][1], {1'b0}};
        stage1_op[1][9]     =   1'b0;
        stage1_op[1][15]    =   1'b0;                                                                 
        stage1_op[2][15:11] =   {3'b0,pp[5][12],1'b0};
        stage1_op[2][5:0]   =   {pp[4][5],1'b0,pp[3][3], 3'b0};
        stage1_op[3][15:11] =   5'b0;
        stage1_op[3][5:0]   =   {pp[5][5],5'b0};
        stage1_op[4][15:0]  =   {7'b0,1'b0,pp[1][7],7'b0}; 
        for (Integer i=0; i<5; i=i+1)
            rg_stage1_op[i] = stage1_op[i];

   //-----------------------------------------------------------Block 2: Reduction using CLA------------------------------------------------------------------------------------------------

        Bit#(16)        stage2_op[3];
        Bit#(5)         o1,o2,o3;

        o1                  =   mk_CLA(rg_stage1_op[0][6:3],rg_stage1_op[1][6:3],rg_stage1_op[2][3]);
        stage2_op[0][6:3]   =   o1[4:1];
        stage2_op[0][7]     =   o1[0];

        o2                  =   mk_CLA({rg_stage1_op[0][8:7],rg_stage1_op[2][6:5]}, {rg_stage1_op[1][8:7], rg_stage1_op[3][6:5]}, 1'b0);
        stage2_op[1][8:5]   =   o2[4:1];
        stage2_op[1][9]     =   o2[0];

        o3                  =   mk_CLA(rg_stage1_op[2][10:7], rg_stage1_op[3][10:7], rg_stage1_op[4][7]);
        stage2_op[2][10:7]  =   o3[4:1];
        stage2_op[2][11]    =   o3[0];

        stage2_op[0][15:8]  =   {1'b0,rg_stage1_op[0][14],rg_stage1_op[1][13],rg_stage1_op[0][12:9],rg_stage1_op[4][8]};
        stage2_op[0][2:0]   =   rg_stage1_op[0][2:0];
        stage2_op[1][4:0]   =   {3'b0, rg_stage1_op[1][1], 1'b0};
        stage2_op[1][15:10] =   {1'b0,rg_stage1_op[1][14],1'b0,rg_stage1_op[1][12:10]};
        stage2_op[2][6:0]   =   7'd0;
        stage2_op[2][15:12] =   {1'b0,2'b0,rg_stage1_op[2][12]};
        for (Integer i=0; i<3; i=i+1)
            rg_stage2_op[i] = stage2_op[i];
    
   //-------------------------------------------------------------Block 3: Reduction using HA and FA------------------------------------------------------------------------------------------
        Bit#(16)        stage3_op[2];
        Bit#(2)         s1,s2,s3,s4,s5,s6;

        s1                  =   mk_HA(rg_stage2_op[0][7], rg_stage2_op[1][7]);
        stage3_op[0][7]     =   s1[1];
        stage3_op[1][8]     =   s1[0];

        s2                  =   mk_FA(rg_stage2_op[0][8],rg_stage2_op[1][8],rg_stage2_op[2][8]);
        stage3_op[0][8]     =   s2[1];
        stage3_op[1][9]     =   s2[0];

        s3                  =   mk_FA(rg_stage2_op[0][9],rg_stage2_op[1][9],rg_stage2_op[2][9]);
        stage3_op[0][9]     =   s3[1];
        stage3_op[1][10]    =   s3[0];

        s4                  =   mk_FA(rg_stage2_op[0][10],rg_stage2_op[1][10],rg_stage2_op[2][10]);
        stage3_op[0][10]    =   s4[1];
        stage3_op[1][11]    =   s4[0];

        s5                  =   mk_FA(rg_stage2_op[0][11],rg_stage2_op[1][11],rg_stage2_op[2][11]);
        stage3_op[0][11]    =   s5[1];
        stage3_op[1][12]    =   s5[0];

        s6                  =   mk_FA(rg_stage2_op[0][12],rg_stage2_op[1][12],rg_stage2_op[2][12]);
        stage3_op[0][12]    =   s6[1];
        stage3_op[1][13]    =   s6[0];

        stage3_op[0][15:13] = {1'b0,rg_stage2_op[0][14:13]};
        stage3_op[0][6:0]   = rg_stage2_op[0][6:0];
        stage3_op[1][15:14] = {1'b0,rg_stage2_op[1][14]};
        stage3_op[1][7:0]   = {rg_stage2_op[2][7], rg_stage2_op[1][6:5], 3'b0, rg_stage2_op[1][1], 1'b0};
        for (Integer i=0; i<2; i=i+1)
            rg_stage3_op[i] = stage3_op[i];

    //--------------------------------------------------------------Block 4: Vector Merge Block------------------------------------------------------------------------------------------
        Bit#(16)        stage4_op;
        Bit#(4)         t = 4'd0;
        Bit#(5)         c1,c2,c3,c4;

        c1                  =   mk_CLA(rg_stage3_op[0][3:0],rg_stage3_op[1][3:0], 1'b0);
        stage4_op[3:0]      =   c1[4:1];
        t[0]                =   c1[0];

        c2                  =   mk_CLA(rg_stage3_op[0][7:4],rg_stage3_op[1][7:4], t[0]);
        stage4_op[7:4]      =   c2[4:1];
        t[1]                =   c2[0];

        c3                  =   mk_CLA(rg_stage3_op[0][11:8],rg_stage3_op[1][11:8], t[1]);
        stage4_op[11:8]     =   c3[4:1];
        t[2]                =   c3[0];

        c4                  =   mk_CLA({1'b0,rg_stage3_op[0][14:12]},{1'b0,rg_stage3_op[1][14:12]}, t[2]);
        stage4_op[15:12]    =   c4[4:1];

        rg_stage4_op        = stage4_op;

   //--------------------------------------------------------------------PUT--------------------------------------------------------------------------------------------------
    method Action put(Bit#(8) a, Bit#(8) b);
        rg_a            <=    a;
        rg_b            <=    b;
    endmethod

   //--------------------------------------------------------------------GET----------------------------------------------------------------------------------------------------------
    method Bit#(16) get;
        return rg_stage4_op;
    endmethod

endmodule
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------4-bit Carry Lookahead Adder Block-------------------------------------------------------------------------------------------------

function Bit#(5) mk_CLA(Bit#(4) rg_a, Bit#(4) rg_b, bit cin);
    Bit#(4)             g,p,sum,carrrg_b;
    bit                 cout;
    g               =   rg_a&rg_b;
    p               =   rg_a^rg_b;
    carrrg_b[0]     =   cin;
    carrrg_b[1]     =   g[0] | p[0]&cin;
    carrrg_b[2]     =   g[1] | p[1]&g[0] | p[1]&p[0]&cin;
    carrrg_b[3]     =   g[2] | p[2]&g[1] | p[2]&p[1]&g[0] | p[2]&p[1]&p[0]&cin;
    cout            =   g[3] | p[3]&g[2] | p[3]&p[2]&g[1] | p[3]&p[2]&p[1]&g[0] | p[3]&p[2]&p[1]&p[0]&cin; 
    sum             =   p ^ carrrg_b;
    return    {sum, cout};
endfunction
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------ Half Adder Block---------------------------------------------------------------------------------------------------------------------  
  
function Bit#(2) mk_HA(bit a, bit b);
    Bit#(2) sum;
    sum[1] = a^b;
    sum[0] = a&b;
    return sum;
endfunction
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------Full Adder Block-------------------------------------------------------------------------------------------------------------------

function Bit#(2) mk_FA(bit a, bit b, bit cin);
   Bit#(2) sum;
   sum[1] = a^b^cin;
   sum[0] = (a&b) | (a^b)&cin;
   return sum;
endfunction
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------ FP32 Adder Sub Blocks-------------------------------------------------------------------------------------------------------------------

//-------------------------------------------Exponent Increment/Decrement Block--------------------------------------------------------------------------------------------------------------
function Bit#(8) expIncDec(Bit#(1) mode, Bit#(8) a, Bit#(5) b);
    Bit#(8)   out;
    Bit#(8)   op1   =   extend(a);
    Bit#(8)   op2   =   extend(b);
    
    out   =   ( mode == 0 ) ? op1 + op2 : op1 - op2;
    return    out;
endfunction

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------Mantissa Adder/Subtractor--------------------------------------------------------------------------------------------------------------
//Description:  Mode= (sgn_a xor sgn_b), i.e for (Mode==1) addition and subtraction we are adding the mantissa preserving the sign
//              for (Mode==0) we subtract the numbers, which can lead to sign change. So this module will return {sign+mantissa}
//              Operand a is already the bigger of two number, compared on the basis of its exponent.
//              If exp_a and exp_b are same, there could be a case where mantissa_a - mantissa_b can lead to sign change
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

function Bit#(31) mantiAddSub(Bit#(1) mode, Bit#(1) sgn_a, Bit#(29) a, Bit#(1) sgn_b, Bit#(29) b);
    Bit#(30)    out;
    Bit#(30)    extended_a    =   (a > b) ? zeroExtend(a) : zeroExtend(b);
    Bit#(30)    extended_b    =   (a > b) ? zeroExtend(b) : zeroExtend(a);
    Bit#(1)     sign          =   (a > b) ? sgn_a : sgn_b;
    
    out   =   ( mode == 0 ) ? extended_a + extended_b : extended_a - extended_b; 
    return    {sign,out[29:0]};

endfunction

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------Mantissa Left Shift----------------------------------------------------------------------------------------------------------------
//Description: After Addition/Subtraction of Mantissa of both operand, we can get result as 0.0001xxx. 
//             Here we need to align the decimal point to find the mantissa as per IEEE 754 format.
//             Amount of shift is determined by (leading zero count + 1)
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
function Bit#(23) leftshift(Bit#(23) in, Bit#(5) shift);
    return    in << shift;
endfunction

//------------------------------------------Mantissa right shift--------------------------------------------------------------------------------------------------------------
//Description: For floating point addition we need to align the decimal point. We shift the operand {1.mantissa} by (exp_greater - exp_smaller)
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
function Bit#(29) rightshift(Bit#(28) in, Bit#(8) shift);
    return    {1'b1, in} >> shift;
endfunction

//------------------------------------------Exponent Subtract-------------------------------------------------------------------------------------------------------
//Description: To align decimal point for floating point addition/subtraction, we need difference of exponent
//             on the basis of which we shift the {1.mantissa} of operant with smaller exponent
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

function Bit#(8) exponentsub(Bit#(8) exp_a, Bit#(8) exp_b);
        return    exp_a-exp_b;
endfunction

//------------------------------------------Leading Zero Counter--------------------------------------------------------------------------------------------------------------
//Description: After Mantissa addition/subtraction we need to align the result as 1.xxx according to IEEE 754 FP format
//             In this block we use a combinational circuit to determine the leading zero count.
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
function Bit#(2) encode(Bit#(2) in);
    return    (in[1]==1) ? 2'b00 : ((in[0]==1) ? 2'b01 : 2'b10 );
endfunction

interface Ifc_lzcounter;
    method    Bit#(5) get(Bit#(23) mantissa);
endinterface

module mk_lzcounter(Ifc_lzcounter);
    method Bit#(5) get(Bit#(23) mantissa);
    Bit#(32)       x              =   {mantissa[22:0], '0};
    Bit#(32)       twobytwo       =   ?;
    Bit#(24)       assemble_s1    =   ?;
    Bit#(16)       assemble_s2    =   ?;
    Bit#(10)       assemble_s3    =   ?;
    Bit#(5)        out;
    
    for(int i=0; i<=30; i=i+2) begin
        twobytwo[i+1:i]= encode(x[i+1:i]);
    end
     assemble_s1[23:21]   =     (twobytwo[31]&twobytwo[29])== 1 ? 3'b100 : ( (~twobytwo[31]==1) ? {1'b0,twobytwo[31:30]} : {2'b01,twobytwo[28]});
     assemble_s1[20:18]   =     (twobytwo[27] & twobytwo[25])== 1 ? 3'b100 : ( (~twobytwo[27] == 1) ? {1'b0,twobytwo[27:26]} : {2'b01,twobytwo[24]});
     assemble_s1[17:15]   =     (twobytwo[23] & twobytwo[21]) == 1 ? 3'b100 : ( (~twobytwo[23] == 1) ? {1'b0,twobytwo[23:22]} : {2'b01,twobytwo[20]});
     assemble_s1[14:12]   =     (twobytwo[19] & twobytwo[17]) == 1 ? 3'b100 : ( (~twobytwo[19] == 1) ? {1'b0,twobytwo[19:18]} : {2'b01,twobytwo[16]});
     assemble_s1[11:9]    =     (twobytwo[15] & twobytwo[13]) == 1 ? 3'b100 : ( (~twobytwo[15] == 1) ? {1'b0,twobytwo[15:14]} : {2'b01,twobytwo[12]});
     assemble_s1[8:6]     =     (twobytwo[11] & twobytwo[9])== 1 ? 3'b100 : ( (~twobytwo[11] == 1) ? {1'b0,twobytwo[11:10]} : {2'b01,twobytwo[8]});
     assemble_s1[5:3]     =     (twobytwo[7] & twobytwo[5])== 1 ? 3'b100 : ( (~twobytwo[7] == 1) ? {1'b0,twobytwo[7:6]} : {2'b01,twobytwo[4]});
     assemble_s1[2:0]     =     (twobytwo[3] & twobytwo[1])== 1 ? 3'b100 : ( (~twobytwo[3] == 1) ? {1'b0,twobytwo[3:2]} : {2'b01,twobytwo[0]});
    
     assemble_s2[15:12]   =     (assemble_s1[23] & assemble_s1[20]) == 1 ? 4'b1000 : ((~assemble_s1[23] == 1) ? {1'b0,assemble_s1[23:21]} : {2'b01,assemble_s1[19:18]});
     assemble_s2[11:8]    =     (assemble_s1[17] & assemble_s1[14]) == 1 ? 4'b1000 : ((~assemble_s1[17] == 1) ? {1'b0,assemble_s1[17:15]} : {2'b01,assemble_s1[13:12]});
     assemble_s2[7:4]     =     (assemble_s1[11] & assemble_s1[8]) == 1 ? 4'b1000 : ((~assemble_s1[11] == 1) ? {1'b0,assemble_s1[11:9]} : {2'b01,assemble_s1[7:6]});
     assemble_s2[3:0]     =     (assemble_s1[5] & assemble_s1[2]) == 1 ? 4'b1000 : ((~assemble_s1[5] == 1) ? {1'b0,assemble_s1[5:3]} : {2'b01,assemble_s1[1:0]});

     assemble_s3[9:5]     =     (assemble_s2[15] & assemble_s2[11]) == 1 ? 5'b10000 : ((~assemble_s2[15] == 1) ? {1'b0,assemble_s2[15:12]} : {2'b01,assemble_s2[10:8]});
     assemble_s3[4:0]     =     (assemble_s2[7] & assemble_s2[3]) == 1 ? 5'b10000 : ((~assemble_s2[7] == 1) ? {1'b0,assemble_s2[7:4]} : {2'b01,assemble_s2[2:0]});

     out    =   (assemble_s3[9] & assemble_s3[4]) == 1 ? 5'b00000 : ((~assemble_s3[9] == 1) ? assemble_s3[9:5] : {1'b1,assemble_s3[3:0]});
    //We add 1 to account for total left shift of mantissa
    
    return out+1;               
    
    endmethod 
endmodule
//--------------------------------------------------Rounding Block--------------------------------------------------------
function Bit#(9) round_bf16(Bit#(3) grs, Bit#(8) mantissa);
    Bool incr = False;
    if( ((grs[2]==1) && ((|grs[1:0]==1) || mantissa[0]==1)) ) begin
    mantissa = mantissa + 1;
    incr=True;
    end
    if(incr==False) begin
      return  {1'b0, mantissa};                                          //xxx
    end
    else begin
      return ((mantissa-1)== 8'hff) ? {1'b1,mantissa} : {1'b0, mantissa};
    end                                                                                                        // {1,mantissa} -> 1 is a signal to increment exp
endfunction

function Bit#(25) round_fp32(Bit#(3) grs, Bit#(24)mantissa);
    Bool incr = False;
    if( ((grs[2]==1) && ((|grs[1:0]==1) || mantissa[0]==1)) ) begin
    mantissa = mantissa + 1;
    incr=True;
    end
    if(incr==False) begin
      return {1'b0, mantissa};
    end
    else begin
      return ((mantissa-1)== 24'hffffff) ? {1'b1,mantissa} : {1'b0, mantissa};
    end                  // { 1, mantissa } -> here 1 is a signal to increment exponent
endfunction

//--------------------------------------------------------------END--------------------------------------------------------------------------------------------------------------

(*always_ready*)
interface Ifc_testinpbf16;
method Action putcnt( Bit#(11) cnt );       //Number of Test Cases
method Bit#(16) getA;
method Bit#(16) getB;
method Bit#(32) getC;
method Bit#(32) getmac;
endinterface

import "BVI" testinpbf16 =
module mk_testinpbf16(Ifc_testinpbf16);
    method putcnt( cnt ) enable(EN);
    method outA getA;
    method outB getB;
    method outC getC;
    method mac getmac;

    default_reset rst(rst);
    default_clock clk(CLK, (*unused*) CLK_GATE);

endmodule


//--------------------------------------------------------------Test Bench--------------------------------------------------------------------------------------------------------------

module mkTb(Empty);
    Reg#(int) crg_bc <- mkReg(0);
    
    Reg#(Bit#(16)) rg_A_bf16 <- mkReg(?);
    Reg#(Bit#(16)) rg_B_bf16 <- mkReg(?);
    Reg#(Bit#(32)) rg_C_fp32 <- mkReg(?);
    Reg#(Bit#(32)) rg_MAC    <- mkReg(?);

    Reg#(Bit#(11)) fail <- mkReg(0);
    Bit#(11) no_test_case = 11'd999;

    Ifc_testinpbf16 inp                <- mk_testinpbf16;
    Ifc_bfloat16_unpipe_main mac       <- mk_bfloat16_unpipe_main;          
    
        Reg#(FloatingPoint#(8,23)) as1 <- mkReg(?);
        Reg#(FloatingPoint#(8,23)) bs1 <- mkReg(?);
        Reg#(FloatingPoint#(8,23)) cs1 <- mkReg(?);
        Reg#(Bit#(32)) s1_MAC          <- mkReg(?);
    rule rl_c;
            inp.putcnt(no_test_case);
            rg_A_bf16 <= inp.getA;
            rg_B_bf16 <= inp.getB;
            rg_C_fp32 <= inp.getC;
            rg_MAC    <= inp.getmac;

            mac.put(rg_A_bf16, rg_B_bf16, rg_C_fp32);
     crg_bc <= crg_bc + 1 ;    
    endrule 

    rule pipe;
    as1     <= unpack({rg_A_bf16,16'd0});           
    bs1     <= unpack({rg_B_bf16,16'd0});
    cs1     <= unpack(rg_C_fp32);
    s1_MAC  <= rg_MAC;
    endrule

    rule rl_finish;
       Bit#(32) mac_result = mac.get();
       Bit#(32) sim_result= s1_MAC;
       if(crg_bc > 1) begin
       Bit#(24) result_mantissa_diff = ( {1'b1,pack(sim_result)[22:0]} > {1'b1,mac_result[22:0]} ) ? {1'b1,pack(sim_result)[22:0]} - {1'b1,mac_result[22:0]} : {1'b1,mac_result[22:0]} - {1'b1,pack(sim_result)[22:0]} ;
       if ((mac_result[30:23] != pack(sim_result)[30:23])                               //exponent check 
                || (result_mantissa_diff > 24'd1)                                       //Mantissa check ( 2 bit LSB error margin )
                && (mac_result[30:23] != 8'b11111111) )                                 // Result == denormal
       begin
         $display("%d inpA: %b inpB: %b inpC: %b \n op: %be^%b expected: %be^%b",crg_bc-1,pack(as1)[31:16],pack(bs1)[31:16],cs1,{1'b1,mac_result[22:0]}, mac_result[30:23] -8'd127, {1'b1,pack(sim_result)[22:0]}, pack(sim_result)[30:23] - 8'd127); 
         fail<=fail+1;
       end
       else begin 
         $display("%d inpA: %b inpB: %b inpC: %b \n op: %be^%b expected: %be^%b",crg_bc-1,pack(as1)[31:16],pack(bs1)[31:16],cs1,{1'b1,mac_result[22:0]}, mac_result[30:23]-8'd127, {1'b1,pack(sim_result)[22:0]}, pack(sim_result)[30:23] - 8'd127); 
       end
       end

    if(crg_bc==1+unpack({21'd0,no_test_case})) begin $display("%d PASSED %d FAILED", unpack(no_test_case)-fail, fail); $finish(0); end
    endrule 
 endmodule

endpackage