// vnclipu: vd[i] = clip(round(vs2[i] + rnd) >> vs1[i])
VRM xrm = P_.VU.get_vround_mode();
uint64_t int_max = ~(-1ll << P_.VU.vsew);
VI_VVXI_LOOP_NARROW
({
  uint128_t result = vs2_u;
  unsigned shift = vs1 & ((sew * 2) - 1);

  // rounding
  INT_ROUNDING(result, xrm, shift);

  result = result >> shift;

  // saturation
  if (result & (uint64_t)(-1ll << sew)) {
    result = int_max;
    P_.VU.vxsat = 1;
  }

  vd = result;
}, true)
