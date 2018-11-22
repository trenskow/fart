//
//  type.cpp
//  fart
//
//  Created by Kristian Trenskow on 10/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#include "type.hpp"

using namespace fart::types;

bool Type::operator==(const Type& other) const {
    if (this->getKind() != other.getKind()) return false;
    return this->getHash() == other.getHash();
}

bool Type::operator!=(const Type& other) const {
    return !(*this == other);
}
