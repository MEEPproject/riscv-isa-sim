CHECK_MEMTILE_ENABLE();

bool b1 = P_.VU.check_raw<uint64_t>(0, 0);
reg_t sew = P_.VU.vsew;
reg_t zimm5 = insn.v_zimm5();
bool b2 = false;
switch (sew) {
  case e8:
    b2 = P_.VU.check_raw<uint8_t>(insn.rs2(), zimm5);
    break;
  case e16:
    b2 = P_.VU.check_raw<uint16_t>(insn.rs2(), zimm5);
    break;
  case e32:
    b2 = P_.VU.check_raw<uint32_t>(insn.rs2(), zimm5);
    break;
  default:
    b2 = P_.VU.check_raw<uint64_t>(insn.rs2(), zimm5);
    break;
}
return (b1 || b2);
