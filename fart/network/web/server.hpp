//
//  server.hpp
//  fart
//
//  Created by Kristian Trenskow on 11/12/2019.
//  Copyright © 2019 Kristian Trenskow. All rights reserved.
//

#ifndef server_hpp
#define server_hpp

#include "../../memory/object.hpp"

namespace fart::network::web {
    
    template<class Request, class Response>
    class Server : public Object {
        
    public:
        
        Server() {
            
        }
        
    };
    
}

#endif /* server_hpp */
