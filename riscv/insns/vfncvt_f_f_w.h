// vfncvt.f.f.v vd, vs2, vm
VI_CHECK_SDS(false);
if (P_.VU.vsew == e32)
  require(p->supports_extension('D'));

VI_VFP_LOOP_BASE
  auto vs2 = P_.VU.elt<float64_t>(rs2_num, i, VREAD);
  P_.VU.elt<float32_t>(rd_num, i, VWRITE) = f64_to_f32(vs2);
  set_fp_exceptions;
VI_VFP_LOOP_END
