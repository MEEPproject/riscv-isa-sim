// vs1r.v vs3, (rs1)
require_vector;
const reg_t baseAddr = RS1;
const reg_t vs3 = insn.rd();
for (reg_t i = 0; i < P_.VU.vlenb; ++i) {
  auto val = P_.VU.elt<uint8_t>(vs3, i, VREAD);
  MMU.store_uint8(baseAddr + i, val);
}
P_.VU.vstart = 0; 
