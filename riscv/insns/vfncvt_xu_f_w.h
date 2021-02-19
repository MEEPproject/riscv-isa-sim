// vfncvt.xu.f.v vd, vs2, vm
VI_CHECK_SDS(false);
if (P_.VU.vsew == e32)
  require(p->supports_extension('D'));

VI_VFP_LOOP_BASE
  auto vs2 = P_.VU.elt<float64_t>(rs2_num, i);
  P_.VU.elt<uint32_t>(rd_num, i) = f64_to_ui32(vs2, STATE.frm, true);
  set_fp_exceptions;
VI_VFP_LOOP_END
