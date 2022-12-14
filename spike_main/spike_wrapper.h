
#ifndef _SPIKE_WRAPPER_H
#define _SPIKE_WRAPPER_H

#include "sim.h"
#include "cachesim.h"
#include "fastcachesim.h"
#include "serviceable.h"
#include <vector>
#include <queue>
#include <boost/algorithm/string.hpp>

#include "sparta/ports/PortSet.hpp"
#include "sparta/ports/SignalPort.hpp"
#include "sparta/ports/DataPort.hpp"
#include "sparta/events/EventSet.hpp"
#include "sparta/events/UniqueEvent.hpp"
#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"
#include "sparta/simulation/TreeNode.hpp"
#include "sparta/collection/Collectable.hpp"
#include "sparta/events/StartupEvent.hpp"
#include "sparta/resources/Pipeline.hpp"
#include "sparta/resources/Buffer.hpp"
#include "sparta/pairs/SpartaKeyPairs.hpp"
#include "sparta/simulation/State.hpp"
#include "sparta/utils/SpartaSharedPointer.hpp"

#include <memory>
#include "CacheRequest.hpp"
#include "Request.hpp"
#include "Event.hpp"


namespace coyote
{
    class SpikeWrapper
    {
        /*!
         * \class coyote::SpikeWrapper
         * \brief A representation of a wrapped Spike instance that can simulate on a instruction by instructiopn basis
         */
        public:

            /*!
             * \brief Constructor for a wrapped spike simulation
             * \param  node The node that represent the SpikeWrapper and
             * \param  p The SpikeWrapper parameter set
             */
            SpikeWrapper(std::string p, std::string t, std::string ic, std::string dc, std::string isa, std::string cmd, std::string varch, bool fast_cache, bool enable_smart_mcpu, bool vector_bypass_l1, bool vector_bypass_l2, bool l1_writeback, uint16_t lanes_per_vpu, size_t scratchpad_size);

            ~SpikeWrapper() 
            {                
                for(unsigned i=0;i<ics.size();i++)
                {
                    std::cout << "--------CORE " << i << "--------\n";
                    delete ics[i];
                    delete dcs[i];
                }
            }

            //! name of this resource.
            static const char name[];
    
            void setInstructionLogFile(std::shared_ptr<std::ofstream> f, uint64_t start, uint64_t end);
    
            void checkInstructionGraduation(std::shared_ptr<CacheRequest> req, uint64_t timestamp);


        private:
            std::string p_;
            std::string t_;
            std::string ic_;
            std::string dc_;
            std::string isa_;
            std::string cmd_;
            std::string varch_;

            std::shared_ptr<sim_t> simulation;        

            /*!
             * \brief Start a Spike simulation using the same parameters 
             *  as a command line launch of standalone Spike.
             *
             * \param  argc The number of arguments
             * \param  argv The arguments for the Spike simulation
             */
            void start_spike(int argc, const char** argv);
            
            uint32_t running_cores;


            std::vector<serviceable_cache_memtracer_t *> ics;
            std::vector<serviceable_cache_memtracer_t *> dcs;
            //std::vector<icache_sim_t *> ics;
            //std::vector<dcache_sim_t *> dcs;

            bool fast_cache;
            size_t threads_per_core;
            bool enable_smart_mcpu;
            bool vector_bypass_l1;
            bool vector_bypass_l2;
            bool l1_writeback;
            uint16_t lanes_per_vpu;
            size_t scratchpad_size;

        public:

//            SpikeWrapper(uint32_t num_cores);

//            SpikeWrapper(const SpikeWrapper &s);

            /*!
             * \brief Start a Spike simulation using a list of strings as parameters.
             * Internally it uses method start_spike.
             * \param  args The arguments for the simulation
             */
            void setup(std::vector<std::string> args);

            /*!
             * \brief Simulate a single instruction in the specified core.
             * \param  core The core in which the instruction will be simulated
             * \param  current_cycle The current cycle of the simulation
             * \param  events List for the events generated by the simulation of the instruction (output parameter)
             * \return  Whether a RAW dependency was identified or not.
             */
            bool simulateOne(uint16_t core, uint64_t current_cycle, std::list<std::shared_ptr<coyote::Event>>& events);
            
            /*!
             * \brief  Notify that a RAW dependency has been satisfied.
             * \param  coreId The core id in which the RAW dependency needs to be satisfied.
             * \param  destRegType The type of the destination register.
             * \param  destRegId The id of the destination register.
             * \param  timestamp The cycle in which the dependency was satisfied
             * \return Whether the core associated to the dependency can now run or not.
             */
            bool ackRegister(uint64_t coreId, coyote::Request::RegType destRegType, size_t destRegId, uint64_t timestamp);

            /*!
             * \brief  Set the virtual vector length of the VPU.
             * \param  coreId The core id in which the VVL needs to be set.
             * \param  vvl The VVL value.
             */
            void setVVL(uint64_t coreId, uint64_t vvl);

            bool canResume(uint64_t coreId, size_t srcRegId,
                           coyote::Request::RegType srcRegType,
                           size_t destRegId,
                           coyote::Request::RegType destRegType,
                           uint64_t latency, uint64_t timestamp);

            /*
             * \brief Notify that an L2 request has been serviced
             * \param req The serviced request
             * \param timestamp The timestamp of the service
             * \return A request for a writeback or null
             */
            std::shared_ptr<CacheRequest> serviceCacheRequest(std::shared_ptr<CacheRequest> req, uint64_t timestamp);
    
            void decrementInFlightScalarStores(uint64_t coreId);
            bool checkInFlightScalarStores(uint64_t coreId);

            /*
             * \brief Check the number of in flight misses in an L1
             * \param core_idx The id of the core of the L1 that will be checked
             */
             size_t checkNumInFlightL1Misses(uint16_t core_idx);

             /*
             * \brief Get the accumulated number of L1 hits for all the data caches
             * \return The number of L1 hits
             */
             uint64_t getNumL1DataHits();
    };
}
#endif
