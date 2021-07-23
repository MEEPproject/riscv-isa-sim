if(p->enable_smart_mcpu && !p->is_vl_available)
{
  p->get_state()->raw = true;
  return true;
}
reg_t sew = P_.VU.vsew;
bool b1 = C_RS1;

bool b2 = P_.VU.check_raw<uint64_t>(0, 0);
bool b3 = false;
reg_t rs1 = RS1;
switch (sew) {
  case e8:
    b3 = P_.VU.check_raw<uint8_t>(insn.rs2(), rs1);
    break;
  case e16:
    b3 = P_.VU.check_raw<uint16_t>(insn.rs2(), rs1);
    break;
  case e32:
    b3 = P_.VU.check_raw<uint32_t>(insn.rs2(), rs1);
    break;
  default:
    b3 = P_.VU.check_raw<uint64_t>(insn.rs2(), rs1);
    break;
}
return (b1 || b2 || b3);
