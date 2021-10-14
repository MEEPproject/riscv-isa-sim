// See LICENSE for license details.

#include "fastcachesim.h"
#include "common.h"
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <math.h>  

fast_cache_sim_t::fast_cache_sim_t(size_t lines, size_t length, size_t log2Lines, size_t log2LineLength, size_t numways, const char* _name)
: lines(lines), length(length), log2Lines(log2Lines), log2LineLength(log2LineLength), numways(numways), name(_name), replacementMethod(replaceLRU), log(false)
{
  init();
}

static void help()
{
  std::cerr << "Cache configurations must be of the form" << std::endl;
  std::cerr << "  sets:ways:blocksize" << std::endl;
  std::cerr << "where sets, ways, and blocksize are positive integers, with" << std::endl;
  std::cerr << "sets and blocksize both powers of two and blocksize at least 8." << std::endl;
  exit(1);
}

fast_cache_sim_t* fast_cache_sim_t::construct(const char* config, const char* name)
{
    const char* wp = strchr(config, ':');
    if (!wp++) help();
    const char* bp = strchr(wp, ':');
    if (!bp++) help();
    size_t sets = atoi(std::string(config, wp).c_str());

    // sets up the specified cacheData
    size_t lines =  sets;
    size_t length = atoi(bp);		// BYTES
    size_t log2Lines = log2(lines);
    size_t log2LineLength = log2(length);
    size_t numways = atoi(std::string(wp, bp).c_str());

    return new fast_cache_sim_t(lines, length, log2Lines, log2LineLength, numways, strdup(name));
}

void fast_cache_sim_t::init()
{
    ways = (cacheWay **) malloc(numways * sizeof(cacheWay *));

    for (uint32_t way = 0; way < numways; way++) {
        cacheWay * cw = (cacheWay *) malloc(sizeof(cacheWay));
        if (cw) {
            char * wayname = (char *) malloc(64);
            sprintf(wayname, "way[%d]", way);
            cw -> name = wayname;
            ways[way] = cw;
            cw -> tags = (uint64_t *) malloc(lines * sizeof(uint64_t));
            if (cw -> tags) cw -> valid = (uint8_t *) malloc(lines * sizeof(uint8_t));
            if (cw -> valid) cw -> dirty = (uint8_t *) malloc(lines * sizeof(uint8_t));
            if (cw -> dirty) cw -> timetag = (uint64_t *) malloc(lines * sizeof(uint64_t));

            if (cw -> timetag == NULL) {
                printf("Allocate error: internal structure of a cache way\n");
            }
        }
        else {
            printf("Allocate error: cache way\n");
        }
    }
    read_accesses = 0;
    read_misses = 0;
    bytes_read = 0;
    write_accesses = 0;
    write_misses = 0;
    bytes_written = 0;
    writebacks = 0;
		
    lasthittag = -1;
	lasthitway = NULL;

    miss_handler = NULL;
}

fast_cache_sim_t::fast_cache_sim_t(const fast_cache_sim_t& rhs)
 : lines(rhs.lines), length(rhs.length), log2Lines(rhs.log2Lines), log2LineLength(rhs.log2LineLength), numways(rhs.numways), name(rhs.name), log(false) 
{
  ways = (cacheWay **) malloc(numways * sizeof(cacheWay *));
  memcpy(ways, rhs.ways, numways * sizeof(cacheWay *));
}

fast_cache_sim_t::~fast_cache_sim_t()
{
  print_stats();
  delete [] ways;
}

void fast_cache_sim_t::print_stats()
{
  if(read_accesses + write_accesses == 0)
    return;

  float mr = 100.0f*(read_misses+write_misses)/(read_accesses+write_accesses);

  std::cout << std::setprecision(3) << std::fixed;
  std::cout << name << " ";
  std::cout << "Bytes Read:            " << bytes_read << std::endl;
  std::cout << name << " ";
  std::cout << "Bytes Written:         " << bytes_written << std::endl;
  std::cout << name << " ";
  std::cout << "Read Accesses:         " << read_accesses << std::endl;
  std::cout << name << " ";
  std::cout << "Write Accesses:        " << write_accesses << std::endl;
  std::cout << name << " ";
  std::cout << "Read Misses:           " << read_misses << std::endl;
  std::cout << name << " ";
  std::cout << "Write Misses:          " << write_misses << std::endl;
  std::cout << name << " ";
  std::cout << "Writebacks:            " << writebacks << std::endl;
  std::cout << name << " ";
  std::cout << "Miss Rate:             " << mr << '%' << std::endl;
}


uint64_t fast_cache_sim_t::addrMaskForCache() {
	uint64_t mask = (1 << log2Lines) - 1;
	return mask;
}

// ------------- addrToTag ---------------------

uint64_t fast_cache_sim_t::addressToTag(uint64_t address) {
	uint64_t mask = (1 << log2LineLength) - 1;
	uint64_t tag = address & (~mask);
	return tag;
}

uint64_t fast_cache_sim_t::getIndex(uint64_t address) {
	// **** new cache design ****
	// for the new cache access functions iCacheAccess() and dcacheAccess() only
	// in these we keep the unshifted but masked address as the tag
	uint64_t mask = addrMaskForCache() << log2LineLength;
	uint64_t index = address & mask;
	index = index >> log2LineLength;
	return index;
}

uint64_t fast_cache_sim_t::getLineAddress(uint64_t tag, uint64_t index) {
    return (tag << (log2Lines + log2LineLength)) | (index << log2LineLength);
}

bool fast_cache_sim_t::i_cache_access(uint64_t address)
{
	// faster icache, because no dirty, no eviction
	// plus cached 'most recent tag' to allow quick hit determination
	// compute index into the cache, and the matching value for the ta
	read_accesses++;
	uint64_t index = getIndex(address);

	uint64_t maskedAddress = addressToTag(address);
	if (maskedAddress == lastHitLineStartAddress) {
		return true;
	}
	// ok, the quickie failed: look across the ways to see if we hit
	for (uint64_t way = 0; way < numways; way++) {
		cacheWay * cw = ways[way];
		if (cw -> tags[index] == maskedAddress) {			// the invalid tags are negative, so a simple equality test works
			// update its time for LRU
			cw -> timetag[index] = read_accesses;		//LRU info
			// update the cached info
			lastHitLineStartAddress = maskedAddress;
			lasthitway = cw;
			return true;
		}
	}

	// we missed.
	// first, update LRU for the way we missed in
	cacheWay * cw = lasthitway;
	if (cw) cw -> timetag[index] = read_accesses;

	//choose a way according to the replacement algo specified for the cache,
	// then eject the incumbent if any and install the new
	read_misses++;						// count misses. we compute hits from accesses - misses
	int way = 0;
	lastHitLineStartAddress = -1;
	switch (replacementMethod) {

		case replaceDirect:
			way = 0;
			break;

		case replaceLRU:
		{
			// look across the ways at this index and find the oldest - the one with the smallest timetag
			uint64_t oldest = read_accesses;
			for (uint64_t i = 0; i < numways; i++) {
				cacheWay * cw = ways[i];
				if (cw -> timetag[index] <= oldest) {
					oldest = cw -> timetag[index];
					way = i;
				}
			}
		}
			break;

		default:
			printf("\nERROR - illegal/unimplemented way replacement mechanism.");
	}

	// set up this line
	cw = ways[way];

	// update line with new tag, validty and timetag
	cw -> tags[index] = maskedAddress;
	cw -> valid[index] = 1;
	cw -> timetag[index] = read_accesses;

	return false;
}

bool fast_cache_sim_t::d_cache_read(uint64_t address)
{
	// hopefully faster dcache
	// compute index into the cache, and the matching value for the tag
	read_accesses++;
	uint64_t index = getIndex(address);

	uint64_t maskedAddress = addressToTag(address);
	if (maskedAddress == lastHitLineStartAddress) {
		// we hit in the same line! update LRU info
		cacheWay * cw = lasthitway;
		cw -> timetag[index] = read_accesses+write_accesses;
		cw -> dirty[index] = 0;
		return true;
	}

	// ok, the quickie failed. look across the ways to see if we hit
	for (uint64_t way = 0; way < numways; way++) {
		cacheWay * cw = ways[way];
		if (cw -> tags[index] == maskedAddress) {
			// update its time for LRU
			cw -> timetag[index] = read_accesses+write_accesses;		//LRU info
			// update the cached info
			lastHitLineStartAddress = maskedAddress;
			lasthitway = cw;
			return true;
		}
	}

	// we missed. choose a way according to the replacement algo specified for the cache,
	// then eject the incumbent if any and install the new
	read_misses++;						// count misses. we compute hits from accesses - misses
	int way = 0;
	lastHitLineStartAddress = -1;
	switch (replacementMethod) {

		case replaceLRU:
		{
			// look across the ways at this index and find the oldest - the one with the smallest timetag
			uint64_t oldest = read_accesses+write_accesses;
			for (uint64_t i = 0; i < numways; i++) {
				cacheWay * cw = ways[i];
				if (cw -> timetag[index] <= oldest) {
					oldest = cw -> timetag[index];
					way = i;
				}
			}
		}
			break;

		default:
			printf("\nERROR - illegal/unimplemented way replacement mechanism.");
	}

	cacheWay * cw = ways[way];
	// if the line was already dirty, we already have a cacheline here, so eject the line
	if (cw -> dirty[index]) {
		writebacks++;
	}

	// update line with new tag, validity and timetag
	cw -> tags[index] = maskedAddress;
	cw -> valid[index] = 1;
	cw -> timetag[index] = read_accesses+write_accesses;
	cw -> dirty[index] = 0;

	return false;

}

bool fast_cache_sim_t::d_cache_write(uint64_t address)
{
	// hopefully faster dcache
	// compute index into the cache, and the matching value for the tag
	write_accesses++;
	uint64_t index = getIndex(address);

	uint64_t maskedAddress = addressToTag(address);
	if (maskedAddress == lastHitLineStartAddress) {
		// we hit in the same line! update LRU info
		cacheWay * cw = lasthitway;
		cw -> timetag[index] = read_accesses+write_accesses;
		cw -> dirty[index] = 1;
		return true;
	}

	// ok, the quickie failed. look across the ways to see if we hit
	for (uint64_t way = 0; way < numways; way++) {
		cacheWay * cw = ways[way];
		if (cw -> tags[index] == maskedAddress) {
			// update its time for LRU
			cw -> timetag[index] = read_accesses+write_accesses;		//LRU info
			// update the cached info
			lastHitLineStartAddress = maskedAddress;
			lasthitway = cw;
			return true;
		}
	}

	// we missed. choose a way according to the replacement algo specified for the cache,
	// then eject the incumbent if any and install the new
	write_misses++;						// count misses. we compute hits from accesses - misses
	int way = 0;
	lastHitLineStartAddress = -1;
	switch (replacementMethod) {

		case replaceDirect:
			way = 0;
			break;

		case replaceLRU:
		{
			// look across the ways at this index and find the oldest - the one with the smallest timetag
			uint64_t oldest = read_accesses+write_accesses;
			for (uint64_t i = 0; i < numways; i++) {
				cacheWay * cw = ways[i];
				if (cw -> timetag[index] <= oldest) {
					oldest = cw -> timetag[index];
					way = i;
				}
			}
		}
			break;

		default:
			printf("\nERROR - illegal/unimplemented way replacement mechanism.");
	}

	cacheWay * cw = ways[way];
	// if the line was already dirty, we already have a cacheline here, so eject the line
	if (cw -> dirty[index]) {
		writebacks++;
	}

	// update line with new tag, validty and timetag
	cw -> tags[index] = maskedAddress;
	cw -> timetag[index] = read_accesses+write_accesses;
	cw -> dirty[index] = 1;

	return false;
}
