require_vector_for_vsetvl;
extern bool enable_smart_mcpu;
if(enable_smart_mcpu)
  P_.VU.get_vl_from_mcpu(insn.rd(), insn.rs1(), RS1, RS2);
else
  WRITE_RD(P_.VU.set_vl(insn.rd(), insn.rs1(), RS1, RS2));
