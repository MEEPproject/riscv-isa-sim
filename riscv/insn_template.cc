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
          //PVL in bits
          //ELEN in bits
          //vl in elements
          //
          //Assumes full pipelining 

          uint16_t lane_chunks;
          uint16_t pvl_chunks;

          if(p->VU.get_vl()<=p->VU.PVL/p->VU.ELEN) //The end of the vector might be smaller
          {
            lane_chunks=ceil((float)p->VU.get_vl()/p->lanes_per_vpu);
            pvl_chunks=1;
          }
          else
          {
            lane_chunks=ceil((float)p->VU.PVL/p->VU.ELEN/p->lanes_per_vpu);
            pvl_chunks=ceil((float)p->VU.get_vl()/(p->VU.PVL/p->VU.ELEN));
          } 

          p->VU.set_event_dependent(p->curr_write_reg, 0,
                            p->get_current_cycle() + (p->get_curr_insn_latency()+lane_chunks-1)*pvl_chunks);
          p->VU.set_busy_until(p->get_current_cycle() + (p->get_curr_insn_latency()+lane_chunks-1)*(pvl_chunks-1)+lane_chunks);
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
          p->set_vpu_latency_considering_lanes();
          //PVL in bits
          //ELEN in bits
          //vl in elements
          //
          //Assumes full pipelining

          uint16_t chunks=ceil((float)p->VU.get_vl()/p->lanes_per_vpu);
          p->VU.set_event_dependent(p->curr_write_reg, 0,
                                    p->get_current_cycle() + p->get_curr_insn_latency()+chunks-1);
          p->VU.set_busy_until(p->get_current_cycle()+chunks);
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
