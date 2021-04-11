// See LICENSE for license details.

#include "insn_template.h"

reg_t rv32_NAME(processor_t* p, insn_t insn, reg_t pc)
{
  int xlen = 32;
  reg_t npc = sext_xlen(pc + insn_length(OPCODE));
  p->curr_insn_latency = insn_to_latency["rv32_NAME"];
  #include "insns/NAME.h"
  if(p->vl_dependent)
  {
    p->vl_dependent = false;
    return pc;
  }
  trace_opcode(p, OPCODE, insn);
  return npc;
}

reg_t rv64_NAME(processor_t* p, insn_t insn, reg_t pc)
{
  int xlen = 64;
  reg_t npc = sext_xlen(pc + insn_length(OPCODE));
  p->curr_insn_latency = insn_to_latency["rv64_NAME"];
  #include "insns/NAME.h"
  if(p->vl_dependent)
  {
    p->vl_dependent = false;
    return pc;
  }
  trace_opcode(p, OPCODE, insn);
  return npc;
}
