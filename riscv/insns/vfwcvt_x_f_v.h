// vfwcvt.x.f.v vd, vs2, vm
VI_CHECK_DSS(false);
if (P_.VU.vsew == e32)
  require(p->supports_extension('D'));

VI_VFP_LOOP_BASE
  auto vs2 = P_.VU.elt<float32_t>(rs2_num, i, VREAD);
  P_.VU.elt<int64_t>(rd_num, i, VWRITE) = f32_to_i64(vs2, STATE.frm, true);
  set_fp_exceptions;
VI_VFP_LOOP_WIDE_END
