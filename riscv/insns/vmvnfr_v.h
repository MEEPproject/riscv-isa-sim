// vmv1r.v vd, vs2
require_vector;
const reg_t baseAddr = RS1;
const reg_t vd = insn.rd();
const reg_t vs2 = insn.rs2();
const reg_t len = insn.rs1() + 1;
require((vd & (len - 1)) == 0);
require((vs2 & (len - 1)) == 0);
if (vd != vs2)
  memcpy(&P_.VU.elt<uint8_t>(vd, 0, VWRITE),
         &P_.VU.elt<uint8_t>(vs2, 0, VREAD), P_.VU.vlenb * len);
P_.VU.vstart = 0;
