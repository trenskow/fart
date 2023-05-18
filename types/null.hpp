//
//  null.hpp
//  fart
//
//  Created by Kristian Trenskow on 2020/04/01.
//  Copyright © 2018-2023 Kristian Trenskow. All rights reserved.
//

#ifndef null_hpp
#define null_hpp

#include "type.hpp"

namespace fart::types {

	class Null: public Type {

	public:

		static Type::Kind typeKind() {
			return Type::Kind::null;
		}

		virtual Kind kind() const override {
			return Kind::null;
		}

		virtual bool operator==(const Type& other) const override {
			return other.kind() == Kind::null;
		}

	};

}

#endif /* null_hpp */
