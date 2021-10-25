// See LICENSE for license details.

#include "insn_template.h"

reg_t rv32_NAME(processor_t* p, insn_t insn, reg_t pc)
{
  int xlen = 32;
  reg_t npc = sext_xlen(pc + insn_length(OPCODE));
  #include "insns/NAME.h"
  trace_opcode(p, OPCODE, insn);
  return npc;
}

reg_t rv64_NAME(processor_t* p, insn_t insn, reg_t pc)
{
  int xlen = 64;
  reg_t npc = sext_xlen(pc + insn_length(OPCODE));
  p->curr_insn_latency = insn_to_latency["rv64_NAME"];
  p->curr_write_reg = std::numeric_limits<uint64_t>::max();
  std::shared_ptr<spike_model::InsnLatencyEvent> insn_latency_ptr;
  #include "insns/NAME.h"
  if(p->curr_write_reg != std::numeric_limits<uint64_t>::max())
  {
    if(p->curr_write_reg_type == spike_model::Request::RegType::VECTOR)
    {
      //If smart mcpu is enabled, set the event dependent to 1
      if(p->enable_smart_mcpu)
      {
        if(p->is_load)
        {
          p->is_load = false;
          p->VU.set_event_dependent(p->curr_write_reg,
                                    1,
                                    std::numeric_limits<uint64_t>::max());
          p->get_mcpu_instruction()->setDestinationReg(p->curr_write_reg, spike_model::CacheRequest::RegType::VECTOR);
        }
        else if(p->is_store)
        {
          p->is_store = false;
          p->VU.set_event_dependent(p->curr_write_reg,
                                    0,
                                    p->get_current_cycle());
          p->get_mcpu_instruction()->setDestinationReg(p->curr_write_reg, spike_model::CacheRequest::RegType::VECTOR);
        }
        else
        {
          p->VU.set_event_dependent(p->curr_write_reg, 0,
                            p->get_current_cycle() + p->get_curr_insn_latency() * p->VU.get_vl()/(p->VU.VLEN/p->VU.ELEN));
          p->VU.set_busy_until(p->get_current_cycle()+p->get_curr_insn_latency() * (p->VU.get_vl()/(p->VU.VLEN/p->VU.ELEN)-1) 
                            + ceil((float)p->VU.VLEN/p->VU.ELEN/p->lanes_per_vpu));
        }
      }
      else
      {
        if(p->get_mmu()->num_pending_data_misses()>0)
        {
          p->VU.set_event_dependent(p->curr_write_reg,
                                    p->get_mmu()->num_pending_data_misses(),
                                    std::numeric_limits<uint64_t>::max());
          p->get_mmu()->set_misses_dest_reg(p->curr_write_reg,
                                          spike_model::CacheRequest::RegType::VECTOR);
        }
        else
        {
          p->VU.set_event_dependent(p->curr_write_reg, 0,
                                    p->get_current_cycle() + p->get_curr_insn_latency());
          p->VU.set_busy_until(p->get_current_cycle()+ceil((float)p->VU.VLEN/p->VU.ELEN/p->lanes_per_vpu));
        }
      }
    }
    else if(p->curr_write_reg_type == spike_model::Request::RegType::INTEGER)
    {
      if(p->get_mmu()->num_pending_data_misses()>0)
      {
        p->get_state()->XPR.set_event_dependent(p->curr_write_reg, 
                            p->get_mmu()->num_pending_data_misses(),
                            std::numeric_limits<uint64_t>::max());
        p->get_mmu()->set_misses_dest_reg(p->curr_write_reg,
            spike_model::CacheRequest::RegType::INTEGER);
      }
      else
        p->get_state()->XPR.set_event_dependent(p->curr_write_reg, 0,
                            p->get_current_cycle() + p->get_curr_insn_latency());
    }
    else
    {
      if(p->get_mmu()->num_pending_data_misses()>0)
      {
        p->get_state()->FPR.set_event_dependent(p->curr_write_reg,
              p->get_mmu()->num_pending_data_misses(), std::numeric_limits<uint64_t>::max());
        p->get_mmu()->set_misses_dest_reg(p->curr_write_reg,
              spike_model::CacheRequest::RegType::FLOAT);
      }
      else 
        p->get_state()->FPR.set_event_dependent(p->curr_write_reg, 0,
                        p->get_current_cycle() + p->get_curr_insn_latency());
    }
  }
  trace_opcode(p, OPCODE, insn);
  return npc;
}

bool is_raw_NAME(processor_t* p, insn_t insn, reg_t pc)
{
  int xlen = 64;
  #include "insns/raw/NAME.h"
}
