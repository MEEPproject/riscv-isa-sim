// vfcvt.f.x.v vd, vd2, vm
VI_VFP_VF_LOOP
({
  auto vs2_i = P_.VU.elt<int32_t>(rs2_num, i, VREAD);
  vd = i32_to_f32(vs2_i);
},
{
  auto vs2_i = P_.VU.elt<int64_t>(rs2_num, i, VREAD);
  vd = i64_to_f64(vs2_i);
})
