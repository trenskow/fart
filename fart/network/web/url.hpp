//
//  url.hpp
//  fart
//
//  Created by Kristian Trenskow on 12/12/2019.
//  Copyright © 2019 Kristian Trenskow. All rights reserved.
//

#ifndef url_hpp
#define url_hpp

#include "../../memory/object.hpp"
#include "../../memory/strong.hpp"
#include "../../types/string.hpp"

using namespace fart::types;
using namespace fart::memory;

namespace fart::network::web {
    
    class Url : public Object {
        
    private:
        static const uint8_t hexValue(uint32_t value) noexcept(false);
        
    public:
        
        static Strong<String> decode(const String& url);
        
    };
    
}

#endif /* url_hpp */
