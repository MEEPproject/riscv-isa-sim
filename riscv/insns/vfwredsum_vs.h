// vfwredsum.vs vd, vs2, vs1
require_vector;
require(P_.VU.vsew * 2 <= P_.VU.ELEN);
require((insn.rs2() & (P_.VU.vlmul - 1)) == 0);
require(P_.VU.vlmul <= 4);
VI_VFP_VV_LOOP_WIDE_REDUCTION
({
  vd_0 = f64_add(vd_0, vs2);
})
