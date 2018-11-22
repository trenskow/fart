//
//  socket.cpp
//  fart
//
//  Created by Kristian Trenskow on 19/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#include "socket.hpp"

using namespace fart::network;

Socket::Socket(bool isUDP) : _isUDP(isUDP), _localEndpoint(nullptr), _remoteEndpoint(nullptr) {
    
}

Socket::Socket(Strong<Endpoint> endpoint) : Socket(false) {
    
}

Socket::~Socket() {
    
}

void Socket::bind() const {
    
}

void Socket::connect() const {
    
}

const size_t Socket::send(Strong<Data<uint8_t>> data) const {
    return 0;
}

const size_t Socket::sendTo(Strong<Endpoint> endpoint, Strong<Data<uint8_t>> data) const {
    return 0;
}

void Socket::close() const {
    
}

Strong<Endpoint> Socket::getLocalEndpoint() const {
    return nullptr;
}

Strong<Endpoint> Socket::getRemoteEndpoint() const {
    return nullptr;
}

const bool Socket::getIsUDP() const {
    return this->_isUDP;
}

const bool Socket::getIsConnected() const {
    return false;
}
