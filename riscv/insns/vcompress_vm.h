// vcompress vd, vs2, vs1
require(P_.VU.vstart == 0);
require((insn.rd() & (P_.VU.vlmul - 1)) == 0);
require((insn.rs2() & (P_.VU.vlmul - 1)) == 0);
require(insn.rd() != insn.rs2());
require(!is_overlapped(insn.rd(), P_.VU.vlmul, insn.rs1(), 1));

reg_t pos = 0;

VI_GENERAL_LOOP_BASE
  const int mlen = P_.VU.vmlen;
  const int midx = (mlen * i) / 64;
  const int mpos = (mlen * i) % 64;

  bool do_mask = (P_.VU.elt<uint64_t>(rs1_num, midx) >> mpos) & 0x1;
  if (do_mask) {
    switch (sew) {
    case e8:
      P_.VU.elt<uint8_t>(rd_num, pos) = P_.VU.elt<uint8_t>(rs2_num, i);
      break;
    case e16:
      P_.VU.elt<uint16_t>(rd_num, pos) = P_.VU.elt<uint16_t>(rs2_num, i);
      break;
    case e32:
      P_.VU.elt<uint32_t>(rd_num, pos) = P_.VU.elt<uint32_t>(rs2_num, i);
      break;
    default:
      P_.VU.elt<uint64_t>(rd_num, pos) = P_.VU.elt<uint64_t>(rs2_num, i);
      break;
    }

    ++pos;
  }
VI_LOOP_END;
