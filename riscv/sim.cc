// See LICENSE for license details.

#include "sim.h"
#include "mmu.h"
#include "dts.h"
#include "remote_bitbang.h"
#include "byteorder.h"
#include <map>
#include <iostream>
#include <sstream>
#include <climits>
#include <cstdlib>
#include <cassert>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <thread>

volatile bool ctrlc_pressed = false;
static void handle_signal(int sig)
{
  if (ctrlc_pressed)
    exit(-1);
  ctrlc_pressed = true;
  signal(sig, &handle_signal);
}

sim_t::sim_t(const char* isa, const char* priv, const char* varch,
             size_t nprocs, bool halted,
             reg_t start_pc, std::vector<std::pair<reg_t, mem_t*>> mems,
             std::vector<std::pair<reg_t, abstract_device_t*>> plugin_devices,
             const std::vector<std::string>& args,
             std::vector<int> const hartids,
             const debug_module_config_t &dm_config,
             bool enable_smart_mcpu
             )
  : htif_t(args), mems(mems), plugin_devices(plugin_devices),
    procs(std::max(nprocs, size_t(1))), start_pc(start_pc), current_step(0),
    current_proc(0), debug(false), histogram_enabled(false),
    log_commits_enabled(false), dtb_enabled(true),
    remote_bitbang(NULL), debug_module(this, dm_config), enable_smart_mcpu(enable_smart_mcpu)
{
  signal(SIGINT, &handle_signal);

  for (auto& x : mems)
    bus.add_device(x.first, x.second);

  for (auto& x : plugin_devices)
    bus.add_device(x.first, x.second);

  debug_module.add_device(&bus);

  debug_mmu = new mmu_t(this, NULL);


  if (hartids.size() == 0) {
    for (size_t i = 0; i < procs.size(); i++) {
      procs[i] = new processor_t(isa, priv, varch, this, i, halted, enable_smart_mcpu);
    }
  }
  else {
    if (hartids.size() != procs.size()) {
      std::cerr << "Number of specified hartids doesn't match number of processors" << strerror(errno) << std::endl;
      exit(1);
    }
    for (size_t i = 0; i < procs.size(); i++) {
      procs[i] = new processor_t(isa, priv, varch, this, hartids[i], halted);
    }
  }

  clint.reset(new clint_t(procs));
  bus.add_device(CLINT_BASE, clint.get());
  printf("Created\n");
}

sim_t::~sim_t()
{
  for (size_t i = 0; i < procs.size(); i++)
    delete procs[i];
  printf("Spent %lu nanos in the region of interest\n", timer);
  delete debug_mmu;
}

void sim_thread_main(void* arg)
{
  ((sim_t*)arg)->main();
}

void sim_t::main()
{
  if (!debug && log)
    set_procs_debug(true);

  while (!done())
  {
    if (debug || ctrlc_pressed)
      interactive();
    else
    {
      step(INTERLEAVE);
    }
    if (remote_bitbang) {
      remote_bitbang->tick();
    }
  }


}

bool sim_t::simulate_one(uint32_t core, uint64_t current_cycle, std::list<std::shared_ptr<spike_model::Event>>& events)
{
    //struct timeval st, et;
    //gettimeofday(&st,NULL);
    /*std::cout << std::endl << "Before simulate at " << current_cycle << " INT:" << std::endl;
    for(auto itr = procs[core]->get_state()->pending_int_regs->begin();
             itr!= procs[core]->get_state()->pending_int_regs->end(); itr++)
    {
        std::cout << *itr << std::endl;
    }*/

    bool res=false;
    if(!done())
    {
        procs[core]->set_current_cycle(current_cycle);

        res=my_step_one(core);
        std::list<std::shared_ptr<spike_model::CacheRequest>> new_misses=procs[core]->get_mmu()->get_misses();

        //This is clearly inefficient, but we cannot directly return a Event list
        //The destination register has to be set from decode, which runs after misses
        //have been stored in the MMU. We need requests to remain requests until this moment.
        //TODO: Verify that does not incurr in significant performance loss. Other, more performant
        //      implementation might be challenging without significant changes to Spike
        //TODO: Considering we are adding new kinds of events, we might change events into an aggregate
        //      type that already holds classified events (e.g. a list of misses and a list of
        //      the rest of events)

        for(std::shared_ptr<spike_model::CacheRequest> miss: new_misses)
        {
            events.push_back(miss);
        }

        //printf("Got %lu misses here\n", l1Misses.size());

        if(procs[core]->is_in_fence())
        {
            events.push_back(std::make_shared<spike_model::Fence>(0, current_cycle, core));
        }
        else if(procs[core]->is_in_set_vl() && enable_smart_mcpu)
        {
            events.push_back(std::make_shared<spike_model::MCPUSetVVL>(procs[core]->VU.curr_AVL,
                             procs[core]->VU.curr_rd, procs[core]->get_state()->pc, 
                             current_cycle, core));
        }
        else if(procs[core]->is_mcpu_instruction() && enable_smart_mcpu)
        {
            events.push_back(procs[core]->get_mcpu_instruction());
        }

        if(!res)
        {
            /* There is a RAW dependency in the current instruction.
               A new event should be generated only when the depending instruction
               is compute. If the depending instruction is memory instruction,
               a CacheRequest event is generated.
            */
            std::list<std::shared_ptr<spike_model::InsnLatencyEvent>> latency_event_list =
                       procs[core]->get_insn_latency_event_list();
            for(std::shared_ptr<spike_model::InsnLatencyEvent> latency_event: latency_event_list)
            {
                events.push_back(latency_event);
            }
            procs[core]->clear_latency_event_list();
        }
    }
    else
    {
        res=true;
        events.push_back(std::make_shared<spike_model::Finish>(0, current_cycle, core));
        procs[core]->reset_mcpu_instruction();
    }
    //gettimeofday(&et,NULL);

    //uint64_t elapsed = ((et.tv_sec - st.tv_sec) * 1000000) + (et.tv_usec - st.tv_usec);
    //timer+=elapsed;

    procs[core]->VU.clear_bookkeeping_regs();
    procs[core]->clear_bookkeeping_regs();
    return res;
}

bool sim_t::ack_register(uint64_t coreId, spike_model::Request::RegType destRegType, size_t destRegId, uint64_t timestamp)
{
    bool ready;
    switch(destRegType)
    {
        case spike_model::Request::RegType::INTEGER:
            ready=procs[coreId]->get_state()->XPR.ack_for_reg(destRegId, timestamp);
            // If all the requests for the register (vector instructions might require many) have been serviced, it is no longer pending
            if(ready)
            {
                procs[coreId]->get_state()->pending_int_regs->erase(destRegId);
            }
            break;
        case spike_model::Request::RegType::FLOAT:
            ready=procs[coreId]->get_state()->FPR.ack_for_reg(destRegId, timestamp);
            // If all the requests for the register (vector instructions might require many) have been serviced, it is no longer pending
            if(ready)
            {
                procs[coreId]->get_state()->pending_float_regs->erase(destRegId);
            }
            break;
        case spike_model::Request::RegType::VECTOR:
            ready=procs[coreId]->VU.ack_for_reg(destRegId, timestamp);
            // If all the requests for the register (vector instructions might require many) have been serviced, it is no longer pending
            if(ready)
            {
                procs[coreId]->get_state()->pending_vector_regs->erase(destRegId);
            }
            break;
        default:
            std::cout << "Unknown register kind!\n";
            break;
    }

    //Simulation can resumen if there are no pending registers.
    return procs[coreId]->get_state()->pending_int_regs->size()==0 && 
           procs[coreId]->get_state()->pending_float_regs->size()==0 &&
           procs[coreId]->get_state()->pending_vector_regs->size()==0;
}

void sim_t::set_vvl(uint64_t core, uint64_t vvl)
{
    procs[core]->VU.set_vvl(vvl);
}

bool sim_t::can_resume(uint64_t coreId, size_t srcRegId,
                       spike_model::Request::RegType srcRegType,
                       size_t destRegId, spike_model::Request::RegType destRegType,
                       uint64_t latency, uint64_t timestamp)
{
    switch(srcRegType)
    {
        case spike_model::Request::RegType::INTEGER:
             procs[coreId]->get_state()->pending_int_regs->erase(srcRegId);
             break;
        case spike_model::Request::RegType::FLOAT:
             procs[coreId]->get_state()->pending_float_regs->erase(srcRegId);
             break;
        case spike_model::Request::RegType::VECTOR:
             procs[coreId]->get_state()->pending_vector_regs->erase(srcRegId);
             break;
        default:
            std::cout << "Unknown register kind!\n";
            break;
    }

    //Simulation can resume if there are no pending registers.
    return procs[coreId]->get_state()->pending_int_regs->size()==0 &&
           procs[coreId]->get_state()->pending_float_regs->size()==0 &&
           procs[coreId]->get_state()->pending_vector_regs->size()==0;
}

void htif_run_launcher(void* arg)
{
    ((sim_t*)arg)->htif_t::run();
}

/*
Prepares the wrapped spike to wait for simulate requests. This is
basically the opposite of the run function, in the sense that the 
work that is done by each of the two threads is reversed. When
executing in this mode, Spike debug mode is likely to not work.
*/
void sim_t::prepare()
{
  target = *context_t::current();

  host=new context_t();
  host->init(htif_run_launcher, this);

  for(unsigned int i=0; i<procs.size();i++)
  {
      procs[i]->enable_miss_log();
      procs[i]->get_mmu()->enable_miss_log();
  }
 
  host->switch_to();
}

int sim_t::run()
{
  host = context_t::current();
  target.init(sim_thread_main, this);

  int res=htif_t::run();

  return res;
}


/*
Simulates a single instruction in the specified core.
*/
bool sim_t::my_step_one(size_t core)
{
    bool res=procs[core]->step(1);

    current_step += 1;
    if (current_step == INTERLEAVE*procs.size())
    {
      current_step = 0;

      if(!done())
      {
        host->switch_to();
      }
    }
    procs[core]->get_mmu()->yield_load_reservation();
    return res;
}


void sim_t::step(size_t n)
{
  for (size_t i = 0, steps = 0; i < n; i += steps)
  {
    steps = std::min(n - i, INTERLEAVE - current_step);
    procs[current_proc]->step(steps);

    current_step += steps;

    if (current_step == INTERLEAVE)
    {
      current_step = 0;
      procs[current_proc]->get_mmu()->yield_load_reservation();
      if (++current_proc == procs.size()) {
        current_proc = 0;
        clint->increment(INTERLEAVE / INSNS_PER_RTC_TICK);
      }
        
      if(!done())
      {
        host->switch_to();
      }
    }
  }
}

void sim_t::set_debug(bool value)
{
  debug = value;
}

void sim_t::set_log(bool value)
{
  log = value;
}

void sim_t::set_histogram(bool value)
{
  histogram_enabled = value;
  for (size_t i = 0; i < procs.size(); i++) {
    procs[i]->set_histogram(histogram_enabled);
  }
}

void sim_t::set_log_commits(bool value)
{
  log_commits_enabled = value;
  for (size_t i = 0; i < procs.size(); i++) {
    procs[i]->set_log_commits(log_commits_enabled);
  }
}

void sim_t::set_procs_debug(bool value)
{
  for (size_t i=0; i< procs.size(); i++)
    procs[i]->set_debug(value);
}

static bool paddr_ok(reg_t addr)
{
  return (addr >> MAX_PADDR_BITS) == 0;
}

bool sim_t::mmio_load(reg_t addr, size_t len, uint8_t* bytes)
{
  if (addr + len < addr || !paddr_ok(addr + len - 1))
    return false;
  return bus.load(addr, len, bytes);
}

bool sim_t::mmio_store(reg_t addr, size_t len, const uint8_t* bytes)
{
  if (addr + len < addr || !paddr_ok(addr + len - 1))
    return false;
  return bus.store(addr, len, bytes);
}

void sim_t::make_dtb()
{
  const int reset_vec_size = 8;

  start_pc = start_pc == reg_t(-1) ? get_entry_point() : start_pc;

  uint32_t reset_vec[reset_vec_size] = {
    0x297,                                      // auipc  t0,0x0
    0x28593 + (reset_vec_size * 4 << 20),       // addi   a1, t0, &dtb
    0xf1402573,                                 // csrr   a0, mhartid
    get_core(0)->get_xlen() == 32 ?
      0x0182a283u :                             // lw     t0,24(t0)
      0x0182b283u,                              // ld     t0,24(t0)
    0x28067,                                    // jr     t0
    0,
    (uint32_t) (start_pc & 0xffffffff),
    (uint32_t) (start_pc >> 32)
  };
  for(int i = 0; i < reset_vec_size; i++)
    reset_vec[i] = to_le(reset_vec[i]);

  std::vector<char> rom((char*)reset_vec, (char*)reset_vec + sizeof(reset_vec));

  dts = make_dts(INSNS_PER_RTC_TICK, CPU_HZ, procs, mems);
  std::string dtb = dts_compile(dts);

  rom.insert(rom.end(), dtb.begin(), dtb.end());
  const int align = 0x1000;
  rom.resize((rom.size() + align - 1) / align * align);

  boot_rom.reset(new rom_device_t(rom));
  bus.add_device(DEFAULT_RSTVEC, boot_rom.get());
}

char* sim_t::addr_to_mem(reg_t addr) {
  if (!paddr_ok(addr))
    return NULL;
  auto desc = bus.find_device(addr);
  if (auto mem = dynamic_cast<mem_t*>(desc.second))
    if (addr - desc.first < mem->size())
      return mem->contents() + (addr - desc.first);
  return NULL;
}

// htif

void sim_t::reset()
{
  if (dtb_enabled)
    make_dtb();
}

void sim_t::idle()
{
  target.switch_to();
}

void sim_t::read_chunk(addr_t taddr, size_t len, void* dst)
{
  assert(len == 8);
  auto data = to_le(debug_mmu->load_uint64(taddr));
  memcpy(dst, &data, sizeof data);
}

void sim_t::write_chunk(addr_t taddr, size_t len, const void* src)
{
  assert(len == 8);
  uint64_t data;
  memcpy(&data, src, sizeof data);
  debug_mmu->store_uint64(taddr, from_le(data));
}

void sim_t::proc_reset(unsigned id)
{
  debug_module.proc_reset(id);
}
