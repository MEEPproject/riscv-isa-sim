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
  std::shared_ptr<coyote::InsnLatencyEvent> insn_latency_ptr;
  uint64_t time_ready=0;
  #include "insns/NAME.h"
  if(p->curr_write_reg != std::numeric_limits<uint64_t>::max())
  {
    P_.is_vector_memory = false;
    if(p->curr_write_reg_type == coyote::Request::RegType::VECTOR)
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
          p->get_mcpu_instruction()->setDestinationReg(p->curr_write_reg, coyote::CacheRequest::RegType::VECTOR);
        }
        else if(p->is_store)
        {
          p->is_store = false;
          p->VU.set_event_dependent(p->curr_write_reg,
                                    0,
                                    p->get_current_cycle());
          p->get_mcpu_instruction()->setDestinationReg(p->curr_write_reg, coyote::CacheRequest::RegType::VECTOR);
        }
        else
        {
          time_ready=p->set_vpu_latency_considering_lanes();
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
                                          coyote::CacheRequest::RegType::VECTOR);
        }
        else
        {
          time_ready=p->set_vpu_latency_considering_lanes();

        }
      }
    }
    else if(p->curr_write_reg_type == coyote::Request::RegType::INTEGER)
    {
      if(p->get_mmu()->num_pending_data_misses()>0)
      {
        p->get_state()->XPR.set_event_dependent(p->curr_write_reg, 
                            p->get_mmu()->num_pending_data_misses(),
                            std::numeric_limits<uint64_t>::max());
        p->get_mmu()->set_misses_dest_reg(p->curr_write_reg,
            coyote::CacheRequest::RegType::INTEGER);
      }
      else
      {
        p->get_state()->XPR.set_event_dependent(p->curr_write_reg, 0, p->get_current_cycle() + p->get_curr_insn_latency());
        time_ready=p->get_current_cycle() + p->get_curr_insn_latency();
      }
    }
    else
    {
      if(p->get_mmu()->num_pending_data_misses()>0)
      {
        p->get_state()->FPR.set_event_dependent(p->curr_write_reg,
              p->get_mmu()->num_pending_data_misses(), std::numeric_limits<uint64_t>::max());
        p->get_mmu()->set_misses_dest_reg(p->curr_write_reg,
              coyote::CacheRequest::RegType::FLOAT);
      }
      else 
      {
        p->get_state()->FPR.set_event_dependent(p->curr_write_reg, 0, p->get_current_cycle() + p->get_curr_insn_latency());
        time_ready=p->get_current_cycle() + p->get_curr_insn_latency();
      }
    }
  }
  trace_opcode(p, OPCODE, insn);
  p->trace_instruction(insn, pc);
  if(p->get_mmu()->num_pending_misses()==0)
  {
    p->trace_instruction_graduate(insn, pc, time_ready);
  }
  else
  {
    p->pending_instructions[pc]=std::make_pair(insn, p->get_mmu()->num_pending_misses());
  }

  return npc;
}

bool is_raw_NAME(processor_t* p, insn_t insn, reg_t pc)
{
  int xlen = 64;
  #include "insns/raw/NAME.h"
}
