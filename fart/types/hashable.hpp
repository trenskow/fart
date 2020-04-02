//
//  hashable.hpp
//  fart
//
//  Created by Kristian Trenskow on 09/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef hashable_hpp
#define hashable_hpp

#include "../exceptions/exception.hpp"

using namespace fart::exceptions;

namespace fart::types {
    
    class Hashable {
        
    public:
        virtual const uint64_t getHash() const {
            throw NotImplementedException();
        };
        
    };
    
}

#endif /* hashable_hpp */
