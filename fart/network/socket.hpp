//
//  socket.hpp
//  fart
//
//  Created by Kristian Trenskow on 19/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef Socket_hpp
#define Socket_hpp

#include <thread>

#include "../memory/object.hpp"
#include "../memory/strong.hpp"
#include "../memory/weak.hpp"
#include "../threading/thread.hpp"
#include "../threading/mutex.hpp"
#include "../types/data.hpp"
#include "../types/data.hpp"
#include "./endpoint.hpp"

using namespace fart::memory;
using namespace fart::threading;
using namespace std;

namespace fart::network {
    
    enum SocketState {
        SocketStateClosed = 0,
        SocketStateListening,
        SocketStateConnected
    };
    
    class Socket : public Object, public Hashable {
        
    public:
        
        class ICloseListener {
        public:
            virtual void SocketClosed(const Socket& socket) const = 0;
        };
        
        Socket(int socket);
        Socket(bool isUDP = false);
        Socket(const Socket& other);
        Socket(Socket&& other);
        virtual ~Socket();
        
        virtual const uint64_t hash() const;
        
        void bind(Strong<Endpoint> endpoint);
        void listen(function<void(Socket&)> acceptCallback);
        void accept(function<void(Data<uint8_t>&, const Endpoint&)> readCallback);
        void connect(Strong<Endpoint> endpoint, function<void(Data<uint8_t>&, const Endpoint&)> readCallback);
        const size_t send(const Data<uint8_t>& data) const;
        const size_t sendTo(const Endpoint& endpoint, const Data<uint8_t>& data) const;
        void close();
        
        Strong<Endpoint> localEndpoint() const;
        Strong<Endpoint> remoteEndpoint() const;
        
        const bool isUDP() const;
        const SocketState socketState() const;
        
        void awaitClose() const;
        
        void setCloseCallback(void (*callback)(const Socket& socket, void* context), void* context);
        
        bool operator==(const Socket& other) const;
        
    private:
        
        struct CloseCallback {
            CloseCallback() : callback(nullptr), context(nullptr) {}
            void (*callback)(const Socket& socket, void* context);
            void *context;
        };
        
        bool _isUDP;
        
        int _socket;
        SocketState _state;
        
        Strong<Endpoint> _localEndpoint;
        Strong<Endpoint> _remoteEndpoint;
                    
        Thread _listenThread;
        Thread _receiveThread;
        
        Mutex _mutex;
        
        CloseCallback _closeCallback;
        
        void _read(function<void()> setup, function<void(Strong<Data<uint8_t>>, Strong<Endpoint> endpoint)> readCallback);
        
    };

}

#endif /* socket_hpp */
