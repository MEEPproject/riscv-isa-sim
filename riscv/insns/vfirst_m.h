// vmfirst rd, vs2
require(P_.VU.vsew >= e8 && P_.VU.vsew <= e64);
require_vector;
reg_t vl = P_.VU.vl;
reg_t sew = P_.VU.vsew;
reg_t rd_num = insn.rd();
reg_t rs2_num = insn.rs2();
require(P_.VU.vstart == 0);
reg_t pos = -1;
for (reg_t i=P_.VU.vstart; i < vl; ++i) {
  VI_LOOP_ELEMENT_SKIP()

  bool vs2_lsb = ((P_.VU.elt<uint64_t>(rs2_num, midx, VREAD ) >> mpos) & 0x1) == 1;
  if (vs2_lsb) {
    pos = i;
    break;
  }
}
P_.VU.vstart = 0;
WRITE_RD(pos);
