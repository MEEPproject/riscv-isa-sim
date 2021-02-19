// vrgather.vx vd, vs2, rs1, vm # vd[i] = (rs1 >= VLMAX) ? 0 : vs2[rs1];
require((insn.rd() & (P_.VU.vlmul - 1)) == 0);
require((insn.rs2() & (P_.VU.vlmul - 1)) == 0);
require(insn.rd() != insn.rs2());
if (insn.v_vm() == 0)
  require(insn.rd() != 0);

reg_t rs1 = RS1;

VI_LOOP_BASE
  switch (sew) {
  case e8:
    P_.VU.elt<uint8_t>(rd_num, i) = rs1 >= P_.VU.vlmax ? 0 : P_.VU.elt<uint8_t>(rs2_num, rs1);
    break;
  case e16:
    P_.VU.elt<uint16_t>(rd_num, i) = rs1 >= P_.VU.vlmax ? 0 : P_.VU.elt<uint16_t>(rs2_num, rs1);
    break;
  case e32:
    P_.VU.elt<uint32_t>(rd_num, i) = rs1 >= P_.VU.vlmax ? 0 : P_.VU.elt<uint32_t>(rs2_num, rs1);
    break;
  default:
    P_.VU.elt<uint64_t>(rd_num, i) = rs1 >= P_.VU.vlmax ? 0 : P_.VU.elt<uint64_t>(rs2_num, rs1);
    break;
  }
VI_LOOP_END;
