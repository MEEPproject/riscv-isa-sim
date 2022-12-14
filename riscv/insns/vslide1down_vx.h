//vslide1down.vx vd, vs2, rs1
require((insn.rs2() & (P_.VU.vlmul - 1)) == 0);
require((insn.rd() & (P_.VU.vlmul - 1)) == 0);
if (P_.VU.vlmul > 1 && insn.v_vm() == 0)
  require(insn.rd() != 0);

VI_LOOP_BASE
if (i != vl - 1) {
  switch (sew) {
  case e8: {
    VI_XI_SLIDEDOWN_PARAMS(e8, 1);
    vd = vs2;
  }
  break;
  case e16: {
    VI_XI_SLIDEDOWN_PARAMS(e16, 1);
    vd = vs2;
  }
  break;
  case e32: {
    VI_XI_SLIDEDOWN_PARAMS(e32, 1);
    vd = vs2;
  }
  break;
  default: {
    VI_XI_SLIDEDOWN_PARAMS(e64, 1);
    vd = vs2;
  }
  break;
  }
} else {
  switch (sew) {
  case e8:
    P_.VU.elt<uint8_t>(rd_num, vl - 1, VWRITE) = RS1;
    break;
  case e16:
    P_.VU.elt<uint16_t>(rd_num, vl - 1, VWRITE) = RS1;
    break;
  case e32:
    P_.VU.elt<uint32_t>(rd_num, vl - 1, VWRITE) = RS1;
    break;
  default:
    P_.VU.elt<uint64_t>(rd_num, vl - 1, VWRITE) = RS1;
    break;
  }
}
VI_LOOP_END
