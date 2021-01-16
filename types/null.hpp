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
		virtual Kind kind() const override {
			return Kind::null;
		}

		virtual bool operator==(const Type& other) const override {
			return other.kind() == Kind::null;
		}

		virtual bool operator==(const Type* other) const override {
			if (other == nullptr) return true;
			return Type::operator==(other);
		}

	};

}

#endif /* null_hpp */
