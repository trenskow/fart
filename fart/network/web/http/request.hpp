//
//  request.hpp
//  fart
//
//  Created by Kristian Trenskow on 12/12/2019.
//  Copyright © 2019 Kristian Trenskow. All rights reserved.
//

#ifndef request_hpp
#define request_hpp

#include "../../../memory/object.hpp"
#include "../../../memory/strong.hpp"
#include "../../../types/data.hpp"
#include "../../../types/string.hpp"
#include "../message.hpp"
#include "./version.hpp"

using namespace fart::types;
using namespace fart::network::web;

namespace fart::network::web::http {
                
    class RequestHead : public Object {
        
    public:
        
        enum Method {
            HEAD = 0,
            GET,
            POST,
            PUT,
            DELETE
        };
        
        RequestHead(Data<uint8_t>& data);
        RequestHead(const RequestHead& other);
        
        virtual ~RequestHead();
        
        const Version getVersion() const;
        const Method getMethod() const;
        const Strong<String> getPath() const;
        
    private:
        
        Version _version;
        Method _method;
        String _path;
        
        void ensureSpaceAt(Data<uint8_t>& data, size_t index) const noexcept(false);
        
    };
    
    typedef Message<RequestHead> HTTPRequest;

}

#endif /* request_hpp */
