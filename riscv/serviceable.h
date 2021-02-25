#ifndef _SERVICEABLE_H
#define _SERVICEABLE_H

#include <memory>
#include "Request.hpp"

class serviceable
{
    public:
        virtual std::shared_ptr<spike_model::Request> serviceRequest(std::shared_ptr<spike_model::Request> req)=0;
        virtual ~serviceable(){};
};
#endif
