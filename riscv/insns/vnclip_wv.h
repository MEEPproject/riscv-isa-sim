// vnclip: vd[i] = clip(round(vs2[i] + rnd) >> vs1[i])
VRM xrm = P_.VU.get_vround_mode();
int64_t int_max = (1 << (P_.VU.vsew - 1)) - 1;
int64_t int_min = -(1 << (P_.VU.vsew - 1));
VI_VVXI_LOOP_NARROW
({
  int128_t result = vs2;
  unsigned shift = vs1 & ((sew * 2) - 1);

  // rounding
  INT_ROUNDING(result, xrm, shift);

  result = result >> shift;

  // saturation
  if (result < int_min) {
    result = int_min;
    P_.VU.vxsat = 1;
  } else if (result > int_max) {
    result = int_max;
    P_.VU.vxsat = 1;
  }

  vd = result;
}, true)
