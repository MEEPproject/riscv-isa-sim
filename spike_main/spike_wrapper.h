
#ifndef _SPIKE_WRAPPER_H
#define _SPIKE_WRAPPER_H

#include "sim.h"
#include "cachesim.h"
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
#include "L2Request.hpp"


namespace spike_model
{
    class SpikeWrapper : public sparta::Unit
    {
        public:
            class SpikeWrapperParameterSet : public sparta::ParameterSet
            {
            public:
                //! Constructor for NoCParameterSet
                SpikeWrapperParameterSet(sparta::TreeNode* n):
                    sparta::ParameterSet(n)
                {
                }

                PARAMETER(std::string, p, "1", "The number of cores this l2 is connected to")
                PARAMETER(std::string, ic, "64:8:64", "The icache configuration")
                PARAMETER(std::string, dc, "64:8:64", "The dcache configuration")
                PARAMETER(std::string, isa, "RV64IMAFDCV", "The isa to use")
                PARAMETER(std::string, cmd, "", "The command to simulate")
                PARAMETER(std::string, varch, "v128:e64:s128", "The varch to use")
            };

            SpikeWrapper(sparta::TreeNode* node, const SpikeWrapperParameterSet* p);

            ~SpikeWrapper() 
            {
                debug_logger_ << getContainer()->getLocation()
                              << ": "
                              << std::endl;
                
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


            std::vector<icache_sim_t *> ics;
            std::vector<dcache_sim_t *> dcs;

        public:

//            SpikeWrapper(uint32_t num_cores);

//            SpikeWrapper(const SpikeWrapper &s);

            void setup(std::vector<std::string> args);

            bool simulateOne(uint16_t core, uint64_t current_cycle, std::list<std::shared_ptr<spike_model::L2Request>>& l1Misses);
            bool ackRegister(const std::shared_ptr<spike_model::L2Request> & req, uint64_t timestamp);

            void l2AckForCore(uint16_t core, uint64_t timestamp);
    };
}
#endif
