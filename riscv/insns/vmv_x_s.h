// vmv_x_s: rd = vs2[rs1]
require(insn.v_vm() == 1);
uint64_t xmask = UINT64_MAX >> (64 - P_.get_max_xlen());
reg_t rs1 = RS1;
reg_t sew = P_.VU.vsew;
reg_t rs2_num = insn.rs2();

if (!(rs1 >= 0 && rs1 < (P_.VU.get_vlen() / sew))) {
  WRITE_RD(0);
} else {
  switch(sew) {
  case e8:
    WRITE_RD(P_.VU.elt<int8_t>(rs2_num, rs1, VREAD));
    break;
  case e16:
    WRITE_RD(P_.VU.elt<int16_t>(rs2_num, rs1, VREAD));
    break;
  case e32:
    WRITE_RD(P_.VU.elt<int32_t>(rs2_num, rs1, VREAD));
    break;
  case e64:
    if (P_.get_max_xlen() <= sew)
      WRITE_RD(P_.VU.elt<uint64_t>(rs2_num, rs1, VREAD) & xmask);
    else
      WRITE_RD(P_.VU.elt<uint64_t>(rs2_num, rs1, VREAD));
    break;
  }
}
