// vmv_s_x: vd[0] = rs1
require_vector;
require(insn.v_vm() == 1);
require(P_.VU.vsew == e8 || P_.VU.vsew == e16 ||
        P_.VU.vsew == e32 || P_.VU.vsew == e64);
reg_t vl = P_.VU.vl;

if (vl > 0) {
  reg_t rd_num = insn.rd();
  reg_t sew = P_.VU.vsew;

  switch(sew) {
  case e8:
    P_.VU.elt<uint8_t>(rd_num, 0) = RS1;
    break;
  case e16:
    P_.VU.elt<uint16_t>(rd_num, 0) = RS1;
    break;
  case e32:
    P_.VU.elt<uint32_t>(rd_num, 0) = RS1;
    break;
  default:
    P_.VU.elt<uint64_t>(rd_num, 0) = RS1;
    break;
  }

  vl = 0;
}
