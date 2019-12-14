//
//  http.hpp
//  fart
//
//  Created by Kristian Trenskow on 12/12/2019.
//  Copyright © 2019 Kristian Trenskow. All rights reserved.
//

#ifndef http_h
#define http_h

#include "./head.hpp"
#include "./request-head.hpp"
#include "./response-head.hpp"
#include "../server.hpp"

namespace fart::network::web::http {

    typedef Server<RequestHead, ResponseHead> HTTPServer;
    
}

#endif /* http_h */
