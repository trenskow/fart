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
#include "../system/endian.hpp"

using namespace fart::exceptions;
using namespace fart::system;

namespace fart::types {

	class Hashable {

		public:

			class Builder {

			public:

				inline Builder(
#if defined(FART_HASHABLE_ENDIAN_BIG)
					Endian::Variant endianess = Endian::Variant::big
#elif defined(FART_HASHABLE_ENDIAN_LITTLE)
					Endian::Variant endianess = Endian::Variant::little
#else
					Endian::Variant endianess = Endian::systemVariant()
#endif
				) : _hash(5381), _endianess(endianess) { }

				Builder(const Builder& other) : _hash(other._hash), _endianess(other._endianess) { }

				Builder(Builder&& other) : _hash(other._hash), _endianess(other._endianess) { }

				inline Builder& add(uint64_t value) {
					_hash = ((_hash << 5) + _hash) + (uint64_t)value;
					return *this;
				}

				inline Builder& add(double value) {
					return this->add(Endian::fromSystemVariant(*(uint64_t*)(&value), _endianess));
				}

				inline Builder& add(float value) {
					return this->add((double)value);
				}

				inline Builder& add(const Hashable& hashable) {
					return this->add(hashable.hash());
				}

				inline operator uint64_t() {
					return Endian::fromSystemVariant(_hash, _endianess);
				}

			private:

				uint64_t _hash;
				Endian::Variant _endianess;

			};

			Hashable() { }

			Hashable(const Hashable& other) { }

			Hashable(Hashable&& other) { }

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
