//
//  uuid.hpp
//  fart
//
//  Created by Kristian Trenskow on 28/08/2022.
//  Copyright © 2022 Kristian Trenskow. All rights reserved.
//

#ifndef uuid_hpp
#define uuid_hpp

#include "./type.hpp"
#include "../system/endian.hpp"

using namespace fart::exceptions::types;
using namespace fart::system;

namespace fart::types {

	class UUID : public Type {

	public:

		UUID() : Type() {
			this->_lower = ((uint64_t)arc4random() << 32) | arc4random();
			this->_upper = ((uint64_t)arc4random() << 32) | arc4random();
		}

		UUID(const UUID& other) : Type(other), _lower(other._lower), _upper(other._upper) { }

		UUID(const String& string) : Type() {

			String hexString = String::join(string.split("-"))->lowercased();

			if (hexString.length() != 32) throw UUIDMalformedException();

			this->_upper = this->_decode(hexString.substring(0, 16));
			this->_lower = this->_decode(hexString.substring(16));

		}

		virtual ~UUID() { }

		String string() {

			String all = this->_encode(this->_upper).appending(this->_encode(this->_lower));

			Array<String> result;

			result.append(all.substring(0, 8));
			result.append(all.substring(8, 4));
			result.append(all.substring(12, 4));
			result.append(all.substring(16, 4));
			result.append(all.substring(20));

			return String::join(result, "-");

		}

		virtual Kind kind() const override {
			return Kind::uuid;
		}

		virtual uint64_t hash() const override {
			return Hashable::Builder()
				.add(this->_upper)
				.add(this->_lower);
		}

		bool operator==(const UUID& other) const {
			return this->_lower == other._lower && this->_upper == other._upper;
		}

		bool operator!=(const UUID& other) const {
			return !(*this == other);
		}

		UUID& operator=(const UUID& other) {
			Type::operator=(other);
			this->_upper = other._upper;
			this->_lower = other._lower;
			return *this;
		}

	private:

		static inline String _base64Characters() {
			return "0123456789abcdef";
		}

		String _encode(uint64_t value) const {

			value = Endian::fromSystemVariant(value, Endian::Variant::big);

			String characters = UUID::_base64Characters();

			String result;

			for (size_t bits = 0 ; bits < sizeof(uint64_t) * 8 ; bits += 4) {
				result = String(characters[(value >> bits) & 0xF]).appending(result);
			}

			return result;

		}

		uint64_t _decode(const String& string) const noexcept(false) {

			uint64_t result = 0;

			String characters = UUID::_base64Characters();

			for (size_t idx = 0 ; idx < string.length() ; idx++) {
				size_t value = characters.indexOf(string[idx]);
				if (value == NotFound) throw UUIDMalformedException();
				result = (result << 4) | (value & 0xF);
			}

			return Endian::toSystemVariant(result, Endian::Variant::big);

		}

		uint64_t _lower;
		uint64_t _upper;

	};

}

#endif /* uuid_hpp */
