// vmpopc rd, vs2, vm
require(P_.VU.vsew >= e8 && P_.VU.vsew <= e64);
require_vector;
reg_t vl = P_.VU.vl;
reg_t sew = P_.VU.vsew;
reg_t rd_num = insn.rd();
reg_t rs1_num = insn.rs1();
reg_t rs2_num = insn.rs2();

bool has_one = false;
for (reg_t i = P_.VU.vstart ; i < vl; ++i) {
  const int mlen = P_.VU.vmlen;
  const int midx = (mlen * i) / 64;
  const int mpos = (mlen * i) % 64;
  const uint64_t mmask = (UINT64_MAX << (64 - mlen)) >> (64 - mlen - mpos);

  bool vs2_lsb = ((P_.VU.elt<uint64_t>(rs2_num, midx, VREAD ) >> mpos) & 0x1) == 1;
  bool do_mask = (P_.VU.elt<uint64_t>(0, midx, VREAD) >> mpos) & 0x1;
  auto &vd = P_.VU.elt<uint64_t>(rd_num, midx, VREADWRITE);

  if (insn.v_vm() == 1 || (insn.v_vm() == 0 && do_mask)) {
    uint64_t res = 0;
    if (!has_one && !vs2_lsb) {
      res = 1;
    } else if(!has_one && vs2_lsb) {
      has_one = true;
      res = 1;
    }
    vd = (vd & ~mmask) | ((res << mpos) & mmask);
  }
}

P_.VU.vstart = 0;
