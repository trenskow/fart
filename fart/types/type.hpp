//
//  type.hpp
//  fart
//
//  Created by Kristian Trenskow on 10/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef type_hpp
#define type_hpp

#include <stdint.h>

#include "../memory/object.hpp"
#include "./hashable.hpp"

using namespace fart::memory;

namespace fart::types {
        
    class Type : public Object, public Hashable {
        
    public:
        
        enum class Kind : uint8_t {
            data,
            string,
            number,
            array,
            dictionary,
            date,
            null
        };
        
        virtual const Kind kind() const;
        
        virtual bool operator==(const Type& other) const;
        virtual bool operator==(const Type* other) const;
        bool operator!=(const Type& other) const;
        bool operator!=(const Type* other) const;
        
        template<class T>
        T& as() const {
            return (T&)*this;
        }
        
    };

}

#endif /* type_hpp */
