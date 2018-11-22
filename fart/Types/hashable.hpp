//
//  hashable.hpp
//  fart
//
//  Created by Kristian Trenskow on 09/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef hashable_hpp
#define hashable_hpp

#include <unistd.h>

namespace fart {
    
    namespace types {
        
        class Hashable {
            
        public:
            virtual const uint64_t getHash() const = 0;
            
        };
                
    }
    
}

#endif /* hashable_hpp */
