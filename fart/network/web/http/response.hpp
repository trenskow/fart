//
//  response.hpp
//  fart
//
//  Created by Kristian Trenskow on 12/12/2019.
//  Copyright © 2019 Kristian Trenskow. All rights reserved.
//

#ifndef response_hpp
#define response_hpp

#include "../../../memory/object.hpp"
#include "../../../types/data.hpp"
#include "../message.hpp"
#include "./version.hpp"

namespace fart::network::web::http {
    
    class ResponseHead : public Object {
        
    public:
        
        enum Status {
            OK = 200
        };
        
        ResponseHead(Data<uint8_t>& data);
        virtual ~ResponseHead();
        
    private:
        
        Version _version;
        Status _status;
        
    };
    
    typedef Message<ResponseHead> HTTPResponse;
    
}

#endif /* response_hpp */
