//
//  string.hpp
//  fart
//
//  Created by Kristian Trenskow on 17/08/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef string_hpp
#define string_hpp

#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#include "../system/endian.hpp"
#include "../memory/strong.hpp"
#include "../exceptions/exception.hpp"
#include "./data.hpp"
#include "./type.hpp"
#include "./array.hpp"
#include "./unicode.hpp"

using namespace fart::system;
using namespace fart::memory;
using namespace fart::exceptions;

namespace fart::types {

	static const uint8_t _bigEndianBOM[] = { 0xFE, 0xFF };
	static const uint8_t _littleEndianBOM[] = { 0xFF, 0xFE };

	static const Data<uint8_t> bigEndianBOM(_bigEndianBOM, 2);
	static const Data<uint8_t> littleEndianBOM(_littleEndianBOM, 2);

	class String : public Type {

	public:

		String() : String(Data<uint32_t>()) {}

		String(const char* string) noexcept(false) : String() {
			if (string != nullptr) {
				_storage.append(_decodeUTF8((const uint8_t*)string, strlen(string)));
			}
		}

		String(const Data<uint8_t>& data) noexcept(false) : String() {
			_storage.append(_decodeUTF8(data.items(), data.length()));
		}

		String(const Data<uint16_t>& data, const Endian::Variant& endian) noexcept(false) : String() {
			_storage.append(_decodeUTF16(data.items(), data.length(), endian));
		}

		String(const Data<uint16_t>& data) noexcept(false) : String() {
			Strong<Data<uint16_t>> parseData(data);
			Endian::Variant endian = Endian::Variant::big;
			if (data.length() > 1) {
				Strong<Data<uint8_t>> potentialMarker = data.subdata(0, 2)->as<uint8_t>();
				if (*potentialMarker == bigEndianBOM || *potentialMarker == littleEndianBOM) {
					endian = *potentialMarker == bigEndianBOM ? Endian::Variant::big : Endian::Variant::little;
					parseData = parseData->subdata(2);
				}
			}
			_storage.append(_decodeUTF16(parseData->items(), parseData->length(), endian));
		}

		String(const Data<uint32_t>& store) : _storage(store) {}

		String(const uint32_t character) : _storage(&character, 1) {}

		String(const String& other) : String(other._storage) {}

		String(String&& other) : _storage(std::move(other._storage)) { }

		template<typename F>
		inline static String fromCString(const F& todo, const size_t& size) {
			return String(Data<uint8_t>::fromCBuffer([&todo](void* buffer, size_t length) {
				return todo((char*)buffer, length);
			}, size));
		}

		virtual ~String() {}

		static
		__attribute__ ((format (printf, 1, 0)))
		String format(const char* format, ...) {

			va_list args;

			va_start(args, format);

			size_t size = vsnprintf(nullptr, 0, format, args) + 1;

			va_end(args);

			va_start(args, format);

			char buffer[size];

			vsnprintf(buffer, size, format, args);

			va_end(args);

			return String((const char*)buffer);

		}

		inline size_t length() const {
			return _storage.length();
		}

		template<typename T, typename F>
		inline auto mapCString(const F& todo) const {
			return todo((const char*)this->UTF8Data(true)->items());
		}

		template<typename F>
		inline void withCString(const F& todo) const {
			todo((const char*)this->UTF8Data(true)->items());
		}

		inline void print(bool newLine = true) const {
			this->appending(newLine ? "\n" : "")->withCString(printf);
		}

		inline Strong<Data<uint8_t>> UTF8Data(const bool& nullTerminate = false) const {
			return _encodeUTF8(_storage, nullTerminate);
		}

		inline Strong<Data<uint16_t>> UTF16Data(const Endian::Variant& endian = Endian::systemVariant(), const bool& includeBOM = false) const {
			return _encodeUTF16(_storage, endian, includeBOM);
		}

		inline Strong<Data<uint32_t>> UTF32Data() const {
			return Strong<Data<uint32_t>>(this->_storage);
		}

		inline static Strong<String> fromHex(const Data<uint8_t>& data) {
			return Strong<String>(_decodeHex(data));
		}

		inline Strong<Data<uint8_t>> hexData() const {
			return _encodeHex(_storage);
		}

		inline void append(const String& other) {
			_storage.append(other._storage);
		}

		inline void append(const uint32_t& character) {
			_storage.append(character);
		}

		inline Strong<String> appending(const String& other) const {
			return String(this->_storage.appending(other._storage));
		}

		inline Strong<Array<String>> split() const {
			return _storage.split()->map<String>([](Data<uint32_t>& data) {
				return Strong<String>(data);
			});
		}

		inline Strong<Array<String>> split(const String& separator, const IncludeSeparator& includeSeparator = IncludeSeparator::none, const size_t& max = 0) const {
			return _storage.split(separator._storage, includeSeparator, max)->map<String>([](const Data<uint32_t>& current) {
				return Strong<String>(current);
			});
		}

		Strong<Array<String>> split(const Array<String>& separators, const IncludeSeparator& includeSeparator = IncludeSeparator::none, const size_t& max = 0) const {
			auto stores = separators.map<Data<uint32_t>>([](const String& current) {
				return current._storage;
			});
			return _storage.split(stores, includeSeparator, max)->map<String>([](const Data<uint32_t>& current) {
				return Strong<String>(current);
			});
		}

		inline static Strong<String> join(const Array<String>& strings) {
			return Strong<String>(Data<uint32_t>::join(strings.map<Data<uint32_t>>([](const String& current) {
				return current._storage;
			})));
		}

		inline static Strong<String> join(const Array<String>& strings, const String& separator) {
			return Strong<String>(Data<uint32_t>::join(strings.map<Data<uint32_t>>([](const String& current) {
				return current._storage;
			}), separator._storage));
		}

		double doubleValue(const size_t& startIndex = 0, size_t* consumed = nullptr, bool allowLeadingZero = true) const {

			double integerMultiplier = 1;
			bool integerMultiplierParsed = false;
			bool integerHadLeadingZero = false;
			bool integerHadDigits = false;
			size_t integerStartIndex = NotFound;
			double fractionMultiplier = 10;
			bool fractionHadDigits = false;
			size_t fractionStartIndex = NotFound;
			double exponentMultiplier = 1;
			bool exponentMultiplierParsed = false;
			bool exponentHadDigits = false;
			size_t exponentStartIndex = NotFound;

			double integer = 0;
			double fraction = 0;
			double exponent = 0;

			size_t idx = startIndex;
			while (idx < length()) {

				uint32_t chr = _storage[idx];

				if (chr == '+' || chr == '-') {

					if (fractionStartIndex != NotFound && exponentStartIndex == NotFound) throw DecoderException(idx);
					if (exponentStartIndex == NotFound && integerMultiplierParsed) throw DecoderException(idx);
					if (exponentStartIndex != NotFound && exponentMultiplierParsed) throw DecoderException(idx);

					if (exponentStartIndex != NotFound) {
						if (exponentHadDigits) throw DecoderException(idx);
						exponentMultiplier = chr == '+' ? 1 : -1;
						exponentMultiplierParsed = true;
					}
					else {
						if (integerHadDigits) throw DecoderException(idx);
						integerMultiplier = chr == '+' ? 1 : -1;
						integerMultiplierParsed = true;
					}

				}

				else if (chr == '.') {
					if (!integerHadDigits) throw DecoderException(idx);
					if (exponentStartIndex != NotFound || fractionStartIndex != NotFound) throw DecoderException(idx);
					fractionStartIndex = idx;
				}

				else if (chr == 'e' || chr == 'E') {
					if (exponentStartIndex != NotFound) throw DecoderException(idx);
					exponentStartIndex = idx;
				}

				else if (chr >= '0' && chr <= '9') {

					if (exponentStartIndex != NotFound && !exponentHadDigits) exponentHadDigits = true;
					else if (fractionStartIndex != NotFound && !fractionHadDigits) fractionHadDigits = true;
					else if (!integerHadDigits) {
						integerHadDigits = true;
						integerHadLeadingZero = chr == '0';
					}

					double value = chr - '0';

					if (exponentStartIndex != NotFound) {
						exponent = exponent * 10 + value;
					} else if (fractionStartIndex != NotFound) {
						fraction = fraction + (value / fractionMultiplier);
						fractionMultiplier *= 10;
					} else {
						integer = integer * 10 + value;
					}

				} else {
					break;
				}

				idx++;

			}

			if (consumed != nullptr) *consumed = idx - startIndex;

			if (exponentStartIndex != NotFound && !exponentHadDigits) throw DecoderException(exponentStartIndex);
			if (fractionStartIndex != NotFound && !fractionHadDigits) throw DecoderException(fractionStartIndex);
			if (!integerHadDigits) throw DecoderException(startIndex);

			double result = ((integer * integerMultiplier) + fraction) * (pow(10, exponent) * exponentMultiplier);

			if (!allowLeadingZero && integer != 0 && integerHadLeadingZero) throw DecoderException(integerStartIndex);

			return result;

		}

		inline size_t indexOf(const String& other, const size_t& offset = 0) const {
			return this->_storage.indexOf(other._storage, offset);
		}

		inline size_t indexOf(const uint32_t& chr) const {
			return this->_storage.indexOf(chr);
		}

		inline Strong<String> substring(const size_t& offset, const size_t& length = NotFound) const {
			return Strong<String>(_storage.subdata(offset, length));
		}

		inline Strong<String> uppercased() const {
			return Strong<String>(this->_storage.map<uint32_t>(Unicode::lowerToUpper));
		}

		inline Strong<String> lowercased() const {
			return String(this->_storage.map<uint32_t>(Unicode::upperToLower));
		}

		inline Strong<String> capitalized() const {
			return this->substring(0, 1)->uppercased()->appending(this->substring(1)->lowercased());
		}

		virtual uint64_t hash() const override {
			return _storage.hash();
		}

		virtual Kind kind() const override {
			return Kind::string;
		}

		virtual bool operator==(const String& other) const {
			if (!Type::operator==(other)) return false;
			return _storage == other._storage;
		}

		inline bool operator==(const char* other) const {
			return *this == String(other);
		}

		inline uint32_t operator[](const size_t& idx) const {
			return _storage[idx];
		}

		String& operator=(const String& other) {
			Type::operator=(other);
			_storage = other._storage;
			return *this;
		}

		String& operator=(String&& other) {
			Type::operator=(std::move(other));
			_storage = std::move(other._storage);
			return *this;
		}

	private:

		enum class DoublePart {
			integer = 0,
			fraction,
			exponent
		};

		Data<uint32_t>::Value _storage;

		static Data<uint32_t> _decodeUTF8(const uint8_t* buffer, const size_t& length) noexcept(false) {

			size_t offset = 0;

			if (length > 2 && buffer[0] == 0xEF && buffer[1] == 0xBB && buffer[2] == 0xBF) {
				offset += 3;
			}

			Data<uint32_t> ret;

			for (size_t idx = offset ; idx < length ; idx++) {

				const uint8_t firstByte = buffer[0];
				uint32_t codePoint = 0;
				uint8_t charWidth = 1;

				buffer++;

				if ((firstByte & 0x80) == 0) {
					codePoint = firstByte & 0x7F;
				} else if ((firstByte & 0xE0) == 0xC0) {
					charWidth = 2;
					codePoint = firstByte & 0x1F;
				} else if ((firstByte & 0xF0) == 0xE0) {
					charWidth = 3;
					codePoint = firstByte & 0x0F;
				} else if ((firstByte & 0xF8) == 0xF0) {
					charWidth = 4;
					codePoint = firstByte & 0x07;
				} else { // Malformed.
					throw DecoderException(idx);
				}

				// Malformed.
				if (length < charWidth) {
					throw DecoderException(idx);
				}

				for (int64_t i = 1 ; i < charWidth ; i++) {
					if ((buffer[0] & 0xC0) == 0x80) {
						codePoint = (codePoint << 6) | (buffer[0] & 0x3F);
					} else {
						// Malformed.
						throw DecoderException(idx);
					}
					buffer++;
					idx++;
				}

				ret.append(codePoint);

			}

			return ret;

		}

		static Data<uint8_t> _encodeUTF8(const Data<uint32_t> &buffer, const bool& nullTerminate = false) noexcept(false) {

			Data<uint8_t> ret;

			for (size_t idx = 0 ; idx < buffer.length() ; idx++) {

				uint8_t chr[4];

				uint32_t codePoint = buffer[idx];
				if (codePoint < 0x80) {
					ret.append(codePoint);
				} else if (codePoint < 0x800) {
					uint8_t chr[2];
					chr[0] = 0xC0 | (codePoint >> 6);
					chr[1] = 0x80 | (codePoint & 0x3F);
					ret.append(chr, 2);
				} else if (codePoint <= 0xFFFF) {
					chr[0] = 0xE0 | (codePoint >> 12);
					chr[1] = 0x80 | ((codePoint >> 6) & 0x3F);
					chr[2] = 0x80 | (codePoint & 0x3F);
					ret.append(chr, 3);
				} else if (codePoint <= 0x1FFFFF) {
					chr[0] = 0xF0 | (codePoint >> 18);
					chr[1] = 0x80 | ((codePoint >> 12) & 0x3F);
					chr[2] = 0x80 | ((codePoint >> 6) & 0x3F);
					chr[3] = 0x80 | (codePoint & 0x3F);
					ret.append(chr, 4);
				}

			}

			if (nullTerminate) ret.append('\0');

			return ret;

		}

		static Data<uint32_t> _decodeUTF16(const uint16_t* buffer, const size_t& length, const Endian::Variant& endian) noexcept(false) {

			Data<uint32_t> ret;

			for (size_t idx = 0 ; idx < length ; idx++) {

				uint16_t chr = buffer[idx];

				if (chr < 0xD800 || chr >= 0xF000) ret.append(Endian::toSystemVariant(chr, endian));
				else {
					if (idx + 1 >= length) throw DecoderException(idx);
					uint32_t high = Endian::toSystemVariant(chr, endian);
					uint32_t low = Endian::toSystemVariant(buffer[++idx], endian);
					ret.append(((high - 0xD800) * 0x400) + (low - 0xDC00) + 0x10000);
				}

			}

			return ret;

		}

		static Data<uint16_t> _encodeUTF16(const Data<uint32_t> &buffer, const Endian::Variant& endian, const bool& includeBOM) noexcept(false) {

			Data<uint16_t> ret;

			if (includeBOM) {
				ret.append((endian == Endian::Variant::big ? bigEndianBOM : littleEndianBOM).as<uint16_t>());
			}

			for (size_t idx = 0 ; idx < buffer.length() ; idx++) {

				uint32_t chr = buffer[idx];

				if (chr < 0xD800) ret.append(Endian::fromSystemVariant((uint16_t)chr, endian));

				else if (chr < 0xF000 || chr > 0x10FFFF) throw EncoderException(idx);

				else {
					uint32_t tmp = chr - 0x10000;
					uint32_t high = tmp / 0x400 + 0xD800;
					uint32_t low = tmp % 0x400 + 0xDC00;
					ret.append(Endian::fromSystemVariant((uint16_t)high, endian));
					ret.append(Endian::fromSystemVariant((uint16_t)low, endian));
				}

			}

			return ret;

		}

		inline static Data<uint32_t> _decodeUTF32(const Data<uint32_t>& buffer, const Endian::Variant& endian) {
			return buffer.map<uint32_t>([&endian](uint32_t character) {
				return Endian::convert(character, endian, Endian::systemVariant());
			});
		}

		inline static Data<uint32_t> _encodeUTF32(const Data<uint32_t>& buffer, const Endian::Variant& endian) {
			return buffer.map<uint32_t>([&endian](uint32_t character) {
				return Endian::convert(character, Endian::systemVariant(), endian);
			});
		}

		static uint8_t _valueFromHex(const uint8_t& chr, const size_t& idx) noexcept(false) {
			if (chr >= 'a' && chr <= 'f') return chr - 32;
			if (chr >= 'A' && chr <= 'F') return chr - 'A' + 10;
			else if (chr >= '0' && chr <= '9') return chr - '0';
			else throw DecoderException(idx);
		}

		static uint8_t _valueToHex(const uint8_t& value, const size_t& idx) noexcept(false) {
			if (value < 10) return 'A' + value;
			else if (value < 16) return '0' + (value - 10);
			else throw EncoderException(idx);
		}

		static Data<uint32_t> _decodeHex(const Data<uint8_t> &buffer) noexcept(false) {

			if (buffer.length() % 2 != 0) throw OutOfBoundException(buffer.length() + 1);

			Data<uint32_t> ret;

			for (size_t idx = 0 ; idx < buffer.length() ; idx += 2) {
				auto byte = buffer[idx];
				ret.append(_valueToHex(byte >> 4, idx));
				ret.append(_valueToHex(byte & 0xF, idx));
			}

			return ret;

		}

		static Data<uint8_t> _encodeHex(const Data<uint32_t> &buffer) noexcept(false) {

			if (buffer.length() % 2 != 0) throw OutOfBoundException(buffer.length() + 1);

			Data<uint8_t> ret;

			for (size_t idx = 0 ; idx < buffer.length() ; idx += 2) {
				ret.append(_valueFromHex(buffer[idx], idx) << 4 | _valueFromHex(buffer[idx + 1], idx + 1));
			}

			return ret;

		}

	};

}

#endif /* string_hpp */
