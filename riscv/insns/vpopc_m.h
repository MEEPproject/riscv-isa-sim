// vmpopc rd, vs2, vm
require(P_.VU.vsew >= e8 && P_.VU.vsew <= e64);
require_vector;
reg_t vl = P_.VU.vl;
reg_t sew = P_.VU.vsew;
reg_t rd_num = insn.rd();
reg_t rs2_num = insn.rs2();
require(P_.VU.vstart == 0);
reg_t popcount = 0;
for (reg_t i=P_.VU.vstart; i<vl; ++i) {
  const int mlen = P_.VU.vmlen;
  const int midx = (mlen * i) / 32;
  const int mpos = (mlen * i) % 32;

  bool vs2_lsb = ((P_.VU.elt<uint32_t>(rs2_num, midx, VREAD ) >> mpos) & 0x1) == 1;
  if (insn.v_vm() == 1) {
    popcount += vs2_lsb;
  } else {
    bool do_mask = (P_.VU.elt<uint32_t>(0, midx, VREAD) >> mpos) & 0x1;
    popcount += (vs2_lsb && do_mask);
  }
}
P_.VU.vstart = 0;
WRITE_RD(popcount);
