// vfwcvt.f.f.v vd, vs2, vm
VI_CHECK_DSS(false);
if (P_.VU.vsew == e32)
  require(p->supports_extension('D'));

VI_VFP_LOOP_BASE
  auto vs2 = P_.VU.elt<float32_t>(rs2_num, i);
  P_.VU.elt<float64_t>(rd_num, i) = f32_to_f64(vs2);
  set_fp_exceptions;
VI_VFP_LOOP_WIDE_END
