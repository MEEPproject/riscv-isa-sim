// vl1r.v vd, (rs1)
require_vector;
const reg_t baseAddr = RS1;
const reg_t vd = insn.rd();
for (reg_t i = 0; i < P_.VU.vlenb; ++i) {
  auto val = MMU.load_uint8(baseAddr + i);
  P_.VU.elt<uint8_t>(vd, i, VWRITE) = val;
}
P_.VU.vstart = 0;
