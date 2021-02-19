// vmpopc rd, vs2, vm
require(P_.VU.vsew >= e8 && P_.VU.vsew <= e64);
require_vector;
reg_t vl = P_.VU.vl;
reg_t sew = P_.VU.vsew;
reg_t rd_num = insn.rd();
reg_t rs1_num = insn.rs1();
reg_t rs2_num = insn.rs2();
require(P_.VU.vstart == 0);
require(!is_overlapped(rd_num, P_.VU.vlmul, rs2_num, 1));
if (insn.v_vm() == 0)
  require(!is_overlapped(rd_num, P_.VU.vlmul, 0, 1));
require((rd_num & (P_.VU.vlmul - 1)) == 0);

int cnt = 0;
for (reg_t i = 0; i < vl; ++i) {
  const int mlen = P_.VU.vmlen;
  const int midx = (mlen * i) / 64;
  const int mpos = (mlen * i) % 64;

  bool vs2_lsb = ((P_.VU.elt<uint64_t>(rs2_num, midx) >> mpos) & 0x1) == 1;
  bool do_mask = (P_.VU.elt<uint64_t>(0, midx) >> mpos) & 0x1;

  bool has_one = false;
  if (insn.v_vm() == 1 || (insn.v_vm() == 0 && do_mask)) {
    if (vs2_lsb) {
      has_one = true;
    }
  }

  bool use_ori = (insn.v_vm() == 0) && !do_mask;
  switch (sew) {
  case e8:
    P_.VU.elt<uint8_t>(rd_num, i) = use_ori ?
                                   P_.VU.elt<uint8_t>(rd_num, i) : cnt;
    break;
  case e16:
    P_.VU.elt<uint16_t>(rd_num, i) = use_ori ?
                                    P_.VU.elt<uint16_t>(rd_num, i) : cnt;
    break;
  case e32:
    P_.VU.elt<uint32_t>(rd_num, i) = use_ori ?
                                    P_.VU.elt<uint32_t>(rd_num, i) : cnt;
    break;
  default:
    P_.VU.elt<uint64_t>(rd_num, i) = use_ori ?
                                    P_.VU.elt<uint64_t>(rd_num, i) : cnt;
    break;
  }

  if (has_one) {
    cnt++;
  }
}

