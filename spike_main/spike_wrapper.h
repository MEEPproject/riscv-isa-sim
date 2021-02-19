
#ifndef _SPIKE_WRAPPER_H
#define _SPIKE_WRAPPER_H

#include "sim.h"
#include "cachesim.h"
#include "fastcachesim.h"
#include <vector>
#include <queue>

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
#include "Request.hpp"


namespace spike_model
{
    class SpikeWrapper
    {
        public:

            SpikeWrapper(std::string p, std::string ic, std::string dc, std::string isa, std::string cmd, std::string varch, bool fast_cache);

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


        private:
            std::string p_;
            std::string ic_;
            std::string dc_;
            std::string isa_;
            std::string cmd_;
            std::string varch_;

            std::shared_ptr<sim_t> simulation;        

            void start_spike(int argc, const char** argv);
            
            uint32_t running_cores;


            std::vector<memtracer_t *> ics;
            std::vector<memtracer_t *> dcs;
            //std::vector<icache_sim_t *> ics;
            //std::vector<dcache_sim_t *> dcs;

            bool fast_cache;

        public:

//            SpikeWrapper(uint32_t num_cores);

//            SpikeWrapper(const SpikeWrapper &s);

            void setup(std::vector<std::string> args);

            bool simulateOne(uint16_t core, uint64_t current_cycle, std::list<std::shared_ptr<spike_model::Request>>& l1Misses);
            bool ackRegister(const std::shared_ptr<spike_model::Request> & req, uint64_t timestamp);

            void l2AckForCore(uint16_t core, uint64_t timestamp);
    };
}
#endif
