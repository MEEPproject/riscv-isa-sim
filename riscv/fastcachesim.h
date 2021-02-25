// See LICENSE for license details.

#ifndef _RISCV_FAST_CACHE_SIM_H
#define _RISCV_FAST_CACHE_SIM_H

#include "memtracer.h"
#include "cachesim.h"
#include <cstring>
#include <string>
#include <map>
#include <cstdint>

#include <queue>
#include <memory>

#include "serviceable.h"
#include "serviceablecachesim.h"

typedef struct {
	char * name;
	uint64_t * tags;				// arrays of tags, dirty bits, valid bits and timetags
	uint8_t * dirty;
	uint8_t * valid;
	uint64_t * timetag;
} cacheWay;

typedef enum {replacePseudoLRU, replaceLRU, replaceDirect } wayReplacement;

class fast_cache_sim_t : public serviceable
{
 public:
  fast_cache_sim_t(size_t lines, size_t length, size_t log2Lines, size_t log2LineLength, size_t numways, const char* _name);
  fast_cache_sim_t(const fast_cache_sim_t& rhs);
  virtual ~fast_cache_sim_t();

  void print_stats();
  void set_miss_handler(cache_sim_t* mh) { miss_handler = mh; }
  void set_log(bool _log) { log = _log; }

  static fast_cache_sim_t* construct(const char* config, const char* name);

  uint64_t get_victim();

  bool i_cache_access(uint64_t address);
  bool d_cache_read(uint64_t address);
  bool d_cache_write(uint64_t address);
  
  virtual std::shared_ptr<spike_model::Request> serviceRequest(std::shared_ptr<spike_model::Request> req){return std::shared_ptr<spike_model::Request>(nullptr);};//TODO

 protected:
  void init();
  uint64_t getIndex(uint64_t address);
  uint64_t addrMaskForCache();
  uint64_t addressToTag(uint64_t address);
  uint64_t getLineAddress(uint64_t tag, uint64_t index);

  lfsr_t lfsr;
  cache_sim_t* miss_handler;

  uint64_t read_accesses;
  uint64_t read_misses;
  uint64_t bytes_read;
  uint64_t write_accesses;
  uint64_t write_misses;
  uint64_t bytes_written;
  uint64_t writebacks;

  uint64_t last_victim=0;

	

  uint32_t isACache;

  uint64_t lastHitLineStartAddress;
  uint64_t lasthittag;
  cacheWay * lasthitway;
  

  int req_tag;			// tag used for outgoing requests

  // Geometry
  uint32_t lines;
  uint32_t length;			// in BYTES
  uint32_t log2Lines;
  uint32_t log2LineLength;

  uint32_t numways;
  cacheWay ** ways;
  
  std::string name;
  
  wayReplacement replacementMethod;
  
  bool log;
};

class fast_cache_memtracer_t : public serviceable_cache_memtracer_t
{
 public:
  fast_cache_memtracer_t(){};
  fast_cache_memtracer_t(const char* config, const char* name)
  {
    cache = fast_cache_sim_t::construct(config, name);
  }
  ~fast_cache_memtracer_t()
  {
    delete cache;
  }
  void set_miss_handler(cache_sim_t* mh)
  {
    cache->set_miss_handler(mh);
  }
  void set_log(bool log)
  {
    cache->set_log(log);
  }


 protected:
  fast_cache_sim_t* cache;

};


class fast_icache_sim_t : public fast_cache_memtracer_t
{
 public:
  fast_icache_sim_t(const char* config) : fast_cache_memtracer_t(config, "I$") {}
  bool interested_in_range(uint64_t begin, uint64_t end, access_type type)
  {
    return type == FETCH;
  }
  bool trace(uint64_t addr, size_t bytes, access_type type, bool& hit, uint64_t& victim)
  {
    bool res=false;
    victim=0;
    if (type == FETCH)
    {
        res=true;
        hit=cache->i_cache_access(addr);
    }
    return res;
  }
};

class fast_dcache_sim_t : public fast_cache_memtracer_t
{
 public:
  fast_dcache_sim_t(const char* config) : fast_cache_memtracer_t(config, "D$") {}
  bool interested_in_range(uint64_t begin, uint64_t end, access_type type)
  {
    return type == LOAD || type == STORE;
  }
  
  bool trace(uint64_t addr, size_t bytes, access_type type, bool& hit, uint64_t& victim)
  {
    bool res=false;
    victim=0;
    if (type == LOAD || type == STORE)
    {
        res=true;
        bool local_hit;
        if(type == LOAD)
        {
            local_hit=cache->d_cache_read(addr);
        }
        else
        {
            local_hit=cache->d_cache_write(addr);
        }
        victim=cache->get_victim();
        hit=local_hit;
    }
    return res;
  }
};

#endif
