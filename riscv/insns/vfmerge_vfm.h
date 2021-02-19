// vfmerge_vf vd, vs2, vs1, vm
require(insn.rd() != 0);
VI_CHECK_SSS(false);
VI_VFP_COMMON;
reg_t sew = P_.VU.vsew;

switch(P_.VU.vsew) {
  case 32:
    for (reg_t i=P_.VU.vstart; i<vl; ++i) {
      auto &vd = P_.VU.elt<float32_t>(rd_num, i);
      auto rs1 = f32(READ_FREG(rs1_num));
      auto vs2 = P_.VU.elt<float32_t>(rs2_num, i);

      int midx = (P_.VU.vmlen * i) / 64;
      int mpos = (P_.VU.vmlen * i) % 64;
      bool use_first = (P_.VU.elt<uint64_t>(0, midx) >> mpos) & 0x1;

      vd = use_first ? rs1 : vs2;
    }
    break;
  case 64:
    for (reg_t i=P_.VU.vstart; i<vl; ++i) {
      auto &vd = P_.VU.elt<float64_t>(rd_num, i);
      auto rs1 = f64(READ_FREG(rs1_num));
      auto vs2 = P_.VU.elt<float64_t>(rs2_num, i);

      int midx = (P_.VU.vmlen * i) / 64;
      int mpos = (P_.VU.vmlen * i) % 64;
      bool use_first = (P_.VU.elt<uint64_t>(0, midx) >> mpos) & 0x1;

      vd = use_first ? rs1 : vs2;
    }
    break;
  default:
    require(0);
    break;
}

P_.VU.vstart = 0;
