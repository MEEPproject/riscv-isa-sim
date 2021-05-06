// See LICENSE for license details.

#include "insn_template.h"

reg_t rv32_NAME(processor_t* p, insn_t insn, reg_t pc)
{
  int xlen = 32;
  reg_t npc = sext_xlen(pc + insn_length(OPCODE));
  p->curr_insn_latency = insn_to_latency["rv32_NAME"];
  p->curr_write_reg = std::numeric_limits<uint64_t>::max();
  #include "insns/NAME.h"
  if(p->curr_write_reg != std::numeric_limits<uint64_t>::max())
  {
    p->set_dest_reg_in_event_list_raw(p->curr_write_reg, p->curr_write_reg_type);
    p->set_src_load_reg_raw(p->curr_write_reg, p->curr_write_reg_type);
  }
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
  p->curr_write_reg = std::numeric_limits<uint64_t>::max();
  #include "insns/NAME.h"
  if(p->curr_write_reg != std::numeric_limits<uint64_t>::max())
  {
       if(p->get_state()->raw)
       {
         p->set_dest_reg_in_event_list_raw(p->curr_write_reg, p->curr_write_reg_type);
         p->set_src_load_reg_raw(p->curr_write_reg, p->curr_write_reg_type);
       }
       else
       {
         if(p->curr_write_reg_type == spike_model::Request::RegType::VECTOR)
           p->VU.set_event_dependent(p->curr_write_reg, 0, p->get_current_cycle() + p->get_curr_insn_latency());
         else if(p->curr_write_reg_type == spike_model::Request::RegType::INTEGER)
           p->get_state()->XPR.set_event_dependent(p->curr_write_reg, 0, p->get_current_cycle() + p->get_curr_insn_latency());
         else
           p->get_state()->FPR.set_event_dependent(p->curr_write_reg, 0, p->get_current_cycle() + p->get_curr_insn_latency());
       }
       if(p->get_mmu()->num_pending_data_misses()>0)
       {
         if(p->curr_write_reg_type == spike_model::Request::RegType::INTEGER){
           p->get_state()->XPR.set_event_dependent(p->curr_write_reg, p->get_mmu()->num_pending_data_misses(), std::numeric_limits<uint64_t>::max());
           p->get_mmu()->set_misses_dest_reg(p->curr_write_reg, spike_model::CacheRequest::RegType::INTEGER);
         }
         else if(p->curr_write_reg_type == spike_model::Request::RegType::FLOAT){
           p->get_state()->FPR.set_event_dependent(p->curr_write_reg, p->get_mmu()->num_pending_data_misses(), std::numeric_limits<uint64_t>::max());
           p->get_mmu()->set_misses_dest_reg(p->curr_write_reg, spike_model::CacheRequest::RegType::FLOAT);
         }
         else if(p->curr_write_reg_type == spike_model::Request::RegType::VECTOR){
           p->VU.set_event_dependent(p->curr_write_reg, p->get_mmu()->num_pending_data_misses(), std::numeric_limits<uint64_t>::max());
           p->get_mmu()->set_misses_dest_reg(p->curr_write_reg, spike_model::CacheRequest::RegType::VECTOR);
         }
       }
  }
  if(p->vl_dependent)
  {
    p->vl_dependent = false;
    return pc;
  }
  trace_opcode(p, OPCODE, insn);
  return npc;
}
