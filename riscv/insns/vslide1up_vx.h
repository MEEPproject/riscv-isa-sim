//vslide1up.vx vd, vs2, rs1
require((insn.rs2() & (P_.VU.vlmul - 1)) == 0);
require((insn.rd() & (P_.VU.vlmul - 1)) == 0);
require(insn.rd() != insn.rs2());
if (insn.v_vm() == 0)
  require(insn.rd() != 0);

VI_LOOP_BASE
if (i != 0) {
  if (sew == e8) {
    VI_XI_SLIDEUP_PARAMS(e8, 1);
    vd = vs2;
  } else if(sew == e16) {
    VI_XI_SLIDEUP_PARAMS(e16, 1);
    vd = vs2;
  } else if(sew == e32) {
    VI_XI_SLIDEUP_PARAMS(e32, 1);
    vd = vs2;
  } else if(sew == e64) {
    VI_XI_SLIDEUP_PARAMS(e64, 1);
    vd = vs2;
  }
} else {
  if (sew == e8) {
    P_.VU.elt<uint8_t>(rd_num, 0, VWRITE) = RS1;
  } else if(sew == e16) {
    P_.VU.elt<uint16_t>(rd_num, 0, VWRITE) = RS1;
  } else if(sew == e32) {
    P_.VU.elt<uint32_t>(rd_num, 0, VWRITE) = RS1;
  } else if(sew == e64) {
    P_.VU.elt<uint64_t>(rd_num, 0, VWRITE) = RS1;
  }
}
VI_LOOP_END
