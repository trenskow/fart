//
//  tools.hpp
//  fart
//
//  Created by Kristian Trenskow on 05/04/2020.
//  Copyright © 2020 Kristian Trenskow. All rights reserved.
//

#ifndef tools_h
#define tools_h

#include <cstring>

#include "math.hpp"

namespace fart::tools {
    
    template<typename T>
    inline void zero(T* buffer) {
        bzero(buffer, sizeof(T));
    }
    
}

#endif /* tools_h */
