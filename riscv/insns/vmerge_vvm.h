// vmerge.vvm vd, vs2, vs1
require(insn.rd() != 0);
VI_CHECK_SSS(true);
VI_VVXI_MERGE_LOOP
({
  int midx = (P_.VU.vmlen * i) / 64;
  int mpos = (P_.VU.vmlen * i) % 64;
  bool use_first = (P_.VU.elt<uint64_t>(0, midx, VREAD) >> mpos) & 0x1;

  vd = use_first ? vs1 : vs2;
})
