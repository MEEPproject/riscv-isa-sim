if(p->enable_smart_mcpu && !p->is_vl_available)
{
  p->get_state()->raw = true;
  return true;
}
bool b1 = C_RS1;

bool b2 = P_.VU.check_raw<uint64_t>(0, 0);
const reg_t offset = RS1;

bool b3 = false; 
bool global = false; 

VI_LOOP_BASE
if (P_.VU.vstart < offset && i < offset)
  continue;

switch (sew) {
case e8: {
  b3 = P_.VU.check_raw<type_sew_t<e8>::type>(rs2_num, i - offset);
  if(b3)
    global = b3;
}
break;
case e16: {
  b3 = P_.VU.check_raw<type_sew_t<e16>::type>(rs2_num, i - offset);
  if(b3)
    global = b3;
}
break;
case e32: {
  b3 = P_.VU.check_raw<type_sew_t<e32>::type>(rs2_num, i - offset);
  if(b3)
    global = b3;
}
break;
default: {
  b3 = P_.VU.check_raw<type_sew_t<e64>::type>(rs2_num, i - offset);
  if(b3)
    global = b3;
}
break;
}
VI_LOOP_END
return (b1 || b2 || global);
