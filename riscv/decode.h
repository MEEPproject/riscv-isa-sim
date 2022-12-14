// See LICENSE for license details.

#ifndef _RISCV_DECODE_H
#define _RISCV_DECODE_H

#if (-1 != ~0) || ((-1 >> 1) != -1)
# error spike requires a two''s-complement c++ implementation
#endif

#include <algorithm>
#include <cstdint>
#include <string.h>
#include <strings.h>
#include "encoding.h"
#include "config.h"
#include "common.h"
#include "softfloat_types.h"
#include "specialize.h"
#include <cinttypes>

#include <iostream>

enum vreg_access_type {VREAD, VWRITE, VREADWRITE};
typedef int64_t sreg_t;
typedef uint64_t reg_t;

#ifdef __SIZEOF_INT128__
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;
#endif

const int NXPR = 32;
const int NFPR = 32;
const int NVPR = 32;
const int NCSR = 4096;

#define X_RA 1
#define X_SP 2

#define FSR_VXRM_SHIFT 9
#define FSR_VXRM  (0x3 << FSR_VXRM_SHIFT)

#define FSR_VXSAT_SHIFT 8
#define FSR_VXSAT  (0x1 << FSR_VXSAT_SHIFT)

#define FP_RD_NE  0
#define FP_RD_0   1
#define FP_RD_DN  2
#define FP_RD_UP  3
#define FP_RD_NMM 4

#define FSR_RD_SHIFT 5
#define FSR_RD   (0x7 << FSR_RD_SHIFT)

#define FPEXC_NX 0x01
#define FPEXC_UF 0x02
#define FPEXC_OF 0x04
#define FPEXC_DZ 0x08
#define FPEXC_NV 0x10

#define FSR_AEXC_SHIFT 0
#define FSR_NVA  (FPEXC_NV << FSR_AEXC_SHIFT)
#define FSR_OFA  (FPEXC_OF << FSR_AEXC_SHIFT)
#define FSR_UFA  (FPEXC_UF << FSR_AEXC_SHIFT)
#define FSR_DZA  (FPEXC_DZ << FSR_AEXC_SHIFT)
#define FSR_NXA  (FPEXC_NX << FSR_AEXC_SHIFT)
#define FSR_AEXC (FSR_NVA | FSR_OFA | FSR_UFA | FSR_DZA | FSR_NXA)

#define insn_length(x) \
  (((x) & 0x03) < 0x03 ? 2 : \
   ((x) & 0x1f) < 0x1f ? 4 : \
   ((x) & 0x3f) < 0x3f ? 6 : \
   8)
#define MAX_INSN_LENGTH 8
#define PC_ALIGN 2

typedef uint64_t insn_bits_t;
class insn_t
{
public:
  insn_t() = default;
  insn_t(insn_bits_t bits) : b(bits) {}
  insn_bits_t bits() { return b; }
  int length() { return insn_length(b); }
  int64_t i_imm() { return int64_t(b) >> 20; }
  int64_t shamt() { return x(20, 6); }
  int64_t s_imm() { return x(7, 5) + (xs(25, 7) << 5); }
  int64_t sb_imm() { return (x(8, 4) << 1) + (x(25,6) << 5) + (x(7,1) << 11) + (imm_sign() << 12); }
  int64_t u_imm() { return int64_t(b) >> 12 << 12; }
  int64_t uj_imm() { return (x(21, 10) << 1) + (x(20, 1) << 11) + (x(12, 8) << 12) + (imm_sign() << 20); }
  uint64_t rd() { return x(7, 5); }
  uint64_t rs1() { return x(15, 5); }
  uint64_t rs2() { return x(20, 5); }
  uint64_t rs3() { return x(27, 5); }
  uint64_t rm() { return x(12, 3); }
  uint64_t csr() { return x(20, 12); }

  int64_t rvc_imm() { return x(2, 5) + (xs(12, 1) << 5); }
  int64_t rvc_zimm() { return x(2, 5) + (x(12, 1) << 5); }
  int64_t rvc_addi4spn_imm() { return (x(6, 1) << 2) + (x(5, 1) << 3) + (x(11, 2) << 4) + (x(7, 4) << 6); }
  int64_t rvc_addi16sp_imm() { return (x(6, 1) << 4) + (x(2, 1) << 5) + (x(5, 1) << 6) + (x(3, 2) << 7) + (xs(12, 1) << 9); }
  int64_t rvc_lwsp_imm() { return (x(4, 3) << 2) + (x(12, 1) << 5) + (x(2, 2) << 6); }
  int64_t rvc_ldsp_imm() { return (x(5, 2) << 3) + (x(12, 1) << 5) + (x(2, 3) << 6); }
  int64_t rvc_swsp_imm() { return (x(9, 4) << 2) + (x(7, 2) << 6); }
  int64_t rvc_sdsp_imm() { return (x(10, 3) << 3) + (x(7, 3) << 6); }
  int64_t rvc_lw_imm() { return (x(6, 1) << 2) + (x(10, 3) << 3) + (x(5, 1) << 6); }
  int64_t rvc_ld_imm() { return (x(10, 3) << 3) + (x(5, 2) << 6); }
  int64_t rvc_j_imm() { return (x(3, 3) << 1) + (x(11, 1) << 4) + (x(2, 1) << 5) + (x(7, 1) << 6) + (x(6, 1) << 7) + (x(9, 2) << 8) + (x(8, 1) << 10) + (xs(12, 1) << 11); }
  int64_t rvc_b_imm() { return (x(3, 2) << 1) + (x(10, 2) << 3) + (x(2, 1) << 5) + (x(5, 2) << 6) + (xs(12, 1) << 8); }
  int64_t rvc_simm3() { return x(10, 3); }
  uint64_t rvc_rd() { return rd(); }
  uint64_t rvc_rs1() { return rd(); }
  uint64_t rvc_rs2() { return x(2, 5); }
  uint64_t rvc_rs1s() { return 8 + x(7, 3); }
  uint64_t rvc_rs2s() { return 8 + x(2, 3); }

  uint64_t v_vm() { return x(25, 1); }
  uint64_t v_nf() { return x(29, 3); }
  uint64_t v_simm5() { return xs(15, 5); }
  uint64_t v_zimm5() { return x(15, 5); }
  uint64_t v_zimm11() { return x(20, 11); }
  uint64_t v_lmul() { return 1 << x(20, 2); }
  uint64_t v_sew() { return 1 << (x(22, 3) + 3); }

private:
  insn_bits_t b;
  uint64_t x(int lo, int len) { return (b >> lo) & ((insn_bits_t(1) << len)-1); }
  uint64_t xs(int lo, int len) { return int64_t(b) << (64-lo-len) >> (64-len); }
  uint64_t imm_sign() { return xs(63, 1); }
};

template <class T, size_t N, bool zero_reg>
class regfile_t
{
public:
  void write(size_t i, T value)
  {
    if (!zero_reg || i != 0)
      data[i] = value;
  }

  T read(size_t i)
  {
      return data[i];
  }

  uint64_t get_avail_cycle(size_t i)
  {
    return avail_cycle[i];
  }

  void set_avail(size_t i, uint64_t cycle)
  {
    if (!zero_reg || i != 0)
    {
      avail_cycle[i] = cycle;
    }
  }

  bool ack_for_reg(size_t i, uint64_t cycle)
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
  void set_event_dependent(size_t i, uint8_t num_events, uint64_t cycle)
  {
    pending_events[i]=num_events;
    if (!zero_reg || i != 0)
      avail_cycle[i] = cycle;
  }

  const T& operator [] (size_t i) const
  {
    return data[i];
  }

private:
  T data[N];
  uint64_t avail_cycle[N]={0};
  uint8_t pending_events[N]={0};
};

// helpful macros, etc
#define MMU (*p->get_mmu())
#define STATE (*p->get_state())
#define P_ (*p)
#define FLEN (p->get_flen())

//If raw exist, then the timing of dependent instruction will also change.
//Possibilities
// R1 = R1 + R2 (READ and WRITE to the same register)
// R1 = R2 + R3 (Both of them have RAW)
// Need to have ackRegister mechanism. Should resume the core only when
// all of them are available
//R2 + R3 (if both of them are compute and has a raw, we also have to acknowledge these reg
//otherwise we dont know when to make the core active
//Cannot set the timing of the dest reg, if there is a RAW. Needs to be set
//only when its sources are serviced
#define READ_REG(reg) ({ \
        STATE.XPR[reg]; \
    })

#define READ_FREG(reg) ({ \
        STATE.FPR[reg]; \
    })

#define RD READ_REG(insn.rd())
#define RS1 READ_REG(insn.rs1())
#define RS2 READ_REG(insn.rs2())
#define RS3 READ_REG(insn.rs3())
#define WRITE_RD(value) WRITE_REG(insn.rd(), value)


#ifndef RISCV_ENABLE_COMMITLOG
# define WRITE_REG(reg, value) ({ \
        STATE.XPR.write(reg, value); \
        if(P_.write_reg_encountered.find(reg) == P_.write_reg_encountered.end()) \
        { \
          P_.write_reg_encountered[reg] = 1; \
          /*
            Set the destination register also, because once the acknowledge is done, \
            we have to set the availability of this register. \
          */ \
          P_.curr_write_reg = reg; \
          P_.curr_write_reg_type = coyote::Request::RegType::INTEGER; \
        } \
    })

# define WRITE_FREG(reg, value) DO_WRITE_FREG(reg, freg(value))
#else
# define WRITE_REG(reg, value) ({ \
    reg_t wdata = (value); /* value may have side effects */ \
    STATE.log_reg_write = (commit_log_reg_t){(reg) << 1, {wdata, 0}}; \
    STATE.XPR.write(reg, wdata); \
    if(P_.write_reg_encountered.find(reg) == P_.write_reg_encountered.end()) \
    { \
      P_.write_reg_encountered[reg] = 1; \
      /*
        Set the destination register also, because once the acknowledge is done, \
        we have to set the availability of this register. \
      */ \
      P_.curr_write_reg = reg; \
      P_.curr_write_reg_type = coyote::Request::RegType::INTEGER; \
    } \
  })

# define WRITE_FREG(reg, value) ({ \
    freg_t wdata = freg(value); /* value may have side effects */ \
    STATE.log_reg_write = (commit_log_reg_t){((reg) << 1) | 1, wdata}; \
    DO_WRITE_FREG(reg, wdata); \
  })
#endif

// RVC macros
#define WRITE_RVC_RS1S(value) WRITE_REG(insn.rvc_rs1s(), value)
#define WRITE_RVC_RS2S(value) WRITE_REG(insn.rvc_rs2s(), value)
#define WRITE_RVC_FRS2S(value) WRITE_FREG(insn.rvc_rs2s(), value)
#define RVC_RS1 READ_REG(insn.rvc_rs1())
#define RVC_RS2 READ_REG(insn.rvc_rs2())
#define RVC_RS1S READ_REG(insn.rvc_rs1s())
#define RVC_RS2S READ_REG(insn.rvc_rs2s())
#define RVC_FRS2 READ_FREG(insn.rvc_rs2())
#define RVC_FRS2S READ_FREG(insn.rvc_rs2s())
#define RVC_SP READ_REG(X_SP)

// FPU macros
#define FRS1 READ_FREG(insn.rs1())
#define FRS2 READ_FREG(insn.rs2())
#define FRS3 READ_FREG(insn.rs3())
#define dirty_fp_state (STATE.mstatus |= MSTATUS_FS | (xlen == 64 ? MSTATUS64_SD : MSTATUS32_SD))
#define dirty_ext_state (STATE.mstatus |= MSTATUS_XS | (xlen == 64 ? MSTATUS64_SD : MSTATUS32_SD))
#define dirty_vs_state (STATE.mstatus |= MSTATUS_VS | (xlen == 64 ? MSTATUS64_SD : MSTATUS32_SD))
#define DO_WRITE_FREG(reg, value) ({ \
        (STATE.FPR.write(reg, value), dirty_fp_state); \
        if(P_.write_freg_encountered.find(reg) == P_.write_freg_encountered.end()) \
        { \
          P_.write_freg_encountered[reg] = 1; \
          /*
            Set the destination register also, because once the acknowledge is done, \
            we have to set the availability of this register. \
          */ \
          P_.curr_write_reg = reg;\
          P_.curr_write_reg_type = coyote::Request::RegType::FLOAT;\
        } \
    })

#define WRITE_FRD(value) WRITE_FREG(insn.rd(), value)

#define SHAMT (insn.i_imm() & 0x3F)
#define BRANCH_TARGET (pc + insn.sb_imm())
#define JUMP_TARGET (pc + insn.uj_imm())
#define RM ({ int rm = insn.rm(); \
              if(rm == 7) rm = STATE.frm; \
              if(rm > 4) throw trap_illegal_instruction(0); \
              rm; })

#define get_field(reg, mask) (((reg) & (decltype(reg))(mask)) / ((mask) & ~((mask) << 1)))
#define set_field(reg, mask, val) (((reg) & ~(decltype(reg))(mask)) | (((decltype(reg))(val) * ((mask) & ~((mask) << 1))) & (decltype(reg))(mask)))

#define require(x) if (unlikely(!(x))) throw trap_illegal_instruction(0)
#define require_privilege(p) require(STATE.prv >= (p))
#define require_rv64 require(xlen == 64)
#define require_rv32 require(xlen == 32)
#define require_extension(s) require(p->supports_extension(s))
#define require_fp require((STATE.mstatus & MSTATUS_FS) != 0)
#define require_accelerator require((STATE.mstatus & MSTATUS_XS) != 0)

#define require_vector_vs require((STATE.mstatus & MSTATUS_VS) != 0);
#define require_vector \
  do { \
    require_vector_vs; \
    require_extension('V'); \
    require(!P_.VU.vill); \
    dirty_vs_state; \
  } while (0);
#define require_vector_for_vsetvl \
  do {  \
    require_vector_vs; \
    require_extension('V'); \
    dirty_vs_state; \
  } while (0);

#define set_fp_exceptions ({ if (softfloat_exceptionFlags) { \
                               dirty_fp_state; \
                               STATE.fflags |= softfloat_exceptionFlags; \
                             } \
                             softfloat_exceptionFlags = 0; })

#define sext32(x) ((sreg_t)(int32_t)(x))
#define zext32(x) ((reg_t)(uint32_t)(x))
#define sext_xlen(x) (((sreg_t)(x) << (64-xlen)) >> (64-xlen))
#define zext_xlen(x) (((reg_t)(x) << (64-xlen)) >> (64-xlen))

#define set_pc(x) \
  do { p->check_pc_alignment(x); \
       npc = sext_xlen(x); \
     } while(0)

#define set_pc_and_serialize(x) \
  do { reg_t __npc = (x) & p->pc_alignment_mask(); \
       npc = PC_SERIALIZE_AFTER; \
       STATE.pc = __npc; \
     } while(0)

class wait_for_interrupt_t {};

#define wfi() \
  do { set_pc_and_serialize(npc); \
       npc = PC_SERIALIZE_WFI; \
       throw wait_for_interrupt_t(); \
     } while(0)

#define serialize() set_pc_and_serialize(npc)

/* Sentinel PC values to serialize simulator pipeline */
#define PC_SERIALIZE_BEFORE 3
#define PC_SERIALIZE_AFTER 5
#define PC_SERIALIZE_WFI 7
#define invalid_pc(pc) ((pc) & 1)

/* Convenience wrappers to simplify softfloat code sequences */
#define isBoxedF32(r) (isBoxedF64(r) && ((uint32_t)((r.v[0] >> 32) + 1) == 0))
#define unboxF32(r) (isBoxedF32(r) ? (uint32_t)r.v[0] : defaultNaNF32UI)
#define isBoxedF64(r) ((r.v[1] + 1) == 0)
#define unboxF64(r) (isBoxedF64(r) ? r.v[0] : defaultNaNF64UI)
typedef float128_t freg_t;
inline float32_t f32(uint32_t v) { return { v }; }
inline float64_t f64(uint64_t v) { return { v }; }
inline float32_t f32(freg_t r) { return f32(unboxF32(r)); }
inline float64_t f64(freg_t r) { return f64(unboxF64(r)); }
inline float128_t f128(freg_t r) { return r; }
inline freg_t freg(float32_t f) { return { ((uint64_t)-1 << 32) | f.v, (uint64_t)-1 }; }
inline freg_t freg(float64_t f) { return { f.v, (uint64_t)-1 }; }
inline freg_t freg(float128_t f) { return f; }
#define F32_SIGN ((uint32_t)1 << 31)
#define F64_SIGN ((uint64_t)1 << 63)
#define fsgnj32(a, b, n, x) \
  f32((f32(a).v & ~F32_SIGN) | ((((x) ? f32(a).v : (n) ? F32_SIGN : 0) ^ f32(b).v) & F32_SIGN))
#define fsgnj64(a, b, n, x) \
  f64((f64(a).v & ~F64_SIGN) | ((((x) ? f64(a).v : (n) ? F64_SIGN : 0) ^ f64(b).v) & F64_SIGN))

#define isNaNF128(x) isNaNF128UI(x.v[1], x.v[0])
inline float128_t defaultNaNF128()
{
  float128_t nan;
  nan.v[1] = defaultNaNF128UI64;
  nan.v[0] = defaultNaNF128UI0;
  return nan;
}
inline freg_t fsgnj128(freg_t a, freg_t b, bool n, bool x)
{
  a.v[1] = (a.v[1] & ~F64_SIGN) | (((x ? a.v[1] : n ? F64_SIGN : 0) ^ b.v[1]) & F64_SIGN);
  return a;
}
inline freg_t f128_negate(freg_t a)
{
  a.v[1] ^= F64_SIGN;
  return a;
}

#define validate_csr(which, write) ({ \
  if (!STATE.serialized) return PC_SERIALIZE_BEFORE; \
  STATE.serialized = false; \
  unsigned csr_priv = get_field((which), 0x300); \
  bool mode_unsupported = csr_priv == PRV_S && !P_.supports_extension('S'); \
  unsigned csr_read_only = get_field((which), 0xC00) == 3; \
  if (((write) && csr_read_only) || STATE.prv < csr_priv || mode_unsupported) \
    throw trap_illegal_instruction(0); \
  (which); })

/* For debug only. This will fail if the native machine's float types are not IEEE */
inline float to_f(float32_t f){float r; memcpy(&r, &f, sizeof(r)); return r;}
inline double to_f(float64_t f){double r; memcpy(&r, &f, sizeof(r)); return r;}
inline long double to_f(float128_t f){long double r; memcpy(&r, &f, sizeof(r)); return r;}

// Vector macros
#define e8 8      // 8b elements
#define e16 16    // 16b elements
#define e32 32    // 32b elements
#define e64 64    // 64b elements
#define e128 128  // 128b elements

#define vsext(x, sew) (((sreg_t)(x) << (64-sew)) >> (64-sew))
#define vzext(x, sew) (((reg_t)(x) << (64-sew)) >> (64-sew))

#define DEBUG_RVV 0

#if DEBUG_RVV
#define DEBUG_RVV_FP_VV \
  printf("vfp(%lu) vd=%f vs1=%f vs2=%f\n", i, to_f(vd), to_f(vs1), to_f(vs2));
#define DEBUG_RVV_FP_VF \
  printf("vfp(%lu) vd=%f vs1=%f vs2=%f\n", i, to_f(vd), to_f(rs1), to_f(vs2));
#define DEBUG_RVV_FMA_VV \
  printf("vfma(%lu) vd=%f vs1=%f vs2=%f vd_old=%f\n", i, to_f(vd), to_f(vs1), to_f(vs2), to_f(vd_old));
#define DEBUG_RVV_FMA_VF \
  printf("vfma(%lu) vd=%f vs1=%f vs2=%f vd_old=%f\n", i, to_f(vd), to_f(rs1), to_f(vs2), to_f(vd_old));
#else
#define DEBUG_RVV_FP_VV 0
#define DEBUG_RVV_FP_VF 0
#define DEBUG_RVV_FMA_VV 0
#define DEBUG_RVV_FMA_VF 0
#endif

//
// vector: masking skip helper
//
#define VI_MASK_VARS \
  const int mlen = P_.VU.vmlen; \
  const int midx = (mlen * i) / 64; \
  const int mpos = (mlen * i) % 64; \

#define VI_LOOP_ELEMENT_SKIP(BODY) \
  VI_MASK_VARS \
  if (insn.v_vm() == 0) { \
    BODY; \
    bool skip = ((P_.VU.elt<uint64_t>(0, midx, VREAD) >> mpos) & 0x1) == 0; \
    if (skip) \
    {\
      continue; \
    }\
  } \

#define VI_ELEMENT_SKIP(inx) \
  if (inx >= vl) { \
    continue; \
  } else if (inx < P_.VU.vstart) { \
    continue; \
  } else { \
    VI_LOOP_ELEMENT_SKIP(); \
  }

//
// vector: operation and register acccess check helper
//
static inline bool is_overlapped(const int astart, const int asize,
                                const int bstart, const int bsize)
{
  const int aend = astart + asize;
  const int bend = bstart + bsize;
  return std::max(aend, bend) - std::min(astart, bstart) < asize + bsize;
}

#define VI_NARROW_CHECK_COMMON \
  require_vector;\
  require(P_.VU.vlmul <= 4); \
  require(P_.VU.vsew * 2 <= P_.VU.ELEN); \
  require((insn.rs2() & (P_.VU.vlmul * 2 - 1)) == 0); \
  require((insn.rd() & (P_.VU.vlmul - 1)) == 0); \
  if (insn.v_vm() == 0 && P_.VU.vlmul > 1) \
    require(insn.rd() != 0);

#define VI_WIDE_CHECK_COMMON \
  require_vector;\
  require(P_.VU.vlmul <= 4); \
  require(P_.VU.vsew * 2 <= P_.VU.ELEN); \
  require((insn.rd() & (P_.VU.vlmul * 2 - 1)) == 0); \
  if (insn.v_vm() == 0) \
    require(insn.rd() != 0);

#define VI_CHECK_LDST_INDEX \
  require_vector; \
  require((insn.rd() & (P_.VU.vlmul - 1)) == 0); \
  require((insn.rs2() & (P_.VU.vlmul - 1)) == 0); \
  if (insn.v_nf() > 0) \
    require(!is_overlapped(insn.rd(), P_.VU.vlmul, insn.rs2(), P_.VU.vlmul)); \
  if (insn.v_vm() == 0 && (insn.v_nf() > 0 || P_.VU.vlmul > 1)) \
    require(insn.rd() != 0); \

#define VI_CHECK_MSS(is_vs1) \
  if (P_.VU.vlmul > 1) { \
    require(!is_overlapped(insn.rd(), 1, insn.rs2(), P_.VU.vlmul)); \
    require((insn.rs2() & (P_.VU.vlmul - 1)) == 0); \
    if (is_vs1) {\
      require(!is_overlapped(insn.rd(), 1, insn.rs1(), P_.VU.vlmul)); \
      require((insn.rs1() & (P_.VU.vlmul - 1)) == 0); \
    } \
  }

#define VI_CHECK_SSS(is_vs1) \
  if (P_.VU.vlmul > 1) { \
    require((insn.rd() & (P_.VU.vlmul - 1)) == 0); \
    require((insn.rs2() & (P_.VU.vlmul - 1)) == 0); \
    if (is_vs1) { \
      require((insn.rs1() & (P_.VU.vlmul - 1)) == 0); \
    } \
    if (insn.v_vm() == 0) \
      require(insn.rd() != 0); \
  }

#define VI_CHECK_SXX \
  require_vector; \
  if (P_.VU.vlmul > 1) { \
    require((insn.rd() & (P_.VU.vlmul - 1)) == 0); \
    if (insn.v_vm() == 0) \
      require(insn.rd() != 0); \
  }

#define VI_CHECK_DSS(is_vs1) \
  VI_WIDE_CHECK_COMMON; \
  require(!is_overlapped(insn.rd(), P_.VU.vlmul * 2, insn.rs2(), P_.VU.vlmul)); \
  require((insn.rd() & (P_.VU.vlmul * 2 - 1)) == 0); \
  require((insn.rs2() & (P_.VU.vlmul - 1)) == 0); \
  if (is_vs1) {\
     require(!is_overlapped(insn.rd(), P_.VU.vlmul * 2, insn.rs1(), P_.VU.vlmul)); \
     require((insn.rs1() & (P_.VU.vlmul - 1)) == 0); \
  }

#define VI_CHECK_QSS(is_vs1) \
  require_vector;\
  require(P_.VU.vlmul <= 2); \
  require(P_.VU.vsew * 4 <= P_.VU.ELEN); \
  require((insn.rd() & (P_.VU.vlmul * 4 - 1)) == 0); \
  if (insn.v_vm() == 0) \
    require(insn.rd() != 0); \
  require(!is_overlapped(insn.rd(), P_.VU.vlmul * 4, insn.rs2(), P_.VU.vlmul)); \
  require((insn.rs2() & (P_.VU.vlmul - 1)) == 0); \
  if (is_vs1) {\
     require(!is_overlapped(insn.rd(), P_.VU.vlmul * 4, insn.rs1(), P_.VU.vlmul)); \
     require((insn.rs1() & (P_.VU.vlmul - 1)) == 0); \
  }

#define VI_CHECK_DDS(is_rs) \
  VI_WIDE_CHECK_COMMON; \
  require((insn.rs2() & (P_.VU.vlmul * 2 - 1)) == 0); \
  if (is_rs) { \
     require(!is_overlapped(insn.rd(), P_.VU.vlmul * 2, insn.rs1(), P_.VU.vlmul)); \
     require((insn.rs1() & (P_.VU.vlmul - 1)) == 0); \
  }

#define VI_CHECK_SDS(is_vs1) \
  VI_NARROW_CHECK_COMMON; \
  require(!is_overlapped(insn.rd(), P_.VU.vlmul, insn.rs2(), P_.VU.vlmul * 2)); \
  if (is_vs1) \
    require((insn.rs1() & (P_.VU.vlmul - 1)) == 0); \

#define VI_CHECK_REDUCTION(is_wide) \
  require_vector;\
  if (is_wide) {\
    require(P_.VU.vlmul <= 4); \
    require(P_.VU.vsew * 2 <= P_.VU.ELEN); \
  } \
  require((insn.rs2() & (P_.VU.vlmul - 1)) == 0); \

//
// vector: loop header and end helper
//
#define VI_GENERAL_LOOP_BASE \
  require(P_.VU.vsew == e8 || P_.VU.vsew == e16 || P_.VU.vsew == e32 || P_.VU.vsew == e64); \
  require_vector;\
  reg_t vl = P_.VU.vl; \
  reg_t sew = P_.VU.vsew; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  for (reg_t i=P_.VU.vstart; i<vl; ++i){ 

#define VI_LOOP_BASE \
    VI_GENERAL_LOOP_BASE \
    VI_LOOP_ELEMENT_SKIP();

#define VI_LOOP_END \
  } \
  P_.VU.vstart = 0;

#define VI_LOOP_REDUCTION_END(x) \
  } \
  if (vl > 0) { \
    vd_0_des = vd_0_res; \
  } \
  P_.VU.vstart = 0; 

#define VI_LOOP_CMP_BASE \
  require(P_.VU.vsew == e8 || P_.VU.vsew == e16 || P_.VU.vsew == e32 || P_.VU.vsew == e64); \
  require_vector;\
  reg_t vl = P_.VU.vl; \
  reg_t sew = P_.VU.vsew; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  for (reg_t i=P_.VU.vstart; i<vl; ++i){ \
    VI_LOOP_ELEMENT_SKIP(); \
    uint64_t mmask = (UINT64_MAX << (64 - mlen)) >> (64 - mlen - mpos); \
    uint64_t &vdi = P_.VU.elt<uint64_t>(insn.rd(), midx, VREADWRITE); \
    uint64_t res = 0;

#define VI_LOOP_CMP_END \
    vdi = (vdi & ~mmask) | (((res) << mpos) & mmask); \
  } \
  P_.VU.vstart = 0;

#define VI_LOOP_MASK(op) \
  require(P_.VU.vsew <= e64); \
  reg_t vl = P_.VU.vl; \
  for (reg_t i = P_.VU.vstart; i < vl; ++i) { \
    int mlen = P_.VU.vmlen; \
    int midx = (mlen * i) / 64; \
    int mpos = (mlen * i) % 64; \
    uint64_t mmask = (UINT64_MAX << (64 - mlen)) >> (64 - mlen - mpos); \
    uint64_t vs2 = P_.VU.elt<uint64_t>(insn.rs2(), midx, VREAD); \
    uint64_t vs1 = P_.VU.elt<uint64_t>(insn.rs1(), midx, VREAD); \
    uint64_t &res = P_.VU.elt<uint64_t>(insn.rd(), midx, VREADWRITE); \
    res = (res & ~mmask) | ((op) & (1ULL << mpos)); \
  } \
  P_.VU.vstart = 0;

#define VI_LOOP_NSHIFT_BASE \
  VI_GENERAL_LOOP_BASE; \
  VI_LOOP_ELEMENT_SKIP({\
    require(!(insn.rd() == 0 && P_.VU.vlmul > 1));\
  });


#define INT_ROUNDING(result, xrm, gb) \
  do { \
    const uint64_t lsb = 1UL << (gb); \
    const uint64_t lsb_half = lsb >> 1; \
    switch (xrm) {\
      case VRM::RNU:\
        result += lsb_half; \
        break;\
      case VRM::RNE:\
        if ((result & lsb_half) && ((result & (lsb_half - 1)) || (result & lsb))) \
          result += lsb; \
        break;\
      case VRM::RDN:\
        break;\
      case VRM::ROD:\
        if (result & (lsb - 1)) \
          result |= lsb; \
        break;\
      case VRM::INVALID_RM:\
        assert(true);\
    } \
  } while (0)

//
// vector: integer and masking operand access helper
//
#define VXI_PARAMS(x) \
  type_sew_t<x>::type &vd = P_.VU.elt<type_sew_t<x>::type>(rd_num, i, VREADWRITE); \
  type_sew_t<x>::type vs1 = P_.VU.elt<type_sew_t<x>::type>(rs1_num, i, VREAD); \
  type_sew_t<x>::type vs2 = P_.VU.elt<type_sew_t<x>::type>(rs2_num, i, VREAD); \
  type_sew_t<x>::type rs1 = (type_sew_t<x>::type)RS1; \
  type_sew_t<x>::type simm5 = (type_sew_t<x>::type)insn.v_simm5();

#define VV_U_PARAMS(x) \
  type_usew_t<x>::type &vd = P_.VU.elt<type_usew_t<x>::type>(rd_num, i, VREADWRITE); \
  type_usew_t<x>::type vs1 = P_.VU.elt<type_usew_t<x>::type>(rs1_num, i, VREAD); \
  type_usew_t<x>::type vs2 = P_.VU.elt<type_usew_t<x>::type>(rs2_num, i, VREAD);

#define VX_U_PARAMS(x) \
  type_usew_t<x>::type &vd = P_.VU.elt<type_usew_t<x>::type>(rd_num, i, VREADWRITE); \
  type_usew_t<x>::type rs1 = (type_usew_t<x>::type)RS1; \
  type_usew_t<x>::type vs2 = P_.VU.elt<type_usew_t<x>::type>(rs2_num, i, VREAD);

#define VI_U_PARAMS(x) \
  type_usew_t<x>::type &vd = P_.VU.elt<type_usew_t<x>::type>(rd_num, i, VREADWRITE); \
  type_usew_t<x>::type simm5 = (type_usew_t<x>::type)insn.v_zimm5(); \
  type_usew_t<x>::type vs2 = P_.VU.elt<type_usew_t<x>::type>(rs2_num, i, VREAD);

#define VV_PARAMS(x) \
  type_sew_t<x>::type &vd = P_.VU.elt<type_sew_t<x>::type>(rd_num, i, VREADWRITE); \
  type_sew_t<x>::type vs1 = P_.VU.elt<type_sew_t<x>::type>(rs1_num, i, VREAD); \
  type_sew_t<x>::type vs2 = P_.VU.elt<type_sew_t<x>::type>(rs2_num, i, VREAD);

#define VX_PARAMS(x) \
  type_sew_t<x>::type &vd = P_.VU.elt<type_sew_t<x>::type>(rd_num, i, VREADWRITE); \
  type_sew_t<x>::type rs1 = (type_sew_t<x>::type)RS1; \
  type_sew_t<x>::type vs2 = P_.VU.elt<type_sew_t<x>::type>(rs2_num, i, VREAD);

#define VI_PARAMS(x) \
  type_sew_t<x>::type &vd = P_.VU.elt<type_sew_t<x>::type>(rd_num, i, VREADWRITE); \
  type_sew_t<x>::type simm5 = (type_sew_t<x>::type)insn.v_simm5(); \
  type_sew_t<x>::type vs2 = P_.VU.elt<type_sew_t<x>::type>(rs2_num, i, VREAD);

#define XV_PARAMS(x) \
  type_sew_t<x>::type &vd = P_.VU.elt<type_sew_t<x>::type>(rd_num, i, VREADWRITE); \
  type_usew_t<x>::type vs2 = P_.VU.elt<type_usew_t<x>::type>(rs2_num, RS1, VREAD);

#define VI_XI_SLIDEDOWN_PARAMS(x, off) \
  auto &vd = P_.VU.elt<type_sew_t<x>::type>(rd_num, i, VREADWRITE); \
  auto vs2 = P_.VU.elt<type_sew_t<x>::type>(rs2_num, i + off, VREAD);

#define VI_XI_SLIDEUP_PARAMS(x, offset) \
  auto &vd = P_.VU.elt<type_sew_t<x>::type>(rd_num, i, VREADWRITE); \
  auto vs2 = P_.VU.elt<type_sew_t<x>::type>(rs2_num, i - offset, VREAD);

#define VI_NSHIFT_PARAMS(sew1, sew2) \
  auto &vd = P_.VU.elt<type_usew_t<sew1>::type>(rd_num, i, VREADWRITE); \
  auto vs2_u = P_.VU.elt<type_usew_t<sew2>::type>(rs2_num, i, VREAD); \
  auto vs2 = P_.VU.elt<type_sew_t<sew2>::type>(rs2_num, i, VREAD); \
  auto zimm5 = (type_usew_t<sew1>::type)insn.v_zimm5();

#define VX_NSHIFT_PARAMS(sew1, sew2) \
  auto &vd = P_.VU.elt<type_usew_t<sew1>::type>(rd_num, i, VREADWRITE); \
  auto vs2_u = P_.VU.elt<type_usew_t<sew2>::type>(rs2_num, i, VREAD); \
  auto vs2 = P_.VU.elt<type_sew_t<sew2>::type>(rs2_num, i, VREAD); \
  auto rs1 = (type_sew_t<sew1>::type)RS1;

#define VV_NSHIFT_PARAMS(sew1, sew2) \
  auto &vd = P_.VU.elt<type_usew_t<sew1>::type>(rd_num, i, VREADWRITE); \
  auto vs2_u = P_.VU.elt<type_usew_t<sew2>::type>(rs2_num, i, VREAD); \
  auto vs2 = P_.VU.elt<type_sew_t<sew2>::type>(rs2_num, i, VREAD); \
  auto vs1 = P_.VU.elt<type_sew_t<sew1>::type>(rs1_num, i, VREAD);

#define XI_CARRY_PARAMS(x) \
  auto vs2 = P_.VU.elt<type_sew_t<x>::type>(rs2_num, i, VREAD); \
  auto rs1 = (type_sew_t<x>::type)RS1; \
  auto simm5 = (type_sew_t<x>::type)insn.v_simm5(); \
  auto &vd = P_.VU.elt<uint64_t>(rd_num, midx, VREADWRITE);

#define VV_CARRY_PARAMS(x) \
  auto vs2 = P_.VU.elt<type_sew_t<x>::type>(rs2_num, i, VREAD); \
  auto vs1 = P_.VU.elt<type_sew_t<x>::type>(rs1_num, i, VREAD); \
  auto &vd = P_.VU.elt<uint64_t>(rd_num, midx, VREADWRITE);

#define XI_WITH_CARRY_PARAMS(x) \
  auto vs2 = P_.VU.elt<type_sew_t<x>::type>(rs2_num, i, VREAD); \
  auto rs1 = (type_sew_t<x>::type)RS1; \
  auto simm5 = (type_sew_t<x>::type)insn.v_simm5(); \
  auto &vd = P_.VU.elt<type_sew_t<x>::type>(rd_num, i, VREADWRITE);

#define VV_WITH_CARRY_PARAMS(x) \
  auto vs2 = P_.VU.elt<type_sew_t<x>::type>(rs2_num, i, VREAD); \
  auto vs1 = P_.VU.elt<type_sew_t<x>::type>(rs1_num, i, VREAD); \
  auto &vd = P_.VU.elt<type_sew_t<x>::type>(rd_num, i, VREADWRITE);

//
// vector: integer and masking operation loop
//

// comparison result to masking register
#define VI_VV_LOOP_CMP(BODY) \
  VI_CHECK_MSS(true); \
  VI_LOOP_CMP_BASE \
  if (sew == e8){ \
    VV_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VV_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VV_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VV_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_CMP_END

#define VI_VX_LOOP_CMP(BODY) \
  VI_CHECK_MSS(false); \
  VI_LOOP_CMP_BASE \
  if (sew == e8){ \
    VX_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VX_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VX_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VX_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_CMP_END

#define VI_VI_LOOP_CMP(BODY) \
  VI_CHECK_MSS(false); \
  VI_LOOP_CMP_BASE \
  if (sew == e8){ \
    VI_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VI_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VI_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VI_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_CMP_END

#define VI_VV_ULOOP_CMP(BODY) \
  VI_CHECK_MSS(true); \
  VI_LOOP_CMP_BASE \
  if (sew == e8){ \
    VV_U_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VV_U_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VV_U_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VV_U_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_CMP_END

#define VI_VX_ULOOP_CMP(BODY) \
  VI_CHECK_MSS(false); \
  VI_LOOP_CMP_BASE \
  if (sew == e8){ \
    VX_U_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VX_U_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VX_U_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VX_U_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_CMP_END

#define VI_VI_ULOOP_CMP(BODY) \
  VI_CHECK_MSS(false); \
  VI_LOOP_CMP_BASE \
  if (sew == e8){ \
    VI_U_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VI_U_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VI_U_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VI_U_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_CMP_END

// merge and copy loop
#define VI_VVXI_MERGE_LOOP(BODY) \
  VI_CHECK_SXX; \
  VI_GENERAL_LOOP_BASE \
  if (sew == e8){ \
    VXI_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VXI_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VXI_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VXI_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_END 

// reduction loop - signed
#define VI_LOOP_REDUCTION_BASE(x) \
  require(x == e8 || x == e16 || x == e32 || x == e64); \
  reg_t vl = P_.VU.vl; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  auto &vd_0_des = P_.VU.elt<type_sew_t<x>::type>(rd_num, 0, VREADWRITE); \
  auto vd_0_res = P_.VU.elt<type_sew_t<x>::type>(rs1_num, 0, VREAD); \
  for (reg_t i=P_.VU.vstart; i<vl; ++i){ \
    VI_LOOP_ELEMENT_SKIP(); \
    auto vs2 = P_.VU.elt<type_sew_t<x>::type>(rs2_num, i, VREAD); \

#define REDUCTION_LOOP(x, BODY) \
  VI_LOOP_REDUCTION_BASE(x) \
  BODY; \
  VI_LOOP_REDUCTION_END(x)

#define VI_VV_LOOP_REDUCTION(BODY) \
  VI_CHECK_REDUCTION(false); \
  reg_t sew = P_.VU.vsew; \
  if (sew == e8) { \
    REDUCTION_LOOP(e8, BODY) \
  } else if(sew == e16) { \
    REDUCTION_LOOP(e16, BODY) \
  } else if(sew == e32) { \
    REDUCTION_LOOP(e32, BODY) \
  } else if(sew == e64) { \
    REDUCTION_LOOP(e64, BODY) \
  }

// reduction loop - unsigned
#define VI_ULOOP_REDUCTION_BASE(x) \
  require(x == e8 || x == e16 || x == e32 || x == e64); \
  reg_t vl = P_.VU.vl; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  auto &vd_0_des = P_.VU.elt<type_usew_t<x>::type>(rd_num, 0, VREADWRITE); \
  auto vd_0_res = P_.VU.elt<type_usew_t<x>::type>(rs1_num, 0, VREAD); \
  for (reg_t i=P_.VU.vstart; i<vl; ++i){ \
    VI_LOOP_ELEMENT_SKIP(); \
    auto vs2 = P_.VU.elt<type_usew_t<x>::type>(rs2_num, i, VREAD);

#define REDUCTION_ULOOP(x, BODY) \
  VI_ULOOP_REDUCTION_BASE(x) \
  BODY; \
  VI_LOOP_REDUCTION_END(x)

#define VI_VV_ULOOP_REDUCTION(BODY) \
  VI_CHECK_REDUCTION(false); \
  reg_t sew = P_.VU.vsew; \
  if (sew == e8){ \
    REDUCTION_ULOOP(e8, BODY) \
  } else if(sew == e16) { \
    REDUCTION_ULOOP(e16, BODY) \
  } else if(sew == e32) { \
    REDUCTION_ULOOP(e32, BODY) \
  } else if(sew == e64) { \
    REDUCTION_ULOOP(e64, BODY) \
  }

// general VXI signed/unsigned loop
#define VI_VV_ULOOP(BODY) \
  VI_CHECK_SSS(true) \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VV_U_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VV_U_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VV_U_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VV_U_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_END 

#define VI_VV_LOOP(BODY) \
  VI_CHECK_SSS(true) \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VV_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VV_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VV_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VV_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_END 

#define VI_VX_ULOOP(BODY) \
  VI_CHECK_SSS(false) \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VX_U_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VX_U_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VX_U_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VX_U_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_END 

#define VI_VX_LOOP(BODY) \
  VI_CHECK_SSS(false) \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VX_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VX_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VX_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VX_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_END 

#define VI_VI_ULOOP(BODY) \
  VI_CHECK_SSS(false) \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VI_U_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VI_U_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VI_U_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VI_U_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_END 

#define VI_VI_LOOP(BODY) \
  VI_CHECK_SSS(false) \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VI_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VI_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VI_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VI_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_END 

// narrow operation loop
#define VI_VV_LOOP_NARROW(BODY) \
VI_NARROW_CHECK_COMMON; \
VI_LOOP_BASE \
if (sew == e8){ \
  VI_NARROW_SHIFT(e8, e16) \
  BODY; \
}else if(sew == e16){ \
  VI_NARROW_SHIFT(e16, e32) \
  BODY; \
}else if(sew == e32){ \
  VI_NARROW_SHIFT(e32, e64) \
  BODY; \
} \
VI_LOOP_END 

#define VI_NARROW_SHIFT(sew1, sew2) \
  type_usew_t<sew1>::type &vd = P_.VU.elt<type_usew_t<sew1>::type>(rd_num, i, VREADWRITE); \
  type_usew_t<sew2>::type vs2_u = P_.VU.elt<type_usew_t<sew2>::type>(rs2_num, i, VREAD); \
  type_usew_t<sew1>::type zimm5 = (type_usew_t<sew1>::type)insn.v_zimm5(); \
  type_sew_t<sew2>::type vs2 = P_.VU.elt<type_sew_t<sew2>::type>(rs2_num, i, VREAD); \
  type_sew_t<sew1>::type vs1 = P_.VU.elt<type_sew_t<sew1>::type>(rs1_num, i, VREAD); \
  type_sew_t<sew1>::type rs1 = (type_sew_t<sew1>::type)RS1; 

#define VI_VVXI_LOOP_NARROW(BODY, is_vs1) \
  VI_CHECK_SDS(is_vs1); \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VI_NARROW_SHIFT(e8, e16) \
    BODY; \
  } else if (sew == e16) { \
    VI_NARROW_SHIFT(e16, e32) \
    BODY; \
  } else if (sew == e32) { \
    VI_NARROW_SHIFT(e32, e64) \
    BODY; \
  } \
  VI_LOOP_END

#define VI_VI_LOOP_NSHIFT(BODY, is_vs1) \
  VI_CHECK_SDS(is_vs1); \
  VI_LOOP_NSHIFT_BASE \
  if (sew == e8){ \
    VI_NSHIFT_PARAMS(e8, e16) \
    BODY; \
  } else if (sew == e16) { \
    VI_NSHIFT_PARAMS(e16, e32) \
    BODY; \
  } else if (sew == e32) { \
    VI_NSHIFT_PARAMS(e32, e64) \
    BODY; \
  } \
  VI_LOOP_END

#define VI_VX_LOOP_NSHIFT(BODY, is_vs1) \
  VI_CHECK_SDS(is_vs1); \
  VI_LOOP_NSHIFT_BASE \
  if (sew == e8){ \
    VX_NSHIFT_PARAMS(e8, e16) \
    BODY; \
  } else if (sew == e16) { \
    VX_NSHIFT_PARAMS(e16, e32) \
    BODY; \
  } else if (sew == e32) { \
    VX_NSHIFT_PARAMS(e32, e64) \
    BODY; \
  } \
  VI_LOOP_END

#define VI_VV_LOOP_NSHIFT(BODY, is_vs1) \
  VI_CHECK_SDS(is_vs1); \
  VI_LOOP_NSHIFT_BASE \
  if (sew == e8){ \
    VV_NSHIFT_PARAMS(e8, e16) \
    BODY; \
  } else if (sew == e16) { \
    VV_NSHIFT_PARAMS(e16, e32) \
    BODY; \
  } else if (sew == e32) { \
    VV_NSHIFT_PARAMS(e32, e64) \
    BODY; \
  } \
  VI_LOOP_END

// widen operation loop
#define VI_VV_LOOP_WIDEN(BODY) \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VV_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VV_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VV_PARAMS(e32); \
    BODY; \
  } \
  VI_LOOP_END

#define VI_VX_LOOP_WIDEN(BODY) \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VX_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VX_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VX_PARAMS(e32); \
    BODY; \
  } \
  VI_LOOP_END

#define VI_WIDE_OP_AND_ASSIGN(var0, var1, var2, op0, op1, sign) \
  switch(P_.VU.vsew) { \
  case e8: { \
    sign##16_t vd_w = P_.VU.elt<sign##16_t>(rd_num, i, VREAD); \
    P_.VU.elt<uint16_t>(rd_num, i, VWRITE) = \
      op1((sign##16_t)(sign##8_t)var0 op0 (sign##16_t)(sign##8_t)var1) + var2; \
    } \
    break; \
  case e16: { \
    sign##32_t vd_w = P_.VU.elt<sign##32_t>(rd_num, i, VREAD); \
    P_.VU.elt<uint32_t>(rd_num, i, VWRITE) = \
      op1((sign##32_t)(sign##16_t)var0 op0 (sign##32_t)(sign##16_t)var1) + var2; \
    } \
    break; \
  default: { \
    sign##64_t vd_w = P_.VU.elt<sign##64_t>(rd_num, i, VREAD); \
    P_.VU.elt<uint64_t>(rd_num, i, VWRITE) = \
      op1((sign##64_t)(sign##32_t)var0 op0 (sign##64_t)(sign##32_t)var1) + var2; \
    } \
    break; \
  }

#define VI_WIDE_OP_AND_ASSIGN_MIX(var0, var1, var2, op0, op1, sign_d, sign_1, sign_2) \
  switch(P_.VU.vsew) { \
  case e8: { \
    sign_d##16_t vd_w = P_.VU.elt<sign_d##16_t>(rd_num, i, VREAD); \
    P_.VU.elt<uint16_t>(rd_num, i, VWRITE) = \
      op1((sign_1##16_t)(sign_1##8_t)var0 op0 (sign_2##16_t)(sign_2##8_t)var1) + var2; \
    } \
    break; \
  case e16: { \
    sign_d##32_t vd_w = P_.VU.elt<sign_d##32_t>(rd_num, i, VREAD); \
    P_.VU.elt<uint32_t>(rd_num, i, VWRITE) = \
      op1((sign_1##32_t)(sign_1##16_t)var0 op0 (sign_2##32_t)(sign_2##16_t)var1) + var2; \
    } \
    break; \
  default: { \
    sign_d##64_t vd_w = P_.VU.elt<sign_d##64_t>(rd_num, i, VREAD); \
    P_.VU.elt<uint64_t>(rd_num, i, VWRITE) = \
      op1((sign_1##64_t)(sign_1##32_t)var0 op0 (sign_2##64_t)(sign_2##32_t)var1) + var2; \
    } \
    break; \
  }

#define VI_WIDE_WVX_OP(var0, op0, sign) \
  switch(P_.VU.vsew) { \
  case e8: { \
    sign##16_t &vd_w = P_.VU.elt<sign##16_t>(rd_num, i, VWRITE); \
    sign##16_t vs2_w = P_.VU.elt<sign##16_t>(rs2_num, i, VREAD); \
    vd_w = vs2_w op0 (sign##16_t)(sign##8_t)var0; \
    } \
    break; \
  case e16: { \
    sign##32_t &vd_w = P_.VU.elt<sign##32_t>(rd_num, i, VWRITE); \
    sign##32_t vs2_w = P_.VU.elt<sign##32_t>(rs2_num, i, VREAD); \
    vd_w = vs2_w op0 (sign##32_t)(sign##16_t)var0; \
    } \
    break; \
  default: { \
    sign##64_t &vd_w = P_.VU.elt<sign##64_t>(rd_num, i, VWRITE); \
    sign##64_t vs2_w = P_.VU.elt<sign##64_t>(rs2_num, i, VREAD); \
    vd_w = vs2_w op0 (sign##64_t)(sign##32_t)var0; \
    } \
    break; \
  }

// quad operation loop
#define VI_VV_LOOP_QUAD(BODY) \
  VI_CHECK_QSS(true); \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VV_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VV_PARAMS(e16); \
    BODY; \
  } \
  VI_LOOP_END

#define VI_VX_LOOP_QUAD(BODY) \
  VI_CHECK_QSS(false); \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VX_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VX_PARAMS(e16); \
    BODY; \
  } \
  VI_LOOP_END

#define VI_QUAD_OP_AND_ASSIGN(var0, var1, var2, op0, op1, sign) \
  switch(P_.VU.vsew) { \
  case e8: { \
    sign##32_t vd_w = P_.VU.elt<sign##32_t>(rd_num, i, VREAD); \
    P_.VU.elt<uint32_t>(rd_num, i, VWRITE) = \
      op1((sign##32_t)(sign##8_t)var0 op0 (sign##32_t)(sign##8_t)var1) + var2; \
    } \
    break; \
  default: { \
    sign##64_t vd_w = P_.VU.elt<sign##64_t>(rd_num, i, VREAD); \
    P_.VU.elt<uint64_t>(rd_num, i, VWRITE) = \
      op1((sign##64_t)(sign##16_t)var0 op0 (sign##64_t)(sign##16_t)var1) + var2; \
    } \
    break; \
  }

#define VI_QUAD_OP_AND_ASSIGN_MIX(var0, var1, var2, op0, op1, sign_d, sign_1, sign_2) \
  switch(P_.VU.vsew) { \
  case e8: { \
    sign_d##32_t vd_w = P_.VU.elt<sign_d##32_t>(rd_num, i, VREAD); \
    P_.VU.elt<uint32_t>(rd_num, i, VWRITE) = \
      op1((sign_1##32_t)(sign_1##8_t)var0 op0 (sign_2##32_t)(sign_2##8_t)var1) + var2; \
    } \
    break; \
  default: { \
    sign_d##64_t vd_w = P_.VU.elt<sign_d##64_t>(rd_num, i, VREAD); \
    P_.VU.elt<uint64_t>(rd_num, i, VWRITE) = \
      op1((sign_1##64_t)(sign_1##16_t)var0 op0 (sign_2##64_t)(sign_2##16_t)var1) + var2; \
    } \
    break; \
  }

// wide reduction loop - signed
#define VI_LOOP_WIDE_REDUCTION_BASE(sew1, sew2) \
  reg_t vl = P_.VU.vl; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  auto &vd_0_des = P_.VU.elt<type_sew_t<sew2>::type>(rd_num, 0, VREADWRITE); \
  auto vd_0_res = P_.VU.elt<type_sew_t<sew2>::type>(rs1_num, 0, VREAD); \
  for (reg_t i=P_.VU.vstart; i<vl; ++i){ \
    VI_LOOP_ELEMENT_SKIP(); \
    auto vs2 = P_.VU.elt<type_sew_t<sew1>::type>(rs2_num, i, VREAD);

#define WIDE_REDUCTION_LOOP(sew1, sew2, BODY) \
  VI_LOOP_WIDE_REDUCTION_BASE(sew1, sew2) \
  BODY; \
  VI_LOOP_REDUCTION_END(sew2)

#define VI_VV_LOOP_WIDE_REDUCTION(BODY) \
  VI_CHECK_REDUCTION(true); \
  reg_t sew = P_.VU.vsew; \
  if (sew == e8){ \
    WIDE_REDUCTION_LOOP(e8, e16, BODY) \
  } else if(sew == e16){ \
    WIDE_REDUCTION_LOOP(e16, e32, BODY) \
  } else if(sew == e32){ \
    WIDE_REDUCTION_LOOP(e32, e64, BODY) \
  }

// wide reduction loop - unsigned
#define VI_ULOOP_WIDE_REDUCTION_BASE(sew1, sew2) \
  reg_t vl = P_.VU.vl; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  auto &vd_0_des = P_.VU.elt<type_usew_t<sew2>::type>(rd_num, 0, VREADWRITE); \
  auto vd_0_res = P_.VU.elt<type_usew_t<sew2>::type>(rs1_num, 0, VREAD); \
  for (reg_t i=P_.VU.vstart; i<vl; ++i) { \
    VI_LOOP_ELEMENT_SKIP(); \
    auto vs2 = P_.VU.elt<type_usew_t<sew1>::type>(rs2_num, i, VREAD);

#define WIDE_REDUCTION_ULOOP(sew1, sew2, BODY) \
  VI_ULOOP_WIDE_REDUCTION_BASE(sew1, sew2) \
  BODY; \
  VI_LOOP_REDUCTION_END(sew2)

#define VI_VV_ULOOP_WIDE_REDUCTION(BODY) \
  VI_CHECK_REDUCTION(true); \
  reg_t sew = P_.VU.vsew; \
  if (sew == e8){ \
    WIDE_REDUCTION_ULOOP(e8, e16, BODY) \
  } else if(sew == e16){ \
    WIDE_REDUCTION_ULOOP(e16, e32, BODY) \
  } else if(sew == e32){ \
    WIDE_REDUCTION_ULOOP(e32, e64, BODY) \
  }

// carry/borrow bit loop
#define VI_VV_LOOP_CARRY(BODY) \
  VI_CHECK_MSS(true); \
  VI_GENERAL_LOOP_BASE \
  VI_MASK_VARS \
    if (sew == e8){ \
      VV_CARRY_PARAMS(e8) \
      BODY; \
    } else if (sew == e16) { \
      VV_CARRY_PARAMS(e16) \
      BODY; \
    } else if (sew == e32) { \
      VV_CARRY_PARAMS(e32) \
      BODY; \
    } else if (sew == e64) { \
      VV_CARRY_PARAMS(e64) \
      BODY; \
    } \
  VI_LOOP_END

#define VI_XI_LOOP_CARRY(BODY) \
  VI_CHECK_MSS(false); \
  VI_GENERAL_LOOP_BASE \
  VI_MASK_VARS \
    if (sew == e8){ \
      XI_CARRY_PARAMS(e8) \
      BODY; \
    } else if (sew == e16) { \
      XI_CARRY_PARAMS(e16) \
      BODY; \
    } else if (sew == e32) { \
      XI_CARRY_PARAMS(e32) \
      BODY; \
    } else if (sew == e64) { \
      XI_CARRY_PARAMS(e64) \
      BODY; \
    } \
  VI_LOOP_END

#define VI_VV_LOOP_WITH_CARRY(BODY) \
  require(insn.rd() != 0); \
  VI_CHECK_SSS(true); \
  VI_GENERAL_LOOP_BASE \
  VI_MASK_VARS \
    if (sew == e8){ \
      VV_WITH_CARRY_PARAMS(e8) \
      BODY; \
    } else if (sew == e16) { \
      VV_WITH_CARRY_PARAMS(e16) \
      BODY; \
    } else if (sew == e32) { \
      VV_WITH_CARRY_PARAMS(e32) \
      BODY; \
    } else if (sew == e64) { \
      VV_WITH_CARRY_PARAMS(e64) \
      BODY; \
    } \
  VI_LOOP_END

#define VI_XI_LOOP_WITH_CARRY(BODY) \
  require(insn.rd() != 0); \
  VI_CHECK_SSS(false); \
  VI_GENERAL_LOOP_BASE \
  VI_MASK_VARS \
    if (sew == e8){ \
      XI_WITH_CARRY_PARAMS(e8) \
      BODY; \
    } else if (sew == e16) { \
      XI_WITH_CARRY_PARAMS(e16) \
      BODY; \
    } else if (sew == e32) { \
      XI_WITH_CARRY_PARAMS(e32) \
      BODY; \
    } else if (sew == e64) { \
      XI_WITH_CARRY_PARAMS(e64) \
      BODY; \
    } \
  VI_LOOP_END

// average loop
#define VI_VVX_LOOP_AVG(opd, op, is_vs1) \
VI_CHECK_SSS(is_vs1); \
VRM xrm = p->VU.get_vround_mode(); \
VI_LOOP_BASE \
  switch(sew) { \
    case e8: { \
     VV_PARAMS(e8); \
     type_sew_t<e8>::type rs1 = RS1; \
     auto res = (int32_t)vs2 op opd; \
     INT_ROUNDING(res, xrm, 1); \
     vd = res >> 1; \
     break; \
    } \
    case e16: { \
     VV_PARAMS(e16); \
     type_sew_t<e16>::type rs1 = RS1; \
     auto res = (int32_t)vs2 op opd; \
     INT_ROUNDING(res, xrm, 1); \
     vd = res >> 1; \
     break; \
    } \
    case e32: { \
     VV_PARAMS(e32); \
     type_sew_t<e32>::type rs1 = RS1; \
     auto res = (int64_t)vs2 op opd; \
     INT_ROUNDING(res, xrm, 1); \
     vd = res >> 1; \
     break; \
    } \
    default: { \
     VV_PARAMS(e64); \
     type_sew_t<e64>::type rs1 = RS1; \
     auto res = (int128_t)vs2 op opd; \
     INT_ROUNDING(res, xrm, 1); \
     vd = res >> 1; \
     break; \
    } \
  } \
VI_LOOP_END

#define VI_VVX_ULOOP_AVG(opd, op, is_vs1) \
VI_CHECK_SSS(is_vs1); \
VRM xrm = p->VU.get_vround_mode(); \
VI_LOOP_BASE \
  switch(sew) { \
    case e8: { \
     VV_U_PARAMS(e8); \
     type_usew_t<e8>::type rs1 = RS1; \
     auto res = (uint16_t)vs2 op opd; \
     INT_ROUNDING(res, xrm, 1); \
     vd = res >> 1; \
     break; \
    } \
    case e16: { \
     VV_U_PARAMS(e16); \
     type_usew_t<e16>::type rs1 = RS1; \
     auto res = (uint32_t)vs2 op opd; \
     INT_ROUNDING(res, xrm, 1); \
     vd = res >> 1; \
     break; \
    } \
    case e32: { \
     VV_U_PARAMS(e32); \
     type_usew_t<e32>::type rs1 = RS1; \
     auto res = (uint64_t)vs2 op opd; \
     INT_ROUNDING(res, xrm, 1); \
     vd = res >> 1; \
     break; \
    } \
    default: { \
     VV_U_PARAMS(e64); \
     type_usew_t<e64>::type rs1 = RS1; \
     auto res = (uint128_t)vs2 op opd; \
     INT_ROUNDING(res, xrm, 1); \
     vd = res >> 1; \
     break; \
    } \
  } \
VI_LOOP_END

//
// vector: load/store helper 
//
#define VI_STRIP(inx) \
  reg_t elems_per_strip = P_.VU.get_slen()/P_.VU.vsew; \
  reg_t elems_per_vreg = P_.VU.get_vlen()/P_.VU.vsew; \
  reg_t elems_per_lane = P_.VU.vlmul * elems_per_strip; \
  reg_t strip_index = (inx) / elems_per_lane; \
  reg_t index_in_strip = (inx) % elems_per_strip; \
  int32_t lmul_inx = (int32_t)(((inx) % elems_per_lane) / elems_per_strip); \
  reg_t vreg_inx = lmul_inx * elems_per_vreg + strip_index * elems_per_strip + index_in_strip;


#define VI_DUPLICATE_VREG(v, vlmax) \
reg_t index[vlmax]; \
for (reg_t i = 0; i < vlmax; ++i) { \
  switch(P_.VU.vsew) { \
    case e8: \
      index[i] = P_.VU.elt<uint8_t>(v, i, VREAD); \
      break; \
    case e16: \
      index[i] = P_.VU.elt<uint16_t>(v, i, VREAD); \
      break; \
    case e32: \
      index[i] = P_.VU.elt<uint32_t>(v, i, VREAD); \
      break; \
    case e64: \
      index[i] = P_.VU.elt<uint64_t>(v, i, VREAD); \
      break; \
  } \
}

#define CHECK_NO_SCALAR_STORE_IN_FLIGHT \
  if(MMU.get_num_in_flight_scalar_stores()>0 && P_.vector_bypass_l1) \
  { \
      P_.log_vector_memory_wait_for_scalar(); \
      return STATE.pc; \
  }
  

#define CHECK_ENABLE_BYPASS \
  if(P_.vector_bypass_l1){ \
    MMU.enable_l1_bypass(); \
  } \
  if(P_.vector_bypass_l2){ \
    MMU.enable_l2_bypass(); \
  } \ 

#define CHECK_DISABLE_BYPASS \
  if(P_.vector_bypass_l1){ \
    MMU.disable_l1_bypass(); \
  } \
  if(P_.vector_bypass_l2){ \
    MMU.disable_l2_bypass(); \
  } 

#define VI_ST_COMMON(stride, offset, st_width, elt_byte) \
  P_.is_store = true; \
  P_.is_vector_memory = true; \
  const reg_t nf = insn.v_nf() + 1; \
  require((nf * P_.VU.vlmul) <= (NVPR / 4)); \
  const reg_t vl = P_.VU.vl; \
  const reg_t baseAddr = RS1; \
  const reg_t vs3 = insn.rd(); \
  require(vs3 + nf * P_.VU.vlmul <= NVPR); \
  const reg_t vlmul = P_.VU.vlmul; \
  if(P_.enable_smart_mcpu){ \
    MMU.enable_smart_mcpu(); \
  } \
  else{ \
    CHECK_ENABLE_BYPASS \
  } \
  for (reg_t i = 0; i < vl; ++i) { \
    VI_STRIP(i) \
    VI_ELEMENT_SKIP(i); \
    P_.VU.vstart = i; \
    for (reg_t fn = 0; fn < nf; ++fn) { \
      st_width##_t val = 0; \
      switch (P_.VU.vsew) { \
      case e8: \
        val = P_.VU.elt<uint8_t>(vs3 + fn * vlmul, vreg_inx, VREAD); \
        break; \
      case e16: \
        val = P_.VU.elt<uint16_t>(vs3 + fn * vlmul, vreg_inx, VREAD); \
        break; \
      case e32: \
        val = P_.VU.elt<uint32_t>(vs3 + fn * vlmul, vreg_inx, VREAD); \
        break; \
      default: \
        val = P_.VU.elt<uint64_t>(vs3 + fn * vlmul, vreg_inx, VREAD); \
        break; \
      } \
      MMU.store_##st_width(baseAddr + (stride) + (offset) * elt_byte, val); \
    } \
  } \
  if(P_.enable_smart_mcpu)\
  { \
    MMU.disable_smart_mcpu(); \
    P_.log_mcpu_instruction(baseAddr, sizeof(st_width##_t), true, insn.bits()); \
    /*
     Set the destination register also, because once the acknowledge is done, \
     we have to set the availability of this register. \
    */ \
    P_.curr_write_reg = vs3;\
    P_.curr_write_reg_type = coyote::Request::RegType::VECTOR;\
  } \
  else\
  { \
    CHECK_DISABLE_BYPASS \
  } \
  P_.VU.vstart = 0;

#define VI_LD_COMMON(stride, offset, ld_width, elt_byte) \
  CHECK_NO_SCALAR_STORE_IN_FLIGHT \
  P_.is_load = true; \
  P_.is_vector_memory = true; \
  const reg_t nf = insn.v_nf() + 1; \
  require((nf * P_.VU.vlmul) <= (NVPR / 4)); \
  const reg_t vl = P_.VU.vl; \
  const reg_t baseAddr = RS1; \
  const reg_t vd = insn.rd(); \
  require(vd + nf * P_.VU.vlmul <= NVPR); \
  const reg_t vlmul = P_.VU.vlmul; \
  if(P_.enable_smart_mcpu){ \
    MMU.enable_smart_mcpu(); \
  } \
  else{ \
    CHECK_ENABLE_BYPASS \
  } \
  for (reg_t i = 0; i < vl; ++i) { \
    VI_ELEMENT_SKIP(i); \
    VI_STRIP(i); \
    P_.VU.vstart = i; \
    for (reg_t fn = 0; fn < nf; ++fn) { \
      ld_width##_t val = MMU.load_##ld_width(baseAddr + (stride) + (offset) * elt_byte); \
      switch(P_.VU.vsew){ \
        case e8: \
          P_.VU.elt<uint8_t>(vd + fn * vlmul, vreg_inx, VWRITE) = val; \
          break; \
        case e16: \
          P_.VU.elt<uint16_t>(vd + fn * vlmul, vreg_inx, VWRITE) = val; \
          break; \
        case e32: \
          P_.VU.elt<uint32_t>(vd + fn * vlmul, vreg_inx, VWRITE) = val; \
          break; \
        default: \
          P_.VU.elt<uint64_t>(vd + fn * vlmul, vreg_inx, VWRITE) = val; \
      } \
    } \
  } \
  if(P_.enable_smart_mcpu)\
  { \
    MMU.disable_smart_mcpu(); \
    P_.log_mcpu_instruction(baseAddr, sizeof(ld_width##_t), false, insn.bits()); \
  } \
  else\
  { \
    CHECK_DISABLE_BYPASS \
  } \
  /*
   Set the destination register also, because once the acknowledge is done, \
   we have to set the availability of this register. \
  */ \
  P_.curr_write_reg = vd;\
  P_.curr_write_reg_type = coyote::Request::RegType::VECTOR;\
  P_.VU.vstart = 0;

#define VI_LD(stride, offset, ld_width, elt_byte) \
  VI_CHECK_SXX; \
  VI_LD_COMMON(stride, offset, ld_width, elt_byte) \
  LOG_STRIDE(stride)
  
#define CHECK_MEMTILE_ENABLE() \
  if(p->enable_smart_mcpu && !p->is_vl_available) { \
    p->get_state()->raw = true; \
    return true; \
  } \

#define LOG_STRIDE(stride) \
  if(P_.enable_smart_mcpu && vl>1){ \
    bool is_strided=false; \
    int i=0; \
    uint64_t s_0=(stride); \
    i=1; \
    uint64_t s_1=(stride); \
    if(s_0!=s_1) \
    { \
        GET_INDICES(stride); \
        P_.set_mcpu_instruction_strided(indices); \
    } \
  } \
    
#define VI_LD_INDEX(stride, offset, ld_width, elt_byte) \
  VI_CHECK_LDST_INDEX; \
  VI_LD_COMMON(stride, offset, ld_width, elt_byte) \
  LOG_INDEX(stride) 

#define LOG_INDEX(stride) \
  if(P_.enable_smart_mcpu){\
    GET_INDICES(stride); \
    P_.set_mcpu_instruction_indexed(indices); \
  }

#define GET_INDICES(stride) \
  std::vector<uint64_t> indices; \
  for (reg_t i = 0; i < vl; ++i) { \
    indices.push_back(stride); \
  }

#define VI_ST(stride, offset, st_width, elt_byte) \
  VI_CHECK_SXX; \
  VI_ST_COMMON(stride, offset, st_width, elt_byte) \
  LOG_STRIDE(stride)

#define VI_ST_INDEX(stride, offset, st_width, elt_byte) \
  VI_CHECK_LDST_INDEX; \
  VI_ST_COMMON(stride, offset, st_width, elt_byte) \
  LOG_INDEX(stride) 

#define VI_LDST_FF(itype, tsew) \
  P_.is_load = true; \
  P_.is_vector_memory = true; \
  require(p->VU.vsew >= e##tsew && p->VU.vsew <= e64); \
  const reg_t nf = insn.v_nf() + 1; \
  require((nf * P_.VU.vlmul) <= (NVPR / 4)); \
  VI_CHECK_SXX; \
  const reg_t sew = p->VU.vsew; \
  const reg_t vl = p->VU.vl; \
  const reg_t baseAddr = RS1; \
  const reg_t rd_num = insn.rd(); \
  bool early_stop = false; \
  const reg_t vlmul = P_.VU.vlmul; \
  require(rd_num + nf * P_.VU.vlmul <= NVPR); \
  p->VU.vstart = 0; \
  if(P_.enable_smart_mcpu){ \
    MMU.enable_smart_mcpu(); \
  } \
  else{ \
    CHECK_ENABLE_BYPASS \
  } \
  for (reg_t i = 0; i < vl; ++i) { \
    VI_STRIP(i); \
    VI_ELEMENT_SKIP(i); \
    \
    for (reg_t fn = 0; fn < nf; ++fn) { \
      itype##64_t val; \
      try { \
        val = MMU.load_##itype##tsew(baseAddr + (i * nf + fn) * (tsew / 8)); \
      } catch (trap_t& t) { \
        if (i == 0) \
          throw t; /* Only take exception on zeroth element */ \
        /* Reduce VL if an exception occurs on a later element */ \
        early_stop = true; \
        P_.VU.vl = i; \
        break; \
      } \
      \
      switch (sew) { \
      case e8: \
        p->VU.elt<uint8_t>(rd_num + fn * vlmul, vreg_inx, VWRITE) = val; \
        break; \
      case e16: \
        p->VU.elt<uint16_t>(rd_num + fn * vlmul, vreg_inx, VWRITE) = val; \
        break; \
      case e32: \
        p->VU.elt<uint32_t>(rd_num + fn * vlmul, vreg_inx, VWRITE) = val; \
        break; \
      case e64: \
        p->VU.elt<uint64_t>(rd_num + fn * vlmul, vreg_inx, VWRITE) = val; \
        break; \
      } \
    } \
    \
    if (early_stop) { \
      if(P_.enable_smart_mcpu)\
      { \
        MMU.disable_smart_mcpu(); \
        P_.log_mcpu_instruction(baseAddr, sizeof(itype##tsew##_t), false, insn.bits()); \
      } \
      else\
      { \
        CHECK_DISABLE_BYPASS \
      } \
      break; \
    } \
    if(P_.enable_smart_mcpu)\
    { \
      MMU.disable_smart_mcpu(); \
      P_.log_mcpu_instruction(baseAddr, sizeof(itype##tsew##_t), false, insn.bits()); \
    } \
    else\
    { \
      CHECK_DISABLE_BYPASS \
    } \
  } \
  /*
    Set the destination register also, because once the acknowledge is done, \
    we have to set the availability of this register. \
  */ \
  P_.curr_write_reg = rd_num;\
  P_.curr_write_reg_type = coyote::Request::RegType::VECTOR;

//
// vector: vfp helper
//
#define VI_VFP_COMMON \
  require_fp; \
  require((P_.VU.vsew == e32 && p->supports_extension('F')) || \
          (P_.VU.vsew == e64 && p->supports_extension('D'))); \
  require_vector;\
  reg_t vl = P_.VU.vl; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  softfloat_roundingMode = STATE.frm;

#define VI_VFP_LOOP_BASE \
  VI_VFP_COMMON \
  for (reg_t i=P_.VU.vstart; i<vl; ++i){ \
    VI_LOOP_ELEMENT_SKIP();

#define VI_VFP_LOOP_CMP_BASE \
  VI_VFP_COMMON \
  for (reg_t i = P_.VU.vstart; i < vl; ++i) { \
    float32_t vs2 = P_.VU.elt<float32_t>(rs2_num, i, VREAD); \
    float32_t vs1 = P_.VU.elt<float32_t>(rs1_num, i, VREAD); \
    float32_t rs1 = f32(READ_FREG(rs1_num)); \
    VI_LOOP_ELEMENT_SKIP(); \
    uint64_t mmask = (UINT64_MAX << (64 - mlen)) >> (64 - mlen - mpos); \
    uint64_t &vdi = P_.VU.elt<uint64_t>(rd_num, midx, VREADWRITE); \
    uint64_t res = 0;

#define VI_VFP_LOOP_REDUCTION_BASE(width) \
  float##width##_t vd_0 = P_.VU.elt<float##width##_t>(rd_num, 0, VREAD); \
  float##width##_t vs1_0 = P_.VU.elt<float##width##_t>(rs1_num, 0, VREAD); \
  vd_0 = vs1_0;\
  for (reg_t i=P_.VU.vstart; i<vl; ++i){ \
    VI_LOOP_ELEMENT_SKIP(); \
    int##width##_t &vd = P_.VU.elt<int##width##_t>(rd_num, i, VREADWRITE); \
    float##width##_t vs2 = P_.VU.elt<float##width##_t>(rs2_num, i, VREAD); \

#define VI_VFP_LOOP_WIDE_REDUCTION_BASE \
  VI_VFP_COMMON \
  float64_t vd_0 = f64(P_.VU.elt<float64_t>(rs1_num, 0, VREAD).v); \
  for (reg_t i=P_.VU.vstart; i<vl; ++i) { \
    VI_LOOP_ELEMENT_SKIP();

#define VI_VFP_LOOP_END \
  } \
  P_.VU.vstart = 0; \

#define VI_VFP_LOOP_WIDE_END \
  } \
  P_.VU.vstart = 0; \
  set_fp_exceptions;

#define VI_VFP_LOOP_REDUCTION_END(x) \
  } \
  P_.VU.vstart = 0; \
  if (vl > 0) { \
    P_.VU.elt<type_sew_t<x>::type>(rd_num, 0, VWRITE) = vd_0.v; \
  }

#define VI_VFP_LOOP_CMP_END \
  switch(P_.VU.vsew) { \
    case e32: \
    case e64: { \
      vdi = (vdi & ~mmask) | (((res) << mpos) & mmask); \
      break; \
    } \
    case e16: \
    default: \
      require(0); \
      break; \
    }; \
  } \
  P_.VU.vstart = 0; \
  set_fp_exceptions;

#define VI_VFP_VV_LOOP(BODY32, BODY64) \
  VI_CHECK_SSS(true); \
  VI_VFP_LOOP_BASE \
  switch(P_.VU.vsew) { \
    case e32: {\
      float32_t &vd = P_.VU.elt<float32_t>(rd_num, i, VREADWRITE); \
      float32_t vs1 = P_.VU.elt<float32_t>(rs1_num, i, VREAD); \
      float32_t vs2 = P_.VU.elt<float32_t>(rs2_num, i, VREAD); \
      BODY32; \
      set_fp_exceptions; \
      break; \
    }\
    case e64: {\
      float64_t &vd = P_.VU.elt<float64_t>(rd_num, i, VREADWRITE); \
      float64_t vs1 = P_.VU.elt<float64_t>(rs1_num, i, VREAD); \
      float64_t vs2 = P_.VU.elt<float64_t>(rs2_num, i, VREAD); \
      BODY64; \
      set_fp_exceptions; \
      break; \
    }\
    case e16: \
    default: \
      require(0); \
      break; \
  }; \
  DEBUG_RVV_FP_VV; \
  VI_VFP_LOOP_END

#define VI_VFP_VV_LOOP_REDUCTION(BODY32, BODY64) \
  VI_CHECK_REDUCTION(false) \
  VI_VFP_COMMON \
  switch(P_.VU.vsew) { \
    case e32: {\
      VI_VFP_LOOP_REDUCTION_BASE(32) \
        BODY32; \
        set_fp_exceptions; \
      VI_VFP_LOOP_REDUCTION_END(e32) \
      break; \
    }\
    case e64: {\
      VI_VFP_LOOP_REDUCTION_BASE(64) \
        BODY64; \
        set_fp_exceptions; \
      VI_VFP_LOOP_REDUCTION_END(e64) \
      break; \
    }\
    case e16: \
    default: \
      require(0); \
      break; \
  }; \

#define VI_VFP_VV_LOOP_WIDE_REDUCTION(BODY) \
  VI_VFP_LOOP_WIDE_REDUCTION_BASE \
  float64_t vs2 = f32_to_f64(P_.VU.elt<float32_t>(rs2_num, i, VREAD)); \
  BODY; \
  set_fp_exceptions; \
  DEBUG_RVV_FP_VV; \
  VI_VFP_LOOP_REDUCTION_END(e64)

#define VI_VFP_VF_LOOP(BODY32, BODY64) \
  VI_CHECK_SSS(false); \
  VI_VFP_LOOP_BASE \
  switch(P_.VU.vsew) { \
    case e32: {\
      float32_t &vd = P_.VU.elt<float32_t>(rd_num, i, VREADWRITE); \
      float32_t rs1 = f32(READ_FREG(rs1_num)); \
      float32_t vs2 = P_.VU.elt<float32_t>(rs2_num, i, VREAD); \
      BODY32; \
      set_fp_exceptions; \
      break; \
    }\
    case e64: {\
      float64_t &vd = P_.VU.elt<float64_t>(rd_num, i, VREADWRITE); \
      float64_t rs1 = f64(READ_FREG(rs1_num)); \
      float64_t vs2 = P_.VU.elt<float64_t>(rs2_num, i, VREAD); \
      BODY64; \
      set_fp_exceptions; \
      break; \
    }\
    case e16: \
    case e8: \
    default: \
      require(0); \
      break; \
  }; \
  DEBUG_RVV_FP_VF; \
  VI_VFP_LOOP_END

#define VI_VFP_LOOP_CMP(BODY32, BODY64, is_vs1) \
  VI_CHECK_MSS(is_vs1); \
  VI_VFP_LOOP_CMP_BASE \
  switch(P_.VU.vsew) { \
    case e32: {\
      float32_t vs2 = P_.VU.elt<float32_t>(rs2_num, i, VREAD); \
      float32_t vs1 = P_.VU.elt<float32_t>(rs1_num, i, VREAD); \
      float32_t rs1 = f32(READ_FREG(rs1_num)); \
      BODY32; \
      set_fp_exceptions; \
      break; \
    }\
    case e64: {\
      float64_t vs2 = P_.VU.elt<float64_t>(rs2_num, i, VREAD); \
      float64_t vs1 = P_.VU.elt<float64_t>(rs1_num, i, VREAD); \
      float64_t rs1 = f64(READ_FREG(rs1_num)); \
      BODY64; \
      set_fp_exceptions; \
      break; \
    }\
    case e16: \
    default: \
      require(0); \
      break; \
  }; \
  VI_VFP_LOOP_CMP_END \

#define VI_VFP_VF_LOOP_WIDE(BODY) \
  VI_CHECK_DSS(false); \
  VI_VFP_LOOP_BASE \
  switch(P_.VU.vsew) { \
    case e32: {\
      float64_t &vd = P_.VU.elt<float64_t>(rd_num, i, VREADWRITE); \
      float64_t vs2 = f32_to_f64(P_.VU.elt<float32_t>(rs2_num, i, VREAD)); \
      float64_t rs1 = f32_to_f64(f32(READ_FREG(rs1_num))); \
      BODY; \
      set_fp_exceptions; \
      break; \
    }\
    case e16: \
    case e8: \
    default: \
      require(0); \
      break; \
  }; \
  DEBUG_RVV_FP_VV; \
  VI_VFP_LOOP_WIDE_END


#define VI_VFP_VV_LOOP_WIDE(BODY) \
  VI_CHECK_DSS(true); \
  VI_VFP_LOOP_BASE \
  switch(P_.VU.vsew) { \
    case e32: {\
      float64_t &vd = P_.VU.elt<float64_t>(rd_num, i, VREADWRITE); \
      float64_t vs2 = f32_to_f64(P_.VU.elt<float32_t>(rs2_num, i, VREAD)); \
      float64_t vs1 = f32_to_f64(P_.VU.elt<float32_t>(rs1_num, i, VREAD)); \
      BODY; \
      set_fp_exceptions; \
      break; \
    }\
    case e16: \
    case e8: \
    default: \
      require(0); \
      break; \
  }; \
  DEBUG_RVV_FP_VV; \
  VI_VFP_LOOP_WIDE_END

#define VI_VFP_WF_LOOP_WIDE(BODY) \
  VI_CHECK_DDS(false); \
  VI_VFP_LOOP_BASE \
  switch(P_.VU.vsew) { \
    case e32: {\
      float64_t &vd = P_.VU.elt<float64_t>(rd_num, i, VREADWRITE); \
      float64_t vs2 = P_.VU.elt<float64_t>(rs2_num, i, VREAD); \
      float64_t rs1 = f32_to_f64(f32(READ_FREG(rs1_num))); \
      BODY; \
      set_fp_exceptions; \
      break; \
    }\
    case e16: \
    case e8: \
    default: \
      require(0); \
  }; \
  DEBUG_RVV_FP_VV; \
  VI_VFP_LOOP_WIDE_END

#define VI_VFP_WV_LOOP_WIDE(BODY) \
  VI_CHECK_DDS(true); \
  VI_VFP_LOOP_BASE \
  switch(P_.VU.vsew) { \
    case e32: {\
      float64_t &vd = P_.VU.elt<float64_t>(rd_num, i, VREADWRITE); \
      float64_t vs2 = P_.VU.elt<float64_t>(rs2_num, i, VREAD); \
      float64_t vs1 = f32_to_f64(P_.VU.elt<float32_t>(rs1_num, i, VREAD)); \
      BODY; \
      set_fp_exceptions; \
      break; \
    }\
    case e16: \
    case e8: \
    default: \
      require(0); \
  }; \
  DEBUG_RVV_FP_VV; \
  VI_VFP_LOOP_WIDE_END


// Seems that 0x0 doesn't work.
#define DEBUG_START             0x100
#define DEBUG_END               (0x1000 - 1)


// FPU macros
#define FRS1 READ_FREG(insn.rs1())
#define FRS2 READ_FREG(insn.rs2())
#define FRS3 READ_FREG(insn.rs3())

#define C_RD P_.XPR_CHECK_RAW(insn.rd())
#define C_RS1 P_.XPR_CHECK_RAW(insn.rs1())
#define C_RS2 P_.XPR_CHECK_RAW(insn.rs2())
#define C_RS3 P_.XPR_CHECK_RAW(insn.rs3())
#define C_RVC_RS1 P_.XPR_CHECK_RAW(insn.rvc_rs1())
#define C_RVC_RS2 P_.XPR_CHECK_RAW(insn.rvc_rs2())
#define C_RVC_RS1S P_.XPR_CHECK_RAW(insn.rvc_rs1s())
#define C_RVC_RS2S P_.XPR_CHECK_RAW(insn.rvc_rs2s())
#define C_RVC_FRS2 P_.FPR_CHECK_RAW(insn.rvc_rs2())
#define C_RVC_FRS2S P_.FPR_CHECK_RAW(insn.rvc_rs2s())
#define C_RVC_SP P_.XPR_CHECK_RAW(X_SP)
#define C_FRS1 P_.FPR_CHECK_RAW(insn.rs1())
#define C_FRS2 P_.FPR_CHECK_RAW(insn.rs2())
#define C_FRS3 P_.FPR_CHECK_RAW(insn.rs3())
#define C_RVC_SP P_.XPR_CHECK_RAW(X_SP)

#define CHECK_VPU_AVAIL \
  bool vpu_unavail = P_.VU.is_busy(P_.get_current_cycle()); \

#define SKIP_CHECK_RAW() \
   bool b6 = P_.VU.check_raw<uint64_t>(0, 0)
  
#define VECTOR_VECTOR_UNSIGNED_CHECK_RAW() \
  CHECK_VPU_AVAIL \
  reg_t sew = P_.VU.vsew; \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  SKIP_CHECK_RAW(); \
  bool b7 = false; \
  bool b8 = false; \
  switch(sew) { \
    case e8: { \
     b7 = P_.VU.check_raw<type_usew_t<e8>::type>(rs1_num, 0); \
     b8 = P_.VU.check_raw<type_usew_t<e8>::type>(rs2_num, 0); \
     break; \
    } \
    case e16: { \
     b7 = P_.VU.check_raw<type_usew_t<e16>::type>(rs1_num, 0); \
     b8 = P_.VU.check_raw<type_usew_t<e16>::type>(rs2_num, 0); \
     break; \
    } \
    case e32: { \
     b7 = P_.VU.check_raw<type_usew_t<e32>::type>(rs1_num, 0); \
     b8 = P_.VU.check_raw<type_usew_t<e32>::type>(rs2_num, 0); \
     break; \
    } \
    default: { \
     b7 = P_.VU.check_raw<type_usew_t<e64>::type>(rs1_num, 0); \
     b8 = P_.VU.check_raw<type_usew_t<e64>::type>(rs2_num, 0); \
     break; \
    } \
  } \
  return (vpu_unavail || b6 || b7 || b8);

#define SCALAR_VECTOR_UNSIGNED_CHECK_RAW() \
  CHECK_VPU_AVAIL \
  reg_t sew = P_.VU.vsew; \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  SKIP_CHECK_RAW(); \
  bool b7 = false; \
  bool b8 = false; \
  switch(sew) { \
    case e8: { \
     b7 = C_RS1; \
     b8 = P_.VU.check_raw<type_usew_t<e8>::type>(rs2_num, 0); \
     break; \
    } \
    case e16: { \
     b7 = C_RS1; \
     b8 = P_.VU.check_raw<type_usew_t<e16>::type>(rs2_num, 0); \
     break; \
    } \
    case e32: { \
     b7 = C_RS1; \
     b8 = P_.VU.check_raw<type_usew_t<e32>::type>(rs2_num, 0); \
     break; \
    } \
    default: { \
     b7 = C_RS1; \
     b8 = P_.VU.check_raw<type_usew_t<e64>::type>(rs2_num, 0); \
     break; \
    } \
  } \
  return (vpu_unavail || b6 || b7 || b8);

#define VECTOR_UNSIGNED_CHECK_RAW() \
  CHECK_VPU_AVAIL \
  reg_t sew = P_.VU.vsew; \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  SKIP_CHECK_RAW(); \
  bool b7 = false; \
  switch(sew) { \
    case e8: { \
     b7 = P_.VU.check_raw<type_usew_t<e8>::type>(rs2_num, 0); \
     break; \
    } \
    case e16: { \
     b7 = P_.VU.check_raw<type_usew_t<e16>::type>(rs2_num, 0); \
     break; \
    } \
    case e32: { \
     b7 = P_.VU.check_raw<type_usew_t<e32>::type>(rs2_num, 0); \
     break; \
    } \
    default: { \
     b7 = P_.VU.check_raw<type_usew_t<e64>::type>(rs2_num, 0); \
     break; \
    } \
  } \
  return (vpu_unavail || b6 || b7);

#define VECTOR_VECTOR_VECTOR_UNSIGNED_CHECK_RAW() \
  CHECK_VPU_AVAIL \
  reg_t sew = P_.VU.vsew; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  bool b7 = false; \
  bool b8 = false; \
  bool b9 = false; \
  SKIP_CHECK_RAW(); \
  switch(sew) { \
    case e8: { \
     b7 = P_.VU.check_raw<type_usew_t<e8>::type>(rs1_num, 0); \
     b8 = P_.VU.check_raw<type_usew_t<e8>::type>(rs2_num, 0); \
     b9 = P_.VU.check_raw<type_usew_t<e8>::type>(rd_num, 0); \
     break; \
    } \
    case e16: { \
     b7 = P_.VU.check_raw<type_usew_t<e16>::type>(rs1_num, 0); \
     b8 = P_.VU.check_raw<type_usew_t<e16>::type>(rs2_num, 0); \
     b9 = P_.VU.check_raw<type_usew_t<e16>::type>(rd_num, 0); \
     break; \
    } \
    case e32: { \
     b7 = P_.VU.check_raw<type_usew_t<e32>::type>(rs1_num, 0); \
     b8 = P_.VU.check_raw<type_usew_t<e32>::type>(rs2_num, 0); \
     b9 = P_.VU.check_raw<type_usew_t<e32>::type>(rd_num, 0); \
     break; \
    } \
    default: { \
     b7 = P_.VU.check_raw<type_usew_t<e64>::type>(rs1_num, 0); \
     b8 = P_.VU.check_raw<type_usew_t<e64>::type>(rs2_num, 0); \
     b9 = P_.VU.check_raw<type_usew_t<e64>::type>(rd_num, 0); \
     break; \
    } \
  } \
  return (vpu_unavail || b6 || b7 || b8 || b9);

#define VECTOR_SCALAR_VECTOR_FLOAT_CHECK_RAW() \
  CHECK_VPU_AVAIL \
  reg_t sew = P_.VU.vsew; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  SKIP_CHECK_RAW(); \
  bool b7 = false; \
  bool b8 = false; \
  bool b9 = false; \
  switch(P_.VU.vsew) { \
    case e32: {\
      b7 = P_.VU.check_raw<float32_t>(rd_num, 0); \
      b8 = C_FRS1; \
      b9 = P_.VU.check_raw<float32_t>(rs2_num, 0); \
      break; \
    }\
    case e64: {\
      b7 = P_.VU.check_raw<float64_t>(rd_num, 0); \
      b8 = C_FRS1; \
      b9 = P_.VU.check_raw<float64_t>(rs2_num, 0); \
      break; \
    }\
    case e16: \
    case e8: \
    default: \
      break; \
  }; \
  return (vpu_unavail || b6 || b7 || b8 || b9);

#define VECTOR_SCALAR_VECTOR_CHECK_RAW() \
  CHECK_VPU_AVAIL \
  reg_t sew = P_.VU.vsew; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  SKIP_CHECK_RAW(); \
  bool b7 = false; \
  bool b8 = false; \
  bool b9 = false; \
  switch(P_.VU.vsew) { \
    case e8: {\
      b7 = P_.VU.check_raw<type_sew_t<e8>::type>(rd_num, 0); \
      b8 = RS1; \
      b9 = P_.VU.check_raw<type_sew_t<e8>::type>(rs2_num, 0); \
      break; \
    }\
    case e16: {\
      b7 = P_.VU.check_raw<type_sew_t<e16>::type>(rd_num, 0); \
      b8 = RS1; \
      b9 = P_.VU.check_raw<type_sew_t<e16>::type>(rs2_num, 0); \
      break; \
    }\
    case e32: {\
      b7 = P_.VU.check_raw<type_sew_t<e32>::type>(rd_num, 0); \
      b8 = RS1; \
      b9 = P_.VU.check_raw<type_sew_t<e32>::type>(rs2_num, 0); \
      break; \
    }\
    default: {\
      b7 = P_.VU.check_raw<type_sew_t<e64>::type>(rd_num, 0); \
      b8 = RS1; \
      b9 = P_.VU.check_raw<type_sew_t<e64>::type>(rs2_num, 0); \
      break; \
    }\
  }; \
  return (vpu_unavail || b6 || b7 || b8 || b9);

#define VECTOR_SCALAR_VECTOR_UNSIGNED_CHECK_RAW() \
  CHECK_VPU_AVAIL \
  reg_t sew = P_.VU.vsew; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  SKIP_CHECK_RAW(); \
  bool b7 = false; \
  bool b8 = false; \
  bool b9 = false; \
  switch(P_.VU.vsew) { \
    case e8: {\
      b7 = P_.VU.check_raw<uint8_t>(rd_num, 0); \
      b8 = RS1; \
      b9 = P_.VU.check_raw<uint8_t>(rs2_num, 0); \
      break; \
    }\
    case e16: {\
      b7 = P_.VU.check_raw<uint16_t>(rd_num, 0); \
      b8 = RS1; \
      b9 = P_.VU.check_raw<uint16_t>(rs2_num, 0); \
      break; \
    }\
    case e32: {\
      b7 = P_.VU.check_raw<uint32_t>(rd_num, 0); \
      b8 = RS1; \
      b9 = P_.VU.check_raw<uint32_t>(rs2_num, 0); \
      break; \
    }\
    default: {\
      b7 = P_.VU.check_raw<uint64_t>(rd_num, 0); \
      b8 = RS1; \
      b9 = P_.VU.check_raw<uint64_t>(rs2_num, 0); \
      break; \
    }\
  }; \
  return (vpu_unavail || b6 || b7 || b8 || b9);

#define VECTOR_VECTOR_VECTOR_FLOAT_CHECK_RAW() \
  CHECK_VPU_AVAIL \
  reg_t sew = P_.VU.vsew; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  SKIP_CHECK_RAW(); \
  bool b7 = false; \
  bool b8 = false; \
  bool b9 = false; \
  switch(P_.VU.vsew) { \
    case e32: {\
      b7 = P_.VU.check_raw<float32_t>(rd_num, 0); \
      b8 = P_.VU.check_raw<float32_t>(rs1_num, 0); \
      b9 = P_.VU.check_raw<float32_t>(rs2_num, 0); \
      break; \
    }\
    case e64: {\
      b7 = P_.VU.check_raw<float64_t>(rd_num, 0); \
      b8 = P_.VU.check_raw<float64_t>(rs1_num, 0); \
      b9 = P_.VU.check_raw<float64_t>(rs2_num, 0); \
      break; \
    }\
    case e16: \
    case e8: \
    default: \
      break; \
  }; \
  return (vpu_unavail || b6 || b7 || b8 || b9);

#define VECTOR_VECTOR_FLOAT_CHECK_RAW() \
  CHECK_VPU_AVAIL \
  reg_t sew = P_.VU.vsew; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  SKIP_CHECK_RAW(); \
  bool b8 = false; \
  bool b9 = false; \
  switch(P_.VU.vsew) { \
    case e32: {\
      b8 = P_.VU.check_raw<float32_t>(rs1_num, 0); \
      b9 = P_.VU.check_raw<float32_t>(rs2_num, 0); \
      break; \
    }\
    case e64: {\
      b8 = P_.VU.check_raw<float64_t>(rs1_num, 0); \
      b9 = P_.VU.check_raw<float64_t>(rs2_num, 0); \
      break; \
    }\
    case e16: \
    case e8: \
    default: \
      break; \
  }; \
  return (vpu_unavail || b6 || b8 || b9);

#define SCALAR_VECTOR_FLOAT_CHECK_RAW() \
  CHECK_VPU_AVAIL \
  reg_t sew = P_.VU.vsew; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  SKIP_CHECK_RAW(); \
  bool b8 = false; \
  bool b9 = false; \
  switch(P_.VU.vsew) { \
    case e32: {\
      b8 = C_FRS1; \
      b9 = P_.VU.check_raw<float32_t>(rs2_num, 0); \
      break; \
    }\
    case e64: {\
      b8 = C_FRS1; \
      b9 = P_.VU.check_raw<float64_t>(rs2_num, 0); \
      break; \
    }\
    case e16: \
    case e8: \
    default: \
      break; \
  }; \
  return (vpu_unavail || b6 || b8 || b9);

#define VECTOR_FLOAT_CHECK_RAW() \
  CHECK_VPU_AVAIL \
  reg_t sew = P_.VU.vsew; \
  reg_t rd_num = insn.rd(); \
  reg_t rs2_num = insn.rs2(); \
  SKIP_CHECK_RAW(); \
  bool b9 = false; \
  switch(P_.VU.vsew) { \
    case e32: {\
      b9 = P_.VU.check_raw<float32_t>(rs2_num, 0); \
      break; \
    }\
    case e64: {\
      b9 = P_.VU.check_raw<float64_t>(rs2_num, 0); \
      break; \
    }\
    case e16: \
    case e8: \
    default: \
      break; \
  }; \
  return (vpu_unavail || b6 || b9);

#define VECTOR_VECTOR_CHECK_RAW() \
  CHECK_VPU_AVAIL \
  reg_t sew = P_.VU.vsew; \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  SKIP_CHECK_RAW(); \
  bool b7 = false; \
  bool b8 = false; \
  switch(sew) { \
    case e8: { \
     b7 = P_.VU.check_raw<type_sew_t<e8>::type>(rs1_num, 0); \
     b8 = P_.VU.check_raw<type_sew_t<e8>::type>(rs2_num, 0); \
     break; \
    } \
    case e16: { \
     b7 = P_.VU.check_raw<type_sew_t<e16>::type>(rs1_num, 0); \
     b8 = P_.VU.check_raw<type_sew_t<e16>::type>(rs2_num, 0); \
     break; \
    } \
    case e32: { \
     b7 = P_.VU.check_raw<type_sew_t<e32>::type>(rs1_num, 0); \
     b8 = P_.VU.check_raw<type_sew_t<e32>::type>(rs2_num, 0); \
     break; \
    } \
    default: { \
     b7 = P_.VU.check_raw<type_sew_t<e64>::type>(rs1_num, 0); \
     b8 = P_.VU.check_raw<type_sew_t<e64>::type>(rs2_num, 0); \
     break; \
    } \
  } \
  return (vpu_unavail || b6 || b7 || b8);

#define SCALAR_VECTOR_CHECK_RAW() \
  CHECK_VPU_AVAIL \
  reg_t sew = P_.VU.vsew; \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  SKIP_CHECK_RAW(); \
  bool b7 = false; \
  bool b8 = false; \
  switch(sew) { \
    case e8: { \
     b7 = C_RS1; \
     b8 = P_.VU.check_raw<type_sew_t<e8>::type>(rs2_num, 0); \
     break; \
    } \
    case e16: { \
     b7 = C_RS1; \
     b8 = P_.VU.check_raw<type_sew_t<e16>::type>(rs2_num, 0); \
     break; \
    } \
    case e32: { \
     b7 = C_RS1; \
     b8 = P_.VU.check_raw<type_sew_t<e32>::type>(rs2_num, 0); \
     break; \
    } \
    default: { \
     b7 = C_RS1; \
     b8 = P_.VU.check_raw<type_sew_t<e64>::type>(rs2_num, 0); \
     break; \
    } \
  } \
  return (vpu_unavail || b6 || b7 || b8);

#define VECTOR_CHECK_RAW() \
  CHECK_VPU_AVAIL \
  reg_t sew = P_.VU.vsew; \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  SKIP_CHECK_RAW(); \
  bool b7 = false; \
  switch(sew) { \
    case e8: { \
     b7 = P_.VU.check_raw<type_sew_t<e8>::type>(rs2_num, 0); \
     break; \
    } \
    case e16: { \
     b7 = P_.VU.check_raw<type_sew_t<e16>::type>(rs2_num, 0); \
     break; \
    } \
    case e32: { \
     b7 = P_.VU.check_raw<type_sew_t<e32>::type>(rs2_num, 0); \
     break; \
    } \
    default: { \
     b7 = P_.VU.check_raw<type_sew_t<e64>::type>(rs2_num, 0); \
     break; \
    } \
  } \
  return (vpu_unavail || b6 || b7);

#define VECTOR_VECTOR_VECTOR_CHECK_RAW() \
  CHECK_VPU_AVAIL \
  reg_t sew = P_.VU.vsew; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  SKIP_CHECK_RAW(); \
  bool b7 = false; \
  bool b8 = false; \
  bool b9 = false; \
  switch(sew) { \
    case e8: { \
     b7 = P_.VU.check_raw<type_sew_t<e8>::type>(rs1_num, 0); \
     b8 = P_.VU.check_raw<type_sew_t<e8>::type>(rs2_num, 0); \
     b9 = P_.VU.check_raw<type_sew_t<e8>::type>(rd_num, 0); \
     break; \
    } \
    case e16: { \
     b7 = P_.VU.check_raw<type_sew_t<e16>::type>(rs1_num, 0); \
     b8 = P_.VU.check_raw<type_sew_t<e16>::type>(rs2_num, 0); \
     b9 = P_.VU.check_raw<type_sew_t<e16>::type>(rd_num, 0); \
     break; \
    } \
    case e32: { \
     b7 = P_.VU.check_raw<type_sew_t<e32>::type>(rs1_num, 0); \
     b8 = P_.VU.check_raw<type_sew_t<e32>::type>(rs2_num, 0); \
     b9 = P_.VU.check_raw<type_sew_t<e32>::type>(rd_num, 0); \
     break; \
    } \
    default: { \
     b7 = P_.VU.check_raw<type_sew_t<e64>::type>(rs1_num, 0); \
     b8 = P_.VU.check_raw<type_sew_t<e64>::type>(rs2_num, 0); \
     b9 = P_.VU.check_raw<type_sew_t<e64>::type>(rd_num, 0); \
     break; \
    } \
  } \
  return (vpu_unavail || b6 || b7 || b8 || b9);

#define SCALAR_DEST_VECTOR_UNSIGNED_CHECK_RAW() \
      SKIP_CHECK_RAW(); \
      CHECK_VPU_AVAIL \
      bool b7 = C_RS1;\
      bool b8 = false; \
      switch (P_.VU.vsew) { \
      case e8: \
        b8 = P_.VU.check_raw<uint8_t>(insn.rd(), 0); \
        break; \
      case e16: \
        b8 = P_.VU.check_raw<uint16_t>(insn.rd(), 0); \
        break; \
      case e32: \
        b8 = P_.VU.check_raw<uint32_t>(insn.rd(), 0); \
        break; \
      default: \
        b8 = P_.VU.check_raw<uint64_t>(insn.rd(), 0); \
        break; \
      } \
      return (vpu_unavail || b6 || b7 || b8);

#define SCALAR_SCALAR_DEST_VECTOR_UNSIGNED_CHECK_RAW() \
      SKIP_CHECK_RAW(); \
      CHECK_VPU_AVAIL \
      bool b7 = C_RS1;\
      bool b8 = C_RS2; \
      bool b9 = false; \
      switch (P_.VU.vsew) { \
      case e8: \
        b9 = P_.VU.check_raw<uint8_t>(insn.rd(), 0); \
        break; \
      case e16: \
        b9 = P_.VU.check_raw<uint16_t>(insn.rd(), 0); \
        break; \
      case e32: \
        b9 = P_.VU.check_raw<uint32_t>(insn.rd(), 0); \
        break; \
      default: \
        b9 = P_.VU.check_raw<uint64_t>(insn.rd(), 0); \
        break; \
      } \
      return (vpu_unavail || b6 || b7 || b8 || b9);

#define VECTOR_DEST_VECTOR_UNSIGNED_CHECK_RAW() \
      SKIP_CHECK_RAW(); \
      CHECK_VPU_AVAIL \
      bool b7 = false;\
      bool b8 = false;\
      switch (P_.VU.vsew) { \
      case e8: \
        b7 = P_.VU.check_raw<uint8_t>(insn.rd(), 0); \
        b8 = P_.VU.check_raw<uint8_t>(insn.rs2(), 0); \
        break; \
      case e16: \
        b7 = P_.VU.check_raw<uint16_t>(insn.rd(), 0); \
        b8 = P_.VU.check_raw<uint16_t>(insn.rs2(), 0); \
        break; \
      case e32: \
        b7 = P_.VU.check_raw<uint32_t>(insn.rd(), 0); \
        b8 = P_.VU.check_raw<uint32_t>(insn.rs2(), 0); \
        break; \
      default: \
        b7 = P_.VU.check_raw<uint64_t>(insn.rd(), 0); \
        b8 = P_.VU.check_raw<uint64_t>(insn.rs2(), 0); \
        break; \
      } \
      return (vpu_unavail || b6 || b7 || b8);

#endif
