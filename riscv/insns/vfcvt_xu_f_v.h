// vfcvt.xu.f.v vd, vd2, vm
VI_VFP_VV_LOOP
({
  P_.VU.elt<uint32_t>(rd_num, i, VWRITE) = f32_to_ui32(vs2, STATE.frm, true);
},
{
  P_.VU.elt<uint64_t>(rd_num, i, VWRITE) = f64_to_ui64(vs2, STATE.frm, true);
})
