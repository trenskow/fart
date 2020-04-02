//
//  socket.cpp
//  fart
//
//  Created by Kristian Trenskow on 19/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#include <arpa/inet.h>

#include "./socket.hpp"

#define BUFFER_SIZE 16384

using namespace fart::network;

Socket::Socket(bool isUDP) : _isUDP(isUDP), _socket(-1), _state(SocketStateClosed), _localEndpoint(nullptr), _remoteEndpoint(nullptr) {}

Socket::Socket(int socket) : Socket() {
    _socket = socket;
}

Socket::Socket(const Socket& other) : _isUDP(other._isUDP), _socket(other._socket), _localEndpoint(other._localEndpoint), _remoteEndpoint(other._remoteEndpoint) {}

Socket::Socket(Socket&& other) : _socket(other._socket), _state(other._state), _localEndpoint(other._localEndpoint), _remoteEndpoint(other._remoteEndpoint) {
    printf("Move\n");
}

Socket::~Socket() {
    close();
    _closeCallback.callback = nullptr;
}

const uint64_t Socket::getHash() const {
    return _mutex.lockedValue([this]() {
        return _socket;
    });
}

void Socket::_read(function<void()> setup, function<void(Strong<Data<uint8_t>>, Strong<Endpoint>)> readCallback) {
    
    this->retain();
    
    _receiveThread.detach([this,setup,readCallback]() {
        
        setup();
        
        ssize_t bytesRead = 0;
        uint8_t buffer[BUFFER_SIZE];
        
        do {
            
            _mutex.lock();
            
            int socket = _socket;
            Strong<Endpoint> endpoint = nullptr;
            
            if (!_isUDP) {
                endpoint = _remoteEndpoint;
                _mutex.unlock();
                bytesRead = recv(socket, buffer, BUFFER_SIZE, 0);
            } else {
                _mutex.unlock();
                sockaddr_storage addr;
                socklen_t len = sizeof(sockaddr_storage);
                bytesRead = recvfrom(socket, buffer, BUFFER_SIZE, 0, (sockaddr *)&addr, &len);
                endpoint = Strong<Endpoint>((sockaddr*)&addr);
            }
            if (bytesRead > 0) {
                Strong<Data<uint8_t>> data(buffer, bytesRead);
                readCallback(data, endpoint);
            } else {
                close();
            }
        } while (bytesRead > 0);
        
        close();
        
        this->release();
        
    });

}

void Socket::bind(Strong<Endpoint> endpoint) {
    
    _mutex.locked([this,endpoint](){
        
        _localEndpoint = endpoint;
        
        const sockaddr* addr = _localEndpoint->getSockAddr();
        
        if (_socket < 0) {
            
            _socket = socket(addr->sa_family, (_isUDP ? SOCK_DGRAM : SOCK_STREAM), (_isUDP ? IPPROTO_UDP : IPPROTO_TCP));
            
            if (_socket < 0) {
                // TODO: Handle error
                return;
            }
            
            if (_localEndpoint->getType() == EndpointTypeIPv6) {
                int32_t on = 1;
                setsockopt(_socket, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));
            }
            
        }
        
        if (!_isUDP) {
            int reuseaddr = 1;
            setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
        }
        
        if (::bind(_socket, addr, addr->sa_len) != 0) {
            // Handle error
            return;
        }
        
    });
    
}

void Socket::listen(function<void(Socket& incoming)> acceptCallback) {
    
    this->retain();
    
    _mutex.locked([this,acceptCallback]() {
        
        if (_state != SocketStateClosed) {
            close();
        }
        
        _listenThread.detach([this,acceptCallback]() {
            
            _mutex.locked([this]() {
                
                if (::listen(_socket, 5) != 0) {
                    // Handle error;
                    return;
                }
                
                _state = SocketStateListening;
                
            });
            
            int newSocketFd = -1;
            
            do {
                
                sockaddr_storage addr;
                socklen_t len;
                
                int socket = _mutex.lockedValue([this](){ return _socket; });
                
                newSocketFd = ::accept(socket, (sockaddr *)&addr, &len);
                
                if (newSocketFd >= 0) {
                    Strong<Socket> newSocket(newSocketFd);
                    acceptCallback(newSocket);
                    if (newSocket->getSocketState() != SocketStateConnected) {
                        newSocket->close();
                    }
                }
                
            } while (newSocketFd >= 0);
            
            close();
            
            this->release();
            
        });
        
    });
    
}

void Socket::accept(function<void(Data<uint8_t>&, const Endpoint&)> readCallback) {
    _mutex.locked([this]() {
        _state = SocketStateConnected;
    });
    _read([this]() {
    }, readCallback);
}

void Socket::connect(Strong<Endpoint> endpoint, function<void(Data<uint8_t>&, const Endpoint&)> readCallback) {
    
    _mutex.locked([this,endpoint,readCallback]() {
        
        if (_isUDP) {
            // Handle error;
            return;
        }
        
        if (_state != SocketStateClosed) {
            close();
        }
        
        _remoteEndpoint = endpoint;
        
        _socket = socket(_remoteEndpoint->getSockAddr()->sa_family, SOCK_STREAM, IPPROTO_TCP);
        
        if (_socket < 0) {
            // Handle error;
            return;
        }
        
    });
    
    _read([this]() {
        
        _mutex.locked([this]() {
            
            if (::connect(_socket, _remoteEndpoint->getSockAddr(), _remoteEndpoint->getSockAddr()->sa_len) != 0) {
                // Handle error
                return;
            }
            
            _state = SocketStateConnected;
            
        });
        
    }, readCallback);
    
}

const size_t Socket::send(const Data<uint8_t>& data) const {
    return ::send(_socket, data.getItems(), data.getCount(), 0);
}

const size_t Socket::sendTo(const Endpoint& endpoint, const Data<uint8_t>& data) const {
    return 0;
}

void Socket::close() {
    if (_state != SocketStateClosed && _closeCallback.callback != nullptr) {
        _closeCallback.callback(*this, _closeCallback.context);
    }
    _mutex.locked([this]() {
        shutdown(_socket, SHUT_RDWR);
        ::close(_socket);
        _socket = -1;
        _state = SocketStateClosed;
    });
}

Strong<Endpoint> Socket::getLocalEndpoint() const {
    return _mutex.lockedValue([this]() {
        return this->_localEndpoint;
    });
}

Strong<Endpoint> Socket::getRemoteEndpoint() const {
    return _mutex.lockedValue([this]() {
        return this->_remoteEndpoint;
    });
}

const bool Socket::getIsUDP() const {
    return _mutex.lockedValue([this]() {
        return this->_isUDP;
    });
}

const SocketState Socket::getSocketState() const {
    return _mutex.lockedValue([this]() {
        return this->_state;
    });
}

void Socket::awaitClose() const {
    if (_receiveThread.getIsDetached()) _receiveThread.join();
    if (_listenThread.getIsDetached()) _listenThread.join();
}

void Socket::setCloseCallback(void (*callback)(const Socket &, void *), void *context) {
    _closeCallback.callback = callback;
    _closeCallback.context = context;
}

bool Socket::operator==(const Socket &other) const {
    return _socket == other._socket;
}
