//
//  null.cpp
//  fart
//
//  Created by Kristian Trenskow on 01/04/2020.
//  Copyright © 2020 Kristian Trenskow. All rights reserved.
//

#include "null.hpp"

using namespace fart::types;

const Type::Kind Null::kind() const {
    return Kind::null;
}

bool Null::operator==(const Type& other) const {
    return other.kind() == Kind::null;
}

bool Null::operator==(const Type* other) const {
    if (other == nullptr) return true;
    return Type::operator==(other);
}
