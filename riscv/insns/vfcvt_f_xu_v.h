// vfcvt.f.xu.v vd, vd2, vm
VI_VFP_VF_LOOP
({
  auto vs2_u = P_.VU.elt<uint32_t>(rs2_num, i, VREAD);
  vd = ui32_to_f32(vs2_u);
},
{
  auto vs2_u = P_.VU.elt<uint64_t>(rs2_num, i, VREAD);
  vd = ui64_to_f64(vs2_u);
})
