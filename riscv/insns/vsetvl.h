require_vector_for_vsetvl;
if(P_.enable_smart_mcpu)
  P_.VU.get_vl_from_mcpu(insn.rd(), insn.rs1(), RS1, RS2);
else
  WRITE_RD(P_.VU.set_vl(insn.rd(), insn.rs1(), RS1, RS2));
