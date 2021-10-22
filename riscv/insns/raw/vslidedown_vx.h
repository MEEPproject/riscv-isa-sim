CHECK_MEMTILE_ENABLE();

bool b1 = C_RS1;

bool b2 = P_.VU.check_raw<uint64_t>(0, 0);
bool b3 = false;
bool global = false;
const reg_t sh = RS1;
VI_LOOP_BASE

reg_t offset = 0;
bool is_valid = (i + sh) < P_.VU.vlmax;

if (is_valid) {
  offset = sh;
}

switch (sew) {
case e8: {
  b3 = P_.VU.check_raw<type_sew_t<e8>::type>(rs2_num, i + offset);
  if(b3)
    global = b3; 
}
break;
case e16: {
  b3 = P_.VU.check_raw<type_sew_t<e16>::type>(rs2_num, i + offset);
  if(b3)
    global = b3; 
}
break;
case e32: {
  b3 = P_.VU.check_raw<type_sew_t<e32>::type>(rs2_num, i + offset);
  if(b3)
    global = b3; 
}
break;
default: {
  b3 = P_.VU.check_raw<type_sew_t<e64>::type>(rs2_num, i + offset);
  if(b3)
    global = b3; 
}
break;
}
VI_LOOP_END

return (b1 || b2 || global);
