// See LICENSE for license details.

#include "serviceablecachesim.h"
#include "common.h"
#include <cstdlib>
#include <iostream>
#include <iomanip>

#include "CacheRequest.hpp"

serviceable_cache_sim_t::serviceable_cache_sim_t(size_t _sets, size_t _ways, size_t _linesz, const char* _name)
: cache_sim_t(_sets, _ways, _linesz, _name)
{
    set_log(false);    
}

static void help()
{
  std::cerr << "Cache configurations must be of the form" << std::endl;
  std::cerr << "  sets:ways:blocksize" << std::endl;
  std::cerr << "where sets, ways, and blocksize are positive integers, with" << std::endl;
  std::cerr << "sets and blocksize both powers of two and blocksize at least 8." << std::endl;
  exit(1);
}

serviceable_cache_sim_t* serviceable_cache_sim_t::construct(const char* config, const char* name)
{
  const char* wp = strchr(config, ':');
  if (!wp++) help();
  const char* bp = strchr(wp, ':');
  if (!bp++) help();

  size_t sets = atoi(std::string(config, wp).c_str());
  size_t ways = atoi(std::string(wp, bp).c_str());
  size_t linesz = atoi(bp);

  return new serviceable_cache_sim_t(sets, ways, linesz, name);
}

bool serviceable_cache_sim_t::access(uint64_t addr, size_t bytes, bool store)
{
  store ? write_accesses++ : read_accesses++;
  (store ? bytes_written : bytes_read) += bytes;

  uint64_t* hit_way = check_tag(addr);
  if (likely(hit_way != NULL))
  {
    if (store && is_writeback())
      *hit_way |= DIRTY;
    return true;
  }

  size_t miss_line_addr=((unsigned)addr/linesz)*linesz;

  store ? write_misses++ : read_misses++;
  if (log)
  {
    std::cerr << name << " "
              << (store ? "write" : "read") << " miss 0x"
              << std::hex << addr << std::endl;
  }
  
  if (miss_handler)
    miss_handler->access(addr & ~(linesz-1), linesz, false);

  return false;
}

std::shared_ptr<coyote::CacheRequest> serviceable_cache_sim_t::serviceCacheRequest(std::shared_ptr<coyote::CacheRequest> req)
{
  std::shared_ptr<coyote::CacheRequest> wb=std::shared_ptr<coyote::CacheRequest>(nullptr);
  uint64_t victim=victimize(req->getAddress());
  
  if ((victim & (VALID | DIRTY)) == (VALID | DIRTY))
  {
    uint64_t dirty_addr = (victim & ~(VALID | DIRTY)) << idx_shift;
    if (miss_handler)
      miss_handler->access(dirty_addr, linesz, true);
    writebacks++;
    wb=std::make_shared<coyote::CacheRequest>(victim<<idx_shift, coyote::CacheRequest::AccessType::WRITEBACK);
    wb->setSize(linesz);
    wb->setDestinationReg(req->getDestinationRegId(), req->getDestinationRegType());
  }
  if (req->getType()==coyote::CacheRequest::AccessType::STORE)
  {
    *check_tag(req->getAddress()) |= DIRTY;
  }
 
  return wb;
}
