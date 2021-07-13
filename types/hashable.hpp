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

			inline Builder() : _hash(5381) { }

			inline Builder& add(double value) {
				_hash = ((_hash << 5) + _hash) + *(uint64_t*)(&value);
				return *this;
			}

			inline Builder& add(float value) {
				return this->add((double)value);
			}

			template<typename T>
			inline Builder& add(const T& value) {
				_hash = ((_hash << 5) + _hash) + (uint64_t)value;
				return *this;
			}

			inline operator uint64_t() {
				return _hash;
			}

		private:

			uint64_t _hash;

		};

		virtual uint64_t hash() const {
			throw NotImplementedException();
		};

		Hashable& operator=(const Hashable& other) {
			return *this;
		}

		Hashable& operator=(Hashable&& other) {
			return *this;
		}

	protected:

		Hashable() { }
		Hashable(const Hashable& other) { }
		Hashable(Hashable&& other) { }

	};

}

#endif /* hashable_hpp */
