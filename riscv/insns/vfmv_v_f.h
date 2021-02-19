// vfmv_vf vd, vs1
require((insn.rd() & (P_.VU.vlmul - 1)) == 0);
VI_VFP_COMMON
switch(P_.VU.vsew) {
  case e32:
    for (reg_t i=P_.VU.vstart; i<vl; ++i) {
      auto &vd = P_.VU.elt<float32_t>(rd_num, i);
      auto rs1 = f32(READ_FREG(rs1_num));

      vd = rs1;
    }
    break;
  case e64:
    for (reg_t i=P_.VU.vstart; i<vl; ++i) {
      auto &vd = P_.VU.elt<float64_t>(rd_num, i);
      auto rs1 = f64(READ_FREG(rs1_num));

      vd = rs1;
    }
    break;
}

P_.VU.vstart = 0;
