//
//  type.cpp
//  fart
//
//  Created by Kristian Trenskow on 10/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#include "../exceptions/exception.hpp"
#include "./type.hpp"

using namespace fart::exceptions;
using namespace fart::types;

const Type::Kind Type::getKind() const {
    throw NotImplementedException();
}

bool Type::operator==(const Type& other) const {
    if (this->getKind() != other.getKind()) return false;
    return this->getHash() == other.getHash();
}

bool Type::operator==(const Type* other) const {
    if (other == nullptr) return false;
    return *this == *other;
}

bool Type::operator!=(const Type& other) const {
    return !(*this == other);
}

bool Type::operator!=(const Type* other) const {
    return !(*this == other);
}
