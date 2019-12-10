//
//  endpoint.hpp
//  fart
//
//  Created by Kristian Trenskow on 19/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef Endpoint_hpp
#define Endpoint_hpp

#include <sys/socket.h>

#include "object.hpp"
#include "string.hpp"
#include "mutex.hpp"

using namespace fart::memory;
using namespace fart::types;
using namespace fart::threading;

namespace fart::network {
    
    enum EndpointType {
        EndpointTypeIPv4 = 0,
        EndpointTypeIPv6
    };
    
    class Endpoint : public Object {
        
    private:
        
        sockaddr_storage _storage;
        Mutex _mutex;
        
    public:
        
        Endpoint(Strong<String> host, uint16_t port, EndpointType types = EndpointTypeIPv4, uint32_t scope_id = 0);
        Endpoint(sockaddr* addr);
        virtual ~Endpoint();
        
        const EndpointType getType() const;
        Strong<String> getHost() const;
        const uint16_t getPort() const;
        void setPort(const uint16_t port);
        const sockaddr* getSockAddr() const;
        
    };

}

#endif /* endpoint_hpp */
