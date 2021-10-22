CHECK_MEMTILE_ENABLE();

bool b1 = false;
bool b2 = P_.VU.check_raw<uint64_t>(0, 0);
bool global = false;
VI_LOOP_BASE
if (i != vl - 1) {
  switch (sew) {
  case e8: {
    b1 = P_.VU.check_raw<type_sew_t<e8>::type>(rs2_num, i + 1);
    if(b1)
      global = b1;
  }
  break;
  case e16: {
    b1 = P_.VU.check_raw<type_sew_t<e16>::type>(rs2_num, i + 1);
    if(b1)
      global = b1;
  }
  break;
  case e32: {
    b1 = P_.VU.check_raw<type_sew_t<e32>::type>(rs2_num, i + 1);
    if(b1)
      global = b1;
  }
  break;
  default: {
    b1 = P_.VU.check_raw<type_sew_t<e64>::type>(rs2_num, i + 1);
    if(b1)
      global = b1;
  }
  break;
  }
} else {
  switch (sew) {
  case e8:
    b1 = C_RS1;
    if(b1)
      global = b1;
    break;
  case e16:
    b1 = C_RS1;
    if(b1)
      global = b1;
    break;
  case e32:
    b1 = C_RS1;
    if(b1)
      global = b1;
    break;
  default:
    b1 = C_RS1;
    if(b1)
      global = b1;
    break;
  }
}
VI_LOOP_END

return (b2 || global);
