// vsxw.v and vsxseg[2-8]w.v
require(P_.VU.vsew >= e32);
VI_CHECK_SXX;
require((insn.rs2() & (P_.VU.vlmul - 1)) == 0); \
reg_t vl = P_.VU.vl;
reg_t baseAddr = RS1;
reg_t stride = insn.rs2();
reg_t vs3 = insn.rd();
reg_t vlmax = P_.VU.vlmax;
VI_DUPLICATE_VREG(stride, vlmax);
for (reg_t i = 0; i < vlmax && vl != 0; ++i) {
  VI_ELEMENT_SKIP(i);
  VI_STRIP(i)

  switch (P_.VU.vsew) {
  case e32:
    MMU.store_uint32(baseAddr + index[i],
                     P_.VU.elt<uint32_t>(vs3, vreg_inx, VREAD));
    break;
  case e64:
    MMU.store_uint32(baseAddr + index[i],
                     P_.VU.elt<uint64_t>(vs3, vreg_inx, VREAD));
    break;
  }
}
P_.VU.vstart = 0;
