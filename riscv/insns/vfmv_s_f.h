// vfmv_s_f: vd[0] = rs1 (vs2=0)
require_vector;
require_fp;
require_extension('F');
require(P_.VU.vsew >= e32 && P_.VU.vsew <= 64);
reg_t vl = P_.VU.vl;

if (vl > 0) {
  reg_t rd_num = insn.rd();

  switch(P_.VU.vsew) {
    case 32:
      if (FLEN == 64)
        P_.VU.elt<uint32_t>(rd_num, 0) = f64(FRS1).v;
      else
        P_.VU.elt<uint32_t>(rd_num, 0) = f32(FRS1).v;
      break;
    case 64:
      if (FLEN == 64)
        P_.VU.elt<uint64_t>(rd_num, 0) = f64(FRS1).v;
      else
        P_.VU.elt<uint64_t>(rd_num, 0) = f32(FRS1).v;
      break;
  }
}
P_.VU.vstart = 0;
