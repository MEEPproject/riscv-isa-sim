// vfcvt.x.f.v vd, vd2, vm
VI_VFP_VF_LOOP
({
  P_.VU.elt<int32_t>(rd_num, i, VWRITE) = f32_to_i32(vs2, STATE.frm, true);
},
{
  P_.VU.elt<int64_t>(rd_num, i, VWRITE) = f64_to_i64(vs2, STATE.frm, true);
})
