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
#include "../socket.hpp"
#include "../../memory/object.hpp"

using namespace fart::exceptions::network::web;

namespace fart::network::web {
    
    template<class Request, class Response>
    class Server : public Object {
        
    public:
        
        Server(uint16_t port, function<void(const Message<Request>& request, Message<Response>& response)> requestHandler) : _requestHandler(requestHandler) {
            _listener.bind(Strong<Endpoint>(port));
            _listener.listen([this](Socket& acceptSocket) {
                _connections.append(acceptSocket);
                acceptSocket.accept([this,&acceptSocket](Data<uint8_t>& data, const Endpoint& sender) {
                    this->_onData(data, acceptSocket);
                });
            });
        }
        
    protected:
        
        virtual void postProcess(Message<Request> request, Socket& socket) const {}
        
    private:
        
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
            
            socket.send(response->getData());
            
            if (!request->hasHeader("connection") || *request->getHeaderValue("connection") != "keep-alive") {
                socket.close();
            }
            
        }
        
        Socket _listener;
        Array<Socket> _connections;
        Strong<Data<uint8_t>> _backbuffer;
        function<void(const Message<Request>& request, Message<Response>& response)> _requestHandler;
        
    };
    
}

#endif /* server_hpp */
