reg_t sew = P_.VU.vsew;
bool b6 = P_.VU.check_raw<uint64_t>(0, 0);
bool b7 = false;
if (sew == e8){
    b7 = P_.VU.check_raw<type_sew_t<e8>::type>(insn.rs1(), 0);
  }else if(sew == e16){
    b7 = P_.VU.check_raw<type_sew_t<e16>::type>(insn.rs1(), 0);
  }else if(sew == e32){
    b7 = P_.VU.check_raw<type_sew_t<e32>::type>(insn.rs1(), 0);
  }else if(sew == e64){
    b7 = P_.VU.check_raw<type_sew_t<e64>::type>(insn.rs1(), 0);
  }

return (b6 || b7);
