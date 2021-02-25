// See LICENSE for license details.

#ifndef _RISCV_SERVICEABLE_CACHE_SIM_H
#define _RISCV_SERVICEABLE_CACHE_SIM_H

#include "cachesim.h"
#include "serviceable.h"
#include "memtracer.h"

#include "Request.hpp"

class serviceable_cache_sim_t : public cache_sim_t, serviceable
{
 public:
   serviceable_cache_sim_t(size_t _sets, size_t _ways, size_t _linesz, const char* _name);
   virtual std::shared_ptr<spike_model::Request> serviceRequest(std::shared_ptr<spike_model::Request> req);
   bool access(uint64_t addr, size_t bytes, bool store);
   virtual ~serviceable_cache_sim_t(){};

   static serviceable_cache_sim_t* construct(const char* config, const char* name);
};

class serviceable_cache_memtracer_t : public memtracer_t
{
 public:
  serviceable_cache_memtracer_t(){}
  serviceable_cache_memtracer_t(const char* config, const char* name)
  {
    cache = serviceable_cache_sim_t::construct(config, name);
  }
  ~serviceable_cache_memtracer_t()
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

  std::shared_ptr<spike_model::Request> serviceRequest(std::shared_ptr<spike_model::Request> req)
  {
    return cache->serviceRequest(req);
  }

 protected:
  serviceable_cache_sim_t* cache;

};

class serviceable_icache_sim_t : public serviceable_cache_memtracer_t
{
 public:
  serviceable_icache_sim_t(const char* config) : serviceable_cache_memtracer_t(config, "I$") {}
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
        hit=cache->access(addr, bytes, false);
    }
    return res;
  }
};

class serviceable_dcache_sim_t : public serviceable_cache_memtracer_t
{
 public:
  serviceable_dcache_sim_t(const char* config) : serviceable_cache_memtracer_t(config, "D$") {}
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
        hit=cache->access(addr, bytes, type == STORE);
        victim=cache->get_victim();
    }
    return res;
  }
};

#endif
