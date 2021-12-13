// See LICENSE for license details.
#ifndef _RISCV_PROCESSOR_H
#define _RISCV_PROCESSOR_H

#include <string>
#include <vector>
#include <map>
#include <cassert>
#include "debug_rom_defines.h"
#include <queue> //BORJA
#include <memory>
#include <unordered_map>

#include "decode.h"
#include "config.h"
#include "devices.h"
#include "trap.h"

#include "CacheRequest.hpp"
#include "MemoryTile/MCPUInstruction.hpp"
#include "MemoryTile/VectorElementType.hpp"
#include "InsnLatencyEvent.hpp"
#include <list>
#include <set>
#include <algorithm>
#include <vector>

class processor_t;
class mmu_t;
typedef reg_t (*insn_func_t)(processor_t*, insn_t, reg_t);
typedef bool (*insn_func_raw_t)(processor_t*, insn_t, reg_t);
class simif_t;
class trap_t;
class extension_t;
class disassembler_t;

struct insn_desc_t
{
  insn_bits_t match;
  insn_bits_t mask;
  insn_func_t rv32;
  insn_func_t rv64;
  insn_func_raw_t is_raw;
};

struct commit_log_reg_t
{
  reg_t addr;
  freg_t data;
};

struct commit_log_mem_t
{
  reg_t addr;
  uint64_t value;
  uint8_t size; // bytes: 1, 2, 4, or 8
};

typedef struct
{
  uint8_t prv;
  bool step;
  bool ebreakm;
  bool ebreakh;
  bool ebreaks;
  bool ebreaku;
  bool halt;
  uint8_t cause;
} dcsr_t;

typedef enum
{
  ACTION_DEBUG_EXCEPTION = MCONTROL_ACTION_DEBUG_EXCEPTION,
  ACTION_DEBUG_MODE = MCONTROL_ACTION_DEBUG_MODE,
  ACTION_TRACE_START = MCONTROL_ACTION_TRACE_START,
  ACTION_TRACE_STOP = MCONTROL_ACTION_TRACE_STOP,
  ACTION_TRACE_EMIT = MCONTROL_ACTION_TRACE_EMIT
} mcontrol_action_t;

typedef enum
{
  MATCH_EQUAL = MCONTROL_MATCH_EQUAL,
  MATCH_NAPOT = MCONTROL_MATCH_NAPOT,
  MATCH_GE = MCONTROL_MATCH_GE,
  MATCH_LT = MCONTROL_MATCH_LT,
  MATCH_MASK_LOW = MCONTROL_MATCH_MASK_LOW,
  MATCH_MASK_HIGH = MCONTROL_MATCH_MASK_HIGH
} mcontrol_match_t;

typedef struct
{
  uint8_t type;
  bool dmode;
  uint8_t maskmax;
  bool select;
  bool timing;
  mcontrol_action_t action;
  bool chain;
  mcontrol_match_t match;
  bool m;
  bool h;
  bool s;
  bool u;
  bool execute;
  bool store;
  bool load;
} mcontrol_t;

inline reg_t BITS(reg_t v, int hi, int lo){
  return (v >> lo) & ((2 << (hi - lo)) - 1);
}

enum VRM{
  RNU = 0,
  RNE,
  RDN,
  ROD,
  INVALID_RM
};

template<uint64_t N>
struct type_usew_t;

template<>
struct type_usew_t<8>
{
  using type=uint8_t;
};

template<>
struct type_usew_t<16>
{
  using type=uint16_t;
};

template<>
struct type_usew_t<32>
{
  using type=uint32_t;
};

template<>
struct type_usew_t<64>
{
  using type=uint64_t;
};

template<uint64_t N>
struct type_sew_t;

template<>
struct type_sew_t<8>
{
  using type=int8_t;
};

template<>
struct type_sew_t<16>
{
  using type=int16_t;
};

template<>
struct type_sew_t<32>
{
  using type=int32_t;
};

template<>
struct type_sew_t<64>
{
  using type=int64_t;
};

typedef struct load_insn_raw_dep
{
    reg_t regId;
    spike_model::Request::RegType regType;
    uint64_t latency;
}load_insn_raw_dep;

class vectorUnit_t {
  public:
    processor_t* p;
    void *reg_file;
    void *dummy_reg;
    char reg_referenced[NVPR];
    int setvl_count;
    reg_t reg_mask, vlmax, vmlen;
    reg_t vstart, vxrm, vxsat, vl, vtype, vlenb;
    reg_t vediv, vsew, vlmul;
    reg_t ELEN, VLEN, SLEN, PVL=0;
    reg_t curr_rd, curr_RS1;
    reg_t curr_AVL, curr_new_type;

    bool vill;
    std::unordered_map<uint64_t, int> read_reg_encountered;
    std::unordered_map<uint64_t, int> write_reg_encountered;

    template<class T>
      T& elt(reg_t vReg, reg_t n, vreg_access_type access_type){
        assert(vsew != 0);
        assert((VLEN >> 3)/sizeof(T) > 0);
        reg_t elts_per_reg = (VLEN >> 3) / (sizeof(T));
        vReg += n / elts_per_reg;
        n = n % elts_per_reg;
#ifdef WORDS_BIGENDIAN
	// "V" spec 0.7.1 requires lower indices to map to lower significant
	// bits when changing SEW, thus we need to index from the end on BE.
	n ^= elts_per_reg - 1;
#endif
        reg_referenced[vReg] = 1;
        T *regStart = (T*)((char*)reg_file + vReg * (VLEN >> 3));

        if(access_type == VWRITE || access_type == VREADWRITE)
        {
          if(write_reg_encountered.find(vReg) == write_reg_encountered.end())
          {
            write_reg_encountered[vReg] = 1;
            /*
             Set the destination register also, because once the acknowledge is done,
             we have to set the availability of this register.
            */
            p->curr_write_reg = vReg;
            p->curr_write_reg_type = spike_model::Request::RegType::VECTOR;
          }
        }
        return regStart[n];
      }

      uint64_t get_avail_cycle(reg_t i)
      {
        return avail_cycle[i];
      }

      void clear_bookkeeping_regs()
      { 
        read_reg_encountered.clear();
        write_reg_encountered.clear();
      }

      template<class T>
      bool check_raw(reg_t vReg, reg_t n)
      {
        reg_t elts_per_reg = (VLEN >> 3) / (sizeof(T));
        vReg += n / elts_per_reg;

        if(get_avail_cycle(vReg)>p->get_current_cycle())
        {
          p->get_state()->raw=true;

          /*Push the data into the vector if the depending
            instruction is a compute instruction.
            If the depending instruction is a load, a separate
            cacheRequest event would be generated */

          if(get_avail_cycle(vReg) != std::numeric_limits<uint64_t>::max())
          {
            std::shared_ptr<spike_model::InsnLatencyEvent> insn_latency_ptr =
                       std::make_shared<spike_model::InsnLatencyEvent>(
                       p->get_state()->pc,
                       p->get_id(),
                       vReg,
                       spike_model::Request::RegType::VECTOR,
                       std::numeric_limits<uint64_t>::max(),
                       p->get_curr_insn_latency(),
                       get_avail_cycle(vReg));
            p->push_insn_latency_event(insn_latency_ptr);
          }
          p->get_state()->pending_vector_regs->insert(vReg);
          return true;
        }
        return false;
      }
    
      void set_busy_until(uint64_t t)
      {
          busy_until=t;
      }
    
      bool is_busy(uint64_t t);

  private:
    uint64_t avail_cycle[NVPR]={0};
    uint8_t pending_events[NVPR]={0};
    uint64_t busy_until=0;

  public:

    void reset();

    vectorUnit_t(){
      reg_file = 0;
      dummy_reg=0;
    }

    ~vectorUnit_t(){
      free(reg_file);
      free(dummy_reg);
      reg_file = 0;
      dummy_reg=0;
    }

    reg_t set_vl(int rd, int rs1, reg_t reqVL, reg_t newType);
    void get_vvl(int rd, int rs1, reg_t req_vvl, reg_t newType);
    void set_vvl(reg_t granted_vvl);

    reg_t get_vlen() { return VLEN; }
    reg_t get_elen() { return ELEN; }
    reg_t get_slen() { return SLEN; }

    VRM get_vround_mode() {
      return (VRM)vxrm;
    }

    reg_t get_vl() {
      return vl;
    }

    void set_avail(reg_t i, uint64_t cycle)
    {
      //if (!zero_reg || i != 0)
      avail_cycle[i] = cycle;
    }

    bool ack_for_reg(reg_t i, uint64_t cycle)
    {
      bool res=false;
      pending_events[i]--;
      if(pending_events[i]==0)
      {
        set_avail(i, cycle);
        res=true; 
      }
      return res;
    }

    /* num_events == 0 => compute instruction. Pending event should be 0
                   because at the time of writing the register,
                   it is not known if this register would be used in future
     num_events >= 0 => memory instruction. Pending event is equal to
                   number of misses.
     cycle < max => compute instruction. Set to the instruction latency.
     cycle == max => memory instruction. The actual cycles is not known
              at the time of writing to the register.
    */
    void set_event_dependent(reg_t i, uint8_t num_events, uint64_t cycle)
    {
      pending_events[i]=num_events;
      avail_cycle[i] = cycle;
    }

};

// architectural state of a RISC-V hart
struct state_t
{
  void reset(reg_t max_isa);

  static const int num_triggers = 4;

  reg_t pc;
  regfile_t<reg_t, NXPR, true> XPR;
  regfile_t<freg_t, NFPR, false> FPR;

  // control and status registers
  reg_t prv;    // TODO: Can this be an enum instead?
  reg_t misa;
  reg_t mstatus;
  reg_t mepc;
  reg_t mtval;
  reg_t mscratch;
  reg_t mtvec;
  reg_t mcause;
  reg_t minstret;
  reg_t mie;
  reg_t mip;
  reg_t medeleg;
  reg_t mideleg;
  uint32_t mcounteren;
  uint32_t scounteren;
  reg_t sepc;
  reg_t stval;
  reg_t sscratch;
  reg_t stvec;
  reg_t satp;
  reg_t scause;

  reg_t dpc;
  reg_t dscratch0, dscratch1;
  dcsr_t dcsr;
  reg_t tselect;
  mcontrol_t mcontrol[num_triggers];
  reg_t tdata2[num_triggers];
  bool debug_mode;

  static const int n_pmp = 16;
  uint8_t pmpcfg[n_pmp];
  reg_t pmpaddr[n_pmp];

  uint32_t fflags;
  uint32_t frm;
  bool serialized; // whether timer CSRs are in a well-defined state

  bool raw=false;
  std::set<size_t> * pending_int_regs;
  std::set<size_t> * pending_float_regs;
  std::set<size_t> * pending_vector_regs;

  // When true, execute a single instruction and then enter debug mode.  This
  // can only be set by executing dret.
  enum {
      STEP_NONE,
      STEP_STEPPING,
      STEP_STEPPED
  } single_step;

#ifdef RISCV_ENABLE_COMMITLOG
  commit_log_reg_t log_reg_write;
  commit_log_mem_t log_mem_read;
  commit_log_mem_t log_mem_write;
  reg_t last_inst_priv;
  int last_inst_xlen;
  int last_inst_flen;
#endif
};

typedef enum {
  OPERATION_EXECUTE,
  OPERATION_STORE,
  OPERATION_LOAD,
} trigger_operation_t;

// Count number of contiguous 1 bits starting from the LSB.
static int cto(reg_t val)
{
  int res = 0;
  while ((val & 1) == 1)
    val >>= 1, res++;
  return res;
}

// this class represents one processor in a RISC-V machine.
class processor_t : public abstract_device_t
{
public:
  processor_t(const char* isa, const char* priv, const char* varch,
              simif_t* sim, uint32_t id, bool halt_on_reset=false, bool enable_smart_mcpu=false, 
              bool vector_bypass_l1=true, bool vector_bypass_l2=false, uint16_t lanes_per_vpu=8,
              size_t scratchpad_size = 0);
  ~processor_t();

  void set_debug(bool value);
  void set_histogram(bool value);
  void set_log_commits(bool value);
  bool get_log_commits() { return log_commits_enabled; }
  void reset();
  bool step(size_t n); // run for n cycles
  void set_csr(int which, reg_t val);
  reg_t get_csr(int which);
  mmu_t* get_mmu() { return mmu; }
  state_t* get_state() { return &state; }
  unsigned get_xlen() { return xlen; }
  unsigned get_max_xlen() { return max_xlen; }
  std::string get_isa_string() { return isa_string; }
  unsigned get_flen() {
    return supports_extension('Q') ? 128 :
           supports_extension('D') ? 64 :
           supports_extension('F') ? 32 : 0;
  }
  extension_t* get_extension() { return ext; }
  bool supports_extension(unsigned char ext) {
    if (ext >= 'a' && ext <= 'z') ext += 'A' - 'a';
    return ext >= 'A' && ext <= 'Z' && ((state.misa >> (ext - 'A')) & 1);
  }
  reg_t pc_alignment_mask() {
    return ~(reg_t)(supports_extension('C') ? 0 : 2);
  }
  void check_pc_alignment(reg_t pc) {
    if (unlikely(pc & ~pc_alignment_mask()))
      throw trap_instruction_address_misaligned(pc);
  }
  reg_t legalize_privilege(reg_t);
  void set_privilege(reg_t);
  void update_histogram(reg_t pc);
  const disassembler_t* get_disassembler() { return disassembler; }

  void register_insn(insn_desc_t);
  void register_extension(extension_t*);

  // MMIO slave interface
  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);

  // When true, display disassembly of each instruction that's executed.
  bool debug;
  // When true, take the slow simulation path.
  bool slow_path();
  bool halted() { return state.debug_mode; }
  bool halt_request;

  // Return the index of a trigger that matched, or -1.
  inline int trigger_match(trigger_operation_t operation, reg_t address, reg_t data)
  {
    if (state.debug_mode)
      return -1;

    bool chain_ok = true;

    for (unsigned int i = 0; i < state.num_triggers; i++) {
      if (!chain_ok) {
        chain_ok |= !state.mcontrol[i].chain;
        continue;
      }

      if ((operation == OPERATION_EXECUTE && !state.mcontrol[i].execute) ||
          (operation == OPERATION_STORE && !state.mcontrol[i].store) ||
          (operation == OPERATION_LOAD && !state.mcontrol[i].load) ||
          (state.prv == PRV_M && !state.mcontrol[i].m) ||
          (state.prv == PRV_S && !state.mcontrol[i].s) ||
          (state.prv == PRV_U && !state.mcontrol[i].u)) {
        continue;
      }

      reg_t value;
      if (state.mcontrol[i].select) {
        value = data;
      } else {
        value = address;
      }

      // We need this because in 32-bit mode sometimes the PC bits get sign
      // extended.
      if (xlen == 32) {
        value &= 0xffffffff;
      }

      switch (state.mcontrol[i].match) {
        case MATCH_EQUAL:
          if (value != state.tdata2[i])
            continue;
          break;
        case MATCH_NAPOT:
          {
            reg_t mask = ~((1 << (cto(state.tdata2[i])+1)) - 1);
            if ((value & mask) != (state.tdata2[i] & mask))
              continue;
          }
          break;
        case MATCH_GE:
          if (value < state.tdata2[i])
            continue;
          break;
        case MATCH_LT:
          if (value >= state.tdata2[i])
            continue;
          break;
        case MATCH_MASK_LOW:
          {
            reg_t mask = state.tdata2[i] >> (xlen/2);
            if ((value & mask) != (state.tdata2[i] & mask))
              continue;
          }
          break;
        case MATCH_MASK_HIGH:
          {
            reg_t mask = state.tdata2[i] >> (xlen/2);
            if (((value >> (xlen/2)) & mask) != (state.tdata2[i] & mask))
              continue;
          }
          break;
      }

      if (!state.mcontrol[i].chain) {
        return i;
      }
      chain_ok = true;
    }
    return -1;
  }

  void trigger_updated();

  bool miss_log_enabled() {return log_misses;} //BORJA

  void enable_miss_log() {log_misses=true;}

  void set_current_cycle(uint64_t c){current_cycle=c;};
  uint64_t get_current_cycle();

  uint64_t get_curr_insn_latency();
  
  void set_vpu_latency_considering_lanes();

  uint16_t get_id() {return id;}

  void log_mcpu_instruction(uint64_t base_address, size_t width, bool store);
  void set_mcpu_instruction_indexed(std::vector<uint64_t> indices);
  void set_mcpu_instruction_strided(std::vector<uint64_t> indices);
  void reset_mcpu_instruction();

  bool XPR_CHECK_RAW(size_t reg)
  {
    if(read_reg_encountered.find(reg) == read_reg_encountered.end())
    {
      read_reg_encountered[reg] = 1;
      if(state.XPR.get_avail_cycle(reg) > get_current_cycle())
      {
        state.raw=true; 
        if(state.XPR.get_avail_cycle(reg) != std::numeric_limits<uint64_t>::max())
        {
          std::shared_ptr<spike_model::InsnLatencyEvent> insn_latency_ptr = 
                     std::make_shared<spike_model::InsnLatencyEvent>(
                     state.pc,
                     get_id(),
                     reg,
                     spike_model::Request::RegType::INTEGER,
                     std::numeric_limits<uint64_t>::max(),
                     get_curr_insn_latency(),
                     state.XPR.get_avail_cycle(reg));
          push_insn_latency_event(insn_latency_ptr);
        }
        state.pending_int_regs->insert(reg);
        return true;
      }
      return false;
    }
    return false;
  }

  bool FPR_CHECK_RAW(size_t reg)
  {
    if(read_freg_encountered.find(reg) == read_freg_encountered.end())
    {
      read_freg_encountered[reg] = 1;
      if(state.FPR.get_avail_cycle(reg) > get_current_cycle())
      {
        state.raw=true; 
        if(state.FPR.get_avail_cycle(reg) != std::numeric_limits<uint64_t>::max())
        {
          std::shared_ptr<spike_model::InsnLatencyEvent> insn_latency_ptr = 
                     std::make_shared<spike_model::InsnLatencyEvent>(
                     state.pc,
                     get_id(),
                     reg,
                     spike_model::Request::RegType::FLOAT,
                     std::numeric_limits<uint64_t>::max(),
                     get_curr_insn_latency(),
                     state.FPR.get_avail_cycle(reg));
          push_insn_latency_event(insn_latency_ptr);
        }
        state.pending_float_regs->insert(reg);
        return true;
      }
      return false;
    }
    return false;
  }


  bool is_mcpu_instruction();
  std::shared_ptr<spike_model::MCPUInstruction> get_mcpu_instruction();

  void sim_fence_log();
  bool is_in_fence();
  bool is_in_set_vl();

  std::list<std::shared_ptr<spike_model::InsnLatencyEvent>>& get_insn_latency_event_list()
  {
    return insn_latency_event_list;
  }

  void clear_latency_event_list()
  {
    insn_latency_event_list.clear();
  }

  void push_insn_latency_event(std::shared_ptr<spike_model::InsnLatencyEvent> insn_latency_ptr)
  {
    insn_latency_event_list.push_back(insn_latency_ptr);
  }

  void clear_bookkeeping_regs()
  {
    read_reg_encountered.clear();
    write_reg_encountered.clear();
    read_freg_encountered.clear();
    write_freg_encountered.clear();
  }

  simif_t* sim;
  insn_func_raw_t is_raw;
  bool enable_smart_mcpu, is_load, is_store, vector_bypass_l1, vector_bypass_l2;
  uint16_t lanes_per_vpu;
  size_t scratchpad_size;
  int curr_insn_latency;
  reg_t curr_write_reg;
  spike_model::Request::RegType curr_write_reg_type;
private:
  
  mmu_t* mmu; // main memory is always accessed via the mmu
  extension_t* ext;
  disassembler_t* disassembler;
  state_t state;
  uint16_t id;
  unsigned max_xlen;
  unsigned xlen;
  reg_t max_isa;
  std::string isa_string;
  bool histogram_enabled;
  bool log_commits_enabled;
  bool halt_on_reset;

  std::vector<insn_desc_t> instructions;
  std::map<reg_t,uint64_t> pc_histogram;

  static const size_t OPCODE_CACHE_SIZE = 8191;
  insn_desc_t opcode_cache[OPCODE_CACHE_SIZE];

  void take_pending_interrupt() { take_interrupt(state.mip & state.mie); }
  void take_interrupt(reg_t mask); // take first enabled interrupt in mask
  void take_trap(trap_t& t, reg_t epc); // take an exception
  void disasm(insn_t insn); // disassemble and print an instruction
  int paddr_bits();

  void enter_debug_mode(uint8_t cause);

  friend class mmu_t;
  friend class clint_t;
  friend class extension_t;

  void parse_varch_string(const char*);
  void parse_priv_string(const char*);
  void parse_isa_string(const char*);
  void build_opcode_map();
  void register_base_instructions();
  insn_func_t decode_insn(insn_t insn);

  // Track repeated executions for processor_t::disasm()
  uint64_t last_pc, last_bits, executions;
 
  bool log_misses=false;
  bool in_fence=false;

  std::list<std::shared_ptr<spike_model::CacheRequest>> pending_misses;
  uint64_t current_cycle;

  std::shared_ptr<spike_model::MCPUInstruction> mcpu_instruction=nullptr;

  /*There is a possibility of duplicate events getting inserted in the list.
    This could result into corruption in the RAW event tracking mechanism.
    Lets use set to avoid any duplicates.
  */
  std::list<std::shared_ptr<spike_model::InsnLatencyEvent>>
             insn_latency_event_list;

public:
  vectorUnit_t VU;

  //READ_REG, READ_FREG etc are all macros. Each of their 
  //use invokes the corresponding macros, even for the same register. So we use 
  //map to keep track of which registers are already referred to in the current
  //instruction
  std::unordered_map<uint64_t, int> read_reg_encountered;
  std::unordered_map<uint64_t, int> write_reg_encountered;
  std::unordered_map<uint64_t, int> read_freg_encountered;
  std::unordered_map<uint64_t, int> write_freg_encountered;
  bool in_set_vl = false;
  bool is_vl_available = true;
};


reg_t illegal_instruction(processor_t* p, insn_t insn, reg_t pc);

#define REGISTER_INSN(proc, name, match, mask) \
  extern reg_t rv32_##name(processor_t*, insn_t, reg_t); \
  extern reg_t rv64_##name(processor_t*, insn_t, reg_t); \
  extern bool is_raw_##name(processor_t*, insn_t, reg_t); \
  proc->register_insn((insn_desc_t){match, mask, rv32_##name, rv64_##name, is_raw_##name});

#endif
