//
//  request.hpp
//  fart
//
//  Created by Kristian Trenskow on 12/12/2019.
//  Copyright © 2019 Kristian Trenskow. All rights reserved.
//

#ifndef request_hpp
#define request_hpp

#include "../message.hpp"
#include "./head.hpp"
#include "../url.hpp"

using namespace fart::types;
using namespace fart::network::web;

namespace fart::network::web::http {
                
    class RequestHead : public Head {
        
    public:
        
        enum Method {
            HEAD = 0,
            GET,
            POST,
            PUT,
            DELETE
        };
        
        RequestHead() : _version(HTTP1_1), _method(GET), _path("/") {}
        
        RequestHead(Data<uint8_t>& data) : Head(data) {
                
            if (*(*_parts)[0] == "HEAD") _method = HEAD;
            else if (*(*_parts)[0] == "GET") _method = GET;
            else if (*(*_parts)[0] == "POST") _method = POST;
            else if (*(*_parts)[0] == "PUT") _method = PUT;
            else if (*(*_parts)[0] == "DELETE") _method = DELETE;
            else throw MethodNotSupportedException();
            
            _version = parseVersion((*_parts)[2]);
            _path = *Url::decode(*(*_parts)[1]);
            
        }
        
        RequestHead(const RequestHead& other) : _version(other._version), _method(other._method), _path(other._path) {}
        
        virtual ~RequestHead() {}
        
        const Version version() const {
            return _version;
        }
        
        const Method method() const {
            return _method;
        }
        
        const Strong<String> path() const {
            return Strong<String>(_path);
        }
        
    protected:
        
        virtual Strong<Data<uint8_t>> headData(const Data<uint8_t> &lineBreak) const {
            
            Strong<Data<uint8_t>> result = MessageHead::headData(lineBreak);
            
            switch (method()) {
               case HEAD:
                   result->append(String("HEAD").UTF8Data());
                   break;
               case GET:
                   result->append(String("GET").UTF8Data());
                   break;
               case POST:
                   result->append(String("POST").UTF8Data());
                   break;
               case PUT:
                   result->append(String("PUT").UTF8Data());
                   break;
               case DELETE:
                   result->append(String("DELETE").UTF8Data());
                   break;
            }
            
            result->append(' ');
            result->append(_path->UTF8Data());
            result->append(' ');
            result->append(Head::versionData(_version));
            result->append(lineBreak);
            
            return result;
            
        }

        
    private:
        
        Version _version;
        Method _method;
        Strong<String> _path;
        
        void ensureSpaceAt(Data<uint8_t>& data, size_t index) const noexcept(false) {
            if (data.count() < index) throw DataIncompleteException();
            if (data.itemAtIndex(index)) throw DataMalformedException();
        }
        
    };
    
    typedef Message<RequestHead> HTTPRequest;

}

#endif /* request_hpp */
