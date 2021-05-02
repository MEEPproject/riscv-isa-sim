bool global_bool = P_.VU.check_raw<uint64_t>(0, 0);

bool b1 = false;
VI_LOOP_BASE
switch (sew) {
  case e8: {
    b1 = P_.VU.check_raw<uint8_t>(rs1_num, i);

    if(b1 && !global_bool)
      global_bool = b1;
    auto vs1 = P_.VU.elt<uint8_t>(rs1_num, i, VREAD);

    b1 = P_.VU.check_raw<uint8_t>(rs2_num, vs1);
    if(b1 && !global_bool)
      global_bool = b1;
    break;
  }
  case e16: {
    b1 = P_.VU.check_raw<uint16_t>(rs1_num, i);

    if(b1 && !global_bool)
      global_bool = b1;
    auto vs1 = P_.VU.elt<uint16_t>(rs1_num, i, VREAD);

    b1 = P_.VU.check_raw<uint16_t>(rs2_num, vs1);
    if(b1 && !global_bool)
      global_bool = b1;
    break;
  }
  case e32: {
    b1 = P_.VU.check_raw<uint32_t>(rs1_num, i);

    if(b1 && !global_bool)
      global_bool = b1;
    auto vs1 = P_.VU.elt<uint32_t>(rs1_num, i, VREAD);

    b1 = P_.VU.check_raw<uint32_t>(rs2_num, vs1);
    if(b1 && !global_bool)
      global_bool = b1;
    break;
  }
  default: {
    b1 = P_.VU.check_raw<uint64_t>(rs1_num, i);

    if(b1 && !global_bool)
      global_bool = b1;
    auto vs1 = P_.VU.elt<uint64_t>(rs1_num, i, VREAD);

    b1 = P_.VU.check_raw<uint64_t>(rs2_num, vs1);
    if(b1 && !global_bool)
      global_bool = b1;
    break;
  }
}

VI_LOOP_END;
