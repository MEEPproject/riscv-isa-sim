// vssra.vi vd, vs2, simm5
VRM xrm = P_.VU.get_vround_mode();
VI_VI_LOOP
({
  int sh = simm5 & (sew - 1) & 0x1f;
  int64_t val = vs2;

  INT_ROUNDING(val, xrm, sh);
  vd = val >> sh;
})
