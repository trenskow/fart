//
//  null.hpp
//  fart
//
//  Created by Kristian Trenskow on 01/04/2020.
//  Copyright © 2020 Kristian Trenskow. All rights reserved.
//

#ifndef null_hpp
#define null_hpp

#include "type.hpp"

namespace fart::types {

    class Null: public Type {
        
    public:
        virtual const Kind getKind() const;
        
        virtual bool operator==(const Type& other) const;
        virtual bool operator==(const Type* other) const;
        
    };
    
}

#endif /* null_hpp */
