// vmulhsu.vx vd, vs2, rs1
VI_CHECK_SSS(false);
VI_LOOP_BASE
switch(sew) {
case e8: {
  auto &vd = P_.VU.elt<int8_t>(rd_num, i, VWRITE);
  auto vs2 = P_.VU.elt<int8_t>(rs2_num, i, VREAD);
  uint8_t rs1 = RS1;

  vd = ((int16_t)vs2 * (uint16_t)rs1) >> sew;
  break;
}
case e16: {
  auto &vd = P_.VU.elt<int16_t>(rd_num, i, VWRITE);
  auto vs2 = P_.VU.elt<int16_t>(rs2_num, i, VREAD);
  uint16_t rs1 = RS1;

  vd = ((int32_t)vs2 * (uint32_t)rs1) >> sew;
  break;
}
case e32: {
  auto &vd = P_.VU.elt<int32_t>(rd_num, i, VWRITE);
  auto vs2 = P_.VU.elt<int32_t>(rs2_num, i, VREAD);
  uint32_t rs1 = RS1;

  vd = ((int64_t)vs2 * (uint64_t)rs1) >> sew;
  break;
}
default: {
  auto &vd = P_.VU.elt<int64_t>(rd_num, i, VWRITE);
  auto vs2 = P_.VU.elt<int64_t>(rs2_num, i, VREAD);
  uint64_t rs1 = RS1;

  vd = ((int128_t)vs2 * (uint128_t)rs1) >> sew;
  break;
}
}
VI_LOOP_END
