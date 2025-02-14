package wrapper_testinpint8;

(*always_ready*)
interface Ifc_testinpint8;
method Bit#(16) getA;
method Bit#(16) getB;
method Bit#(32) getC;
method Bit#(32) getmac;
endinterface

import "BVI" testinpint8 =
module mk_testinpint8(Ifc_testinpint8);
    method outA getA;
    method outB getB;
    method outC getC;
    method mac  getmac;

    default_reset rst(rst);
    default_clock clk(CLK, (*unused*) CLK_GATE);
endmodule
endpackage
