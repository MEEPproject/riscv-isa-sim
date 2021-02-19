// vmpopc rd, vs2, vm
require(P_.VU.vsew >= e8 && P_.VU.vsew <= e64);
require_vector;
reg_t vl = P_.VU.vl;
reg_t sew = P_.VU.vsew;
reg_t rd_num = insn.rd();
reg_t rs1_num = insn.rs1();
reg_t rs2_num = insn.rs2();
require((rd_num & (P_.VU.vlmul - 1)) == 0);
if (insn.v_vm() == 0 && P_.VU.vlmul >= 2) \
  require(insn.rd() != 0);

for (reg_t i = P_.VU.vstart ; i < P_.VU.vl; ++i) {
  VI_LOOP_ELEMENT_SKIP();

  switch (sew) {
  case e8:
    P_.VU.elt<uint8_t>(rd_num, i) = i;
    break;
  case e16:
    P_.VU.elt<uint16_t>(rd_num, i) = i;
    break;
  case e32:
    P_.VU.elt<uint32_t>(rd_num, i) = i;
    break;
  default:
    P_.VU.elt<uint64_t>(rd_num, i) = i;
    break;
  }
}

P_.VU.vstart = 0;
