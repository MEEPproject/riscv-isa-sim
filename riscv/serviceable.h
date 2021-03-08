#ifndef _SERVICEABLE_H
#define _SERVICEABLE_H

#include <memory>
#include "CacheRequest.hpp"

class serviceable
{
    public:
        virtual std::shared_ptr<spike_model::CacheRequest> serviceCacheRequest(std::shared_ptr<spike_model::CacheRequest> req)=0;
        virtual ~serviceable(){};
};
#endif
