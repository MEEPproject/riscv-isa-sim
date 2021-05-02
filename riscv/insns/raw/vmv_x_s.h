bool b1 = C_RS1;
bool b2 = false;
reg_t rs1 = RS1;
reg_t sew = P_.VU.vsew;
reg_t rs2_num = insn.rs2();

if (!(rs1 >= 0 && rs1 < (P_.VU.get_vlen() / sew))) {
} else {
  switch(sew) {
  case e8:
    b2 = P_.VU.check_raw<int8_t>(rs2_num, rs1);
    break;
  case e16:
    b2 = P_.VU.check_raw<int16_t>(rs2_num, rs1);
    break;
  case e32:
    b2 = P_.VU.check_raw<int32_t>(rs2_num, rs1);
    break;
  case e64:
    if (P_.get_max_xlen() <= sew)
      b2 = P_.VU.check_raw<uint64_t>(rs2_num, rs1);
    else
      b2 = P_.VU.check_raw<uint64_t>(rs2_num, rs1);
    break;
  }
}
return (b1 || b2);
