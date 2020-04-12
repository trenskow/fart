//
//  server.hpp
//  fart
//
//  Created by Kristian Trenskow on 11/12/2019.
//  Copyright © 2019 Kristian Trenskow. All rights reserved.
//

#ifndef server_hpp
#define server_hpp

#include <thread>

#include "./message.hpp"
#include "../io/sockets/socket.hpp"
#include "../memory/object.hpp"

using namespace fart::io::sockets;

using namespace fart::exceptions::web;

namespace fart::web {
    
    template<class Request, class Response>
    class Server : public Object {
        
    public:
        
        Server(uint16_t port, function<void(const Message<Request>& request, Message<Response>& response)> requestHandler) : _requestHandler(requestHandler) {
            _listener.bind(Strong<Endpoint>(port));
            _listener.listen([this](Socket& acceptSocket) {
                acceptSocket.setCloseCallback(_socketClosed, this);
                _connections.append(acceptSocket);
                acceptSocket.accept([this,&acceptSocket](Data<uint8_t>& data, const Endpoint& sender) {
                    this->_onData(data, acceptSocket);
                });
            });
        }
        
    protected:
        
        virtual void postProcess(const Message<Request>& request, Socket& socket) const {}
        
    private:
        
        void _socketClosed(const Socket& socket) {
            _connections.removeItem(socket);
        }
        
        static void _socketClosed(const Socket& socket, void* context) {
            ((Server<Request, Response>*)context)->_socketClosed(socket);
        }
        
        void _onData(Data<uint8_t>& data, Socket& socket) {
            
            _backbuffer->append(data);
            
            Strong<Message<Request>> request = nullptr;
            
            try {
                request = Strong<Message<Request>>(_backbuffer);
            } catch (DataIncompleteException) {
                return;
            } catch (DataMalformedException) {
                socket.close();
            }
            
            Strong<Message<Response>> response;
            
            _requestHandler(request, response);
            
            socket.send(response->data());
            
            postProcess(request, socket);
                        
        }
        
        Socket _listener;
        Array<Socket> _connections;
        Strong<Data<uint8_t>> _backbuffer;
        function<void(const Message<Request>& request, Message<Response>& response)> _requestHandler;
        
    };
    
}

#endif /* server_hpp */
