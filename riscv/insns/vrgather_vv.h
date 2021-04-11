// vrgather.vv vd, vs2, vs1, vm # vd[i] = (vs1[i] >= VLMAX) ? 0 : vs2[vs1[i]];
require((insn.rd() & (P_.VU.vlmul - 1)) == 0);
require((insn.rs2() & (P_.VU.vlmul - 1)) == 0);
require((insn.rs1() & (P_.VU.vlmul - 1)) == 0);
require(insn.rd() != insn.rs2() && insn.rd() != insn.rs1());
if (insn.v_vm() == 0)
  require(insn.rd() != 0);

VI_LOOP_BASE
  switch (sew) {
  case e8: {
    auto vs1 = P_.VU.elt<uint8_t>(rs1_num, i, VREAD);
    //if (i > 255) continue;
    P_.VU.elt<uint8_t>(rd_num, i, VWRITE) = vs1 >= P_.VU.vlmax ? 0 : P_.VU.elt<uint8_t>(rs2_num, vs1, VREAD);
    break;
  }
  case e16: {
    auto vs1 = P_.VU.elt<uint16_t>(rs1_num, i, VREAD);
    P_.VU.elt<uint16_t>(rd_num, i, VWRITE) = vs1 >= P_.VU.vlmax ? 0 : P_.VU.elt<uint16_t>(rs2_num, vs1, VREAD);
    break;
  }
  case e32: {
    auto vs1 = P_.VU.elt<uint32_t>(rs1_num, i, VREAD);
    P_.VU.elt<uint32_t>(rd_num, i, VWRITE) = vs1 >= P_.VU.vlmax ? 0 : P_.VU.elt<uint32_t>(rs2_num, vs1, VREAD);
    break;
  }
  default: {
    auto vs1 = P_.VU.elt<uint64_t>(rs1_num, i, VREAD);
    P_.VU.elt<uint64_t>(rd_num, i, VWRITE) = vs1 >= P_.VU.vlmax ? 0 : P_.VU.elt<uint64_t>(rs2_num, vs1, VREAD);
    break;
  }
  }
VI_LOOP_END;
