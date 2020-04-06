//
//  endpoint.cpp
//  fart
//
//  Created by Kristian Trenskow on 19/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#include <netinet/in.h>
#include <arpa/inet.h>

#include "./endpoint.hpp"

using namespace fart::network;

Endpoint::Endpoint(Strong<String> host, uint16_t port, EndpointType type, uint32_t scope_id) {
    
    switch (type) {
        case EndpointTypeIPv4: {
            
            sockaddr_in* addr = (sockaddr_in *)&this->_storage;
            
            addr->sin_len = sizeof(sockaddr_in);
            addr->sin_family = AF_INET;
            addr->sin_addr.s_addr = INADDR_ANY;
            addr->sin_port = htons(port);
            
            host->withCString([&addr](const char* host){
                inet_pton(AF_INET, host, &addr->sin_addr);
            });
            
        } break;
        case EndpointTypeIPv6: {
            
            sockaddr_in6* addr = ((sockaddr_in6*)&this->_storage);
            
            addr->sin6_len = sizeof(sockaddr_in6);
            addr->sin6_family = AF_INET6;
            addr->sin6_addr = in6addr_any;
            addr->sin6_port = htons(port);
            addr->sin6_scope_id = scope_id;
            
            host->withCString([&addr](const char* host) {
                inet_pton(AF_INET6, host, &addr->sin6_addr);
            });

        } break;
    }
    
}

Endpoint::Endpoint(const char* host, uint16_t port, EndpointType types, uint32_t scope_id) : Endpoint(Strong<String>(host), port, types, scope_id) {}

Endpoint::Endpoint(sockaddr *addr) {
    memcpy(&this->_storage, addr, addr->sa_len);
}

Endpoint::Endpoint(uint16_t port, EndpointType types, uint32_t scope_id) : Endpoint("0.0.0.0", port, types, scope_id) {}

Endpoint::~Endpoint() {}

const EndpointType Endpoint::type() const {
    return _mutex.lockedValue([this]() {
        if (this->_storage.ss_family == AF_INET) return EndpointTypeIPv4;
        return EndpointTypeIPv6;
    });
}

Strong<String> Endpoint::host() const {
    
    return _mutex.lockedValue([this]() {
        
        char ret[1024];
        
        switch (this->type()) {
            case EndpointTypeIPv4:
                inet_ntop(AF_INET, &((sockaddr_in *)&this->_storage)->sin_addr, ret, 1024);
                break;
            case EndpointTypeIPv6:
                inet_ntop(AF_INET6, &((sockaddr_in6 *)&this->_storage)->sin6_addr, ret, 1024);
                break;
        }
        
        return Strong<String>(ret);
        
    });
    
}

const uint16_t Endpoint::port() const {
    return _mutex.lockedValue([this]() {
        switch (this->type()) {
            case EndpointTypeIPv4:
                return ntohs(((sockaddr_in *)&this->_storage)->sin_port);
            case EndpointTypeIPv6:
                return ntohs(((sockaddr_in6 *)&this->_storage)->sin6_port);
        }
    });
}

void Endpoint::setPort(const uint16_t port) {
    _mutex.locked([this,port]() {
        switch (this->type()) {
            case EndpointTypeIPv4:
                ((sockaddr_in *)&this->_storage)->sin_port = htons(port);
                break;
            case EndpointTypeIPv6:
                ((sockaddr_in6 *)&this->_storage)->sin6_port = htons(port);
                break;
        }
    });
}

const sockaddr* Endpoint::sockAddr() const {
    return _mutex.lockedValue([this]() {
        return (sockaddr *)&this->_storage;
    });
}
