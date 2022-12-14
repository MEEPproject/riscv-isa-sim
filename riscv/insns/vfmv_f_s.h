// vfmv_f_s: rd = vs2[0] (rs1=0)
require_vector;
require_fp;
require_extension('F');
require(P_.VU.vsew == e32 || P_.VU.vsew == e64);

reg_t rs2_num = insn.rs2();
uint64_t vs2_0 = 0;
const reg_t sew = P_.VU.vsew;
switch(sew) {
case e32:
  vs2_0 = P_.VU.elt<uint32_t>(rs2_num, 0, VREAD);
  break;
default:
  vs2_0 = P_.VU.elt<uint64_t>(rs2_num, 0, VREAD);
  break;
}

// nan_extened
if (FLEN > sew) {
  vs2_0 = vs2_0 | ~((uint64_t(1) << sew) - 1);
}

if (FLEN == 64) {
  WRITE_FRD(f64(vs2_0));
} else {
  WRITE_FRD(f32(vs2_0));
}
