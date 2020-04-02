//
//  json.hpp
//  fart
//
//  Created by Kristian Trenskow on 01/04/2020.
//  Copyright © 2020 Kristian Trenskow. All rights reserved.
//

#ifndef json_hpp
#define json_hpp

#include "../memory/object.hpp"
#include "../types/string.hpp"

using namespace fart::memory;
using namespace fart::types;

namespace fart::serialization {
    
    class JSON: Object {
        
    public:
        
        static Strong<Type> parse(const String& string);
        static Strong<String> stringify(const Type& data);
        
    };
    
}

#endif /* json_hpp */
