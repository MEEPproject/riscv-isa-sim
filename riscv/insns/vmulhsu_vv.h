// vmulhsu.vv vd, vs2, vs1
VI_CHECK_SSS(true);
VI_LOOP_BASE
switch(sew) {
case e8: {
  auto &vd = P_.VU.elt<int8_t>(rd_num, i, VWRITE);
  auto vs2 = P_.VU.elt<int8_t>(rs2_num, i, VREAD);
  auto vs1 = P_.VU.elt<uint8_t>(rs1_num, i, VREAD);

  vd = ((int16_t)vs2 * (uint16_t)vs1) >> sew;
  break;
}
case e16: {
  auto &vd = P_.VU.elt<int16_t>(rd_num, i, VWRITE);
  auto vs2 = P_.VU.elt<int16_t>(rs2_num, i, VREAD);
  auto vs1 = P_.VU.elt<uint16_t>(rs1_num, i, VREAD);

  vd = ((int32_t)vs2 * (uint32_t)vs1) >> sew;
  break;
}
case e32: {
  auto &vd = P_.VU.elt<int32_t>(rd_num, i, VWRITE);
  auto vs2 = P_.VU.elt<int32_t>(rs2_num, i, VREAD);
  auto vs1 = P_.VU.elt<uint32_t>(rs1_num, i, VREAD);

  vd = ((int64_t)vs2 * (uint64_t)vs1) >> sew;
  break;
}
default: {
  auto &vd = P_.VU.elt<int64_t>(rd_num, i, VWRITE);
  auto vs2 = P_.VU.elt<int64_t>(rs2_num, i, VREAD);
  auto vs1 = P_.VU.elt<uint64_t>(rs1_num, i, VREAD);

  vd = ((int128_t)vs2 * (uint128_t)vs1) >> sew;
  break;
}
}
VI_LOOP_END
