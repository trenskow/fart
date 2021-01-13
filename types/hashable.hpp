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

		class Builder {

		public:

			Builder() : _hash(5381) { }

			template<typename T>
			Builder& add(T value) {
				_hash = ((_hash << 5) + _hash) + (uint64_t)value;
				return *this;
			}

			operator uint64_t() {
				return _hash;
			}

		private:

			uint64_t _hash;

		};

		virtual uint64_t hash() const {
			throw NotImplementedException();
		};

	};

}

#endif /* hashable_hpp */
