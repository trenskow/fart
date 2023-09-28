//
// uuid.hpp
// fart
//
// Created by Kristian Trenskow on 2022/08/28.
// See license in LICENSE.
//

#ifndef uuid_hpp
#define uuid_hpp

#include <stdlib.h>
#include "./type.hpp"
#include "../system/endian.hpp"

#if defined(__APPLE__) || defined(BSD)
#define RANDOM() arc4random()
#else
#define RANDOM() random()
#endif

using namespace fart::exceptions::types;
using namespace fart::system;

namespace fart::types {

	class UUID : public Type {

	public:

		static Type::Kind typeKind() {
			return Type::Kind::uuid;
		}

		UUID() : Type() {

			uint64_t upper = ((uint64_t)RANDOM() << 32) | RANDOM();
			uint64_t lower = ((uint64_t)RANDOM() << 32) | RANDOM();

			uint8_t* upperBytes = (uint8_t*)&upper;
			uint8_t* lowerBytes = (uint8_t*)&lower;

			upperBytes[6] = (upperBytes[6] & 0x0f) | 0x40;
			lowerBytes[0] = (lowerBytes[0] & 0x3f) | 0x80;

			this->_upper = Endian::toSystemVariant(upper, Endian::Variant::big);
			this->_lower = Endian::toSystemVariant(lower, Endian::Variant::big);

		}

		UUID(const UUID& other) : Type(), _lower(other._lower), _upper(other._upper) { }

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

		inline static String _base64Characters() {
			return "0123456789abcdef";
		}

		String _encode(uint64_t value) const {

			value = Endian::fromSystemVariant(value, Endian::Variant::big);

			String characters = UUID::_base64Characters();

			String result;

			uint8_t* bytes = (uint8_t*)&value;

			for (size_t idx = 0 ; idx < sizeof(uint64_t) ; idx++) {
				result = result.appending(String::format("%02x", bytes[idx]));
			}

			return result;

		}

		uint64_t _decode(const String& string) const noexcept(false) {

			String characters = UUID::_base64Characters();

			uint64_t result = 0;

			uint8_t* bytes = (uint8_t*)&result;

			for (size_t idx = 0 ; idx < sizeof(uint64_t) ; idx++) {

				size_t upperIndex = characters.indexOf(string[idx * 2]);
				size_t lowerIndex = characters.indexOf(string[idx * 2 + 1]);

				if (upperIndex == NotFound || lowerIndex == NotFound) {
					throw UUIDMalformedException();
				}

				bytes[idx] = (upperIndex << 4) | lowerIndex;

			}

			return Endian::toSystemVariant(result, Endian::Variant::big);

		}

		uint64_t _lower;
		uint64_t _upper;

	};

}

#endif /* uuid_hpp */
