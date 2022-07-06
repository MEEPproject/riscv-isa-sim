#ifndef _SERVICEABLE_H
#define _SERVICEABLE_H

#include <memory>
#include "CacheRequest.hpp"

class serviceable
{
    public:
        virtual std::shared_ptr<coyote::CacheRequest> serviceCacheRequest(std::shared_ptr<coyote::CacheRequest> req)=0;
        virtual ~serviceable(){};
};
#endif
