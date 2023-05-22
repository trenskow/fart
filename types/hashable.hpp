//
//  hashable.hpp
//  fart
//
//  Created by Kristian Trenskow on 2018/09/09.
//  Copyright © 2018-2023 Kristian Trenskow. All rights reserved.
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

				inline Builder& add(uint64_t value) {
					_hash = ((_hash << 5) + _hash) + (uint64_t)value;
					return *this;
				}

				inline Builder& add(double value) {
					return this->add(*(uint64_t*)(&value));
				}

				inline Builder& add(float value) {
					return this->add((double)value);
				}

				inline Builder& add(const Hashable& hashable) {
					return this->add(hashable.hash());
				}

				inline operator uint64_t() {
					return _hash;
				}

			private:

				uint64_t _hash;

			};

			Hashable() { }

			Hashable(const Hashable&) { }

			Hashable(Hashable&&) { }

			virtual uint64_t hash() const {
				return this->hashBuilder();
			};

			Hashable& operator=(const Hashable&) {
				return *this;
			}

			Hashable& operator=(Hashable&&) {
				return *this;
			}

		protected:

			virtual Builder hashBuilder() const {
				throw NotImplementedException();
			}

	};

}

#endif /* hashable_hpp */
