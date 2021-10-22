CHECK_MEMTILE_ENABLE();

bool b1 = P_.VU.check_raw<uint64_t>(0, 0);
const reg_t offset = insn.v_zimm5();

bool b2 = false;
bool global = false;
VI_LOOP_BASE
if (P_.VU.vstart < offset && i < offset)
  continue;

switch (sew) {
case e8: {
  b2 = P_.VU.check_raw<type_sew_t<e8>::type>(rs2_num, i - offset);
  if(b2)
    global = b2;
}
break;
case e16: {
  b2 = P_.VU.check_raw<type_sew_t<e16>::type>(rs2_num, i - offset);
  if(b2)
    global = b2;
}
break;
case e32: {
  b2 = P_.VU.check_raw<type_sew_t<e32>::type>(rs2_num, i - offset);
  if(b2)
    global = b2;
}
break;
default: {
  b2 = P_.VU.check_raw<type_sew_t<e64>::type>(rs2_num, i - offset);
  if(b2)
    global = b2;
}
break;
}
VI_LOOP_END
return (b1 || global);
