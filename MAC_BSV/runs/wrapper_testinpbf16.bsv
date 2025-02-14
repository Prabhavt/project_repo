package wrapper_testinpbf16;

(*always_ready*)
interface Ifc_testinpbf16;
method Bit#(16) getA;
method Bit#(16) getB;
method Bit#(32) getC;
method Bit#(32) getmac;
endinterface

import "BVI" testinpbf16 =
module mk_testinpbf16(Ifc_testinpbf16);
    method outA getA;
    method outB getB;
    method outC getC;
    method mac  getmac;

    default_reset rst(rst);
    default_clock clk(CLK, (*unused*) CLK_GATE);
endmodule
endpackage
