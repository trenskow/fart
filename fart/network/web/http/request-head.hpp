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
        
        RequestHead();
        RequestHead(Data<uint8_t>& data);
        RequestHead(const RequestHead& other);
        
        virtual ~RequestHead();
        
        const Version version() const;
        const Method method() const;
        const Strong<String> path() const;
        
    protected:
        
        virtual Strong<Data<uint8_t>> headData(const Data<uint8_t> &lineBreak) const;
        
    private:
        
        Version _version;
        Method _method;
        Strong<String> _path;
        
        void ensureSpaceAt(Data<uint8_t>& data, size_t index) const noexcept(false);
        
    };
    
    typedef Message<RequestHead> HTTPRequest;

}

#endif /* request_hpp */
