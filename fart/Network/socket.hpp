//
//  socket.hpp
//  fart
//
//  Created by Kristian Trenskow on 19/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef Socket_hpp
#define Socket_hpp

#import <thread>

#import "object.hpp"
#import "strong.hpp"
#import "weak.hpp"
#import "endpoint.hpp"
#import "data.hpp"

using namespace fart::memory;
using namespace std;

namespace fart {
    
    namespace network {
        
        class Socket : public Object {
            
        private:
            
            bool _isUDP;
            
            Strong<Endpoint> _localEndpoint;
            Strong<Endpoint> _remoteEndpoint;
            
        public:
            
            Socket(bool isUDP = false);
            Socket(Strong<Endpoint> endpoint);
            virtual ~Socket();
            
            void bind() const;
            void connect() const;
            const size_t send(Strong<Data<uint8_t>> data) const;
            const size_t sendTo(Strong<Endpoint> endpoint, Strong<Data<uint8_t>> data) const;
            void close() const;
            
            Strong<Endpoint> getLocalEndpoint() const;
            Strong<Endpoint> getRemoteEndpoint() const;
            
            const bool getIsUDP() const;
            const bool getIsConnected() const;
            
        };
        
    }
    
}

#endif /* socket_hpp */
