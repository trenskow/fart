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

using namespace fart::system;
using namespace fart::memory;
using namespace fart::exceptions;

namespace fart::types {

	class String : public Type {

	public:

		enum class Comparison {
			ComparisonCaseSensitive = 0,
			ComparisonCaseInsensitive
		};

		String() {
			_store.setComparitor(_caseComparitor);
			_caseComparitor->setComparison(Comparison::ComparisonCaseInsensitive);
		}

		String(const char* string) noexcept(false) : String() {
			if (string != nullptr) {
				_store.append(_decodeUTF8((const uint8_t*)string, strlen(string)));
			}
		}

		String(const Data<uint8_t>& data) noexcept(false) : String() {
			_store.append(_decodeUTF8(data.items(), data.count()));
		}

		String(const Data<uint16_t>& data, Endian::Variant endian) noexcept(false) : String() {
			_store.append(_decodeUTF16(data.items(), data.count(), endian));
		}

		String(const Data<uint16_t>& data) noexcept(false) {
			Strong<Data<uint16_t>> parseData(data);
			Endian::Variant endian = Endian::Variant::big;
			if (data.count() > 1) {
				Data<uint8_t> bom = data.subdata(0, 2)->as<uint8_t>();
				uint8_t b0 = bom[0];
				uint8_t b1 = bom[1];
				if ((b0 == 0xFF || b0 == 0xFE) && (b1 == 0xFF || b1 == 0xFE)) {
					if (b0 == 0xFE && b1 == 0xFF) endian = system::Endian::Variant::big;
					else if (b0 == 0xFF && b1 == 0xFE) endian = system::Endian::Variant::little;
					else throw DecoderException(0);
					parseData = parseData->subdata(2);
				}
			}
			_store.append(_decodeUTF16(parseData->items(), parseData->count(), endian));
		}

		String(const String& other) : String(other._store) {}

		template<typename F>
		static Strong<String> fromCString(const F& todo, size_t size = Data<uint32_t>::blockSize) {
			return Strong<String>(Data<uint8_t>::fromCBuffer([&todo](void* buffer, size_t length) {
				return todo((char*)buffer, length);
			}));
		}

		virtual ~String() {}

		static
		__attribute__ ((format (printf, 1, 0)))
		Strong<String> format(const char* format, ...) {

			va_list args;

			va_start(args, format);

			size_t size = vsnprintf(nullptr, 0, format, args) + 1;

			va_end(args);

			va_start(args, format);

			char buffer[size];

			vsnprintf(buffer, size, format, args);

			va_end(args);

			return Strong<String>((const char*)buffer);

		}

		void setComparison(Comparison comparison) {
			_caseComparitor->setComparison(comparison);
		}

		size_t length() const {
			return _store.count();
		}

		template<typename F>
		void withCString(const F& todo) const {
			auto data = this->UTF8Data(true);
			todo((const char*)data->items());
		}

		template<typename T, typename F>
		auto mapCString(const F& todo) const {
			auto data = this->UTF8Data(true);
			return todo((const char*)data->items());
		}

		void print(bool newLine = true) const {
			this->withCString([&newLine](const char* str) {
				newLine ? printf("%s\n", str) : printf("%s", str);
			});
		}

		Strong<Data<uint8_t>> UTF8Data(bool nullTerminate = false) const {
			return _encodeUTF8(_store, nullTerminate);
		}

		Strong<Data<uint16_t>> UTF16Data(Endian::Variant endian = Endian::Variant::big) const {
			return _encodeUTF16(_store, endian);
		}

		static Strong<String> fromHex(const Data<uint8_t>& data) {
			return Strong<String>(_decodeHex(data));
		}

		Strong<Data<uint8_t>> hexData() const {
			return _encodeHex(_store);
		}

		void append(const String& other) {
			_store.append(other._store);
		}

		void append(const uint32_t character) {
			_store.append(character);
		}

		void append(const char* string) {
			_store.append(_decodeUTF8((const uint8_t*)string, strlen(string)));
		}

		Strong<Array<String>> split(const char* separator, IncludeSeparator includeSeparator = IncludeSeparator::none, size_t max = 0) const {
			String sep(separator);
			return split(sep, includeSeparator, max);
		}

		Strong<Array<String>> split(String& separator, IncludeSeparator includeSeparator = IncludeSeparator::none, size_t max = 0) const {
			return _store.split(separator._store, includeSeparator, max)->map<String>([](const Data<uint32_t>& current) {
				return Strong<String>(current);
			});
		}

		Strong<Array<String>> split(const Array<String>& separators, IncludeSeparator includeSeparator = IncludeSeparator::none, size_t max = 0) const {
			auto stores = separators.map<Data<uint32_t>>([](const String& current) {
				return current._store;
			});
			return _store.split(stores, includeSeparator, max)->map<String>([](const Data<uint32_t>& current) {
				return String(current);
			});
		}

		static Strong<String> join(Array<String>& strings) {
			return Strong<String>(Data<uint32_t>::join(strings.map<Data<uint32_t>>([](const String& current) {
				return current._store;
			})));
		}

		static Strong<String> join(Array<String>& strings, const char* seperator) {
			String sep(seperator);
			return String::join(strings, sep);
		}

		static Strong<String> join(Array<String>& strings, String& separator) {
			return Strong<String>(Data<uint32_t>::join(strings.map<Data<uint32_t>>([](const String& current) {
				return current._store;
			}), separator._store));
		}

		double doubleValue(const size_t startIndex = 0, size_t* consumed = nullptr, bool allowLeadingZero = true) const {

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

				uint32_t chr = _store[idx];

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

		size_t indexOf(const String& other, size_t offset = 0) const {
			return this->_store.indexOf(other._store, offset);
		}

		size_t indexOf(const uint32_t chr) const {
			return this->_store.indexOf(chr);
		}

		Strong<String> substring(size_t offset, size_t length = NotFound) const {
			return Strong<String>(_store.subdata(offset, length));
		}

		virtual uint64_t hash() const override {
			return _store.hash();
		}

		virtual Kind kind() const override {
			return Kind::string;
		}

		virtual bool operator==(const String& other) const {
			if (!Type::operator==(other)) return false;
			return _store == other._store;
		}

		bool operator==(const char* other) const {
			return *this == String(other);
		}

		uint32_t operator[](size_t idx) const {
			return _store[idx];
		}

		void operator=(const String& other) {
			_store.drain();
			_store.append(other._store);
		}

	private:
		friend class Strong<String>;

		class CaseComparitor : public Data<uint32_t>::Comparitor {

		public:
			virtual ~CaseComparitor() {}

			virtual uint32_t transform(uint32_t value) const {
				switch (_comparison) {
					case Comparison::ComparisonCaseInsensitive:
						if (value >= 'A' && value <= 'Z') return value - ('A' - 'a');
						return value;
					default:
						return value;
				}
			}

			void setComparison(Comparison comparison) {
				_comparison = comparison;
			}

		private:

			Comparison _comparison;

		};

		enum class DoublePart {
			integer = 0,
			fraction,
			exponent
		};

		Data<uint32_t> _store;
		Strong<CaseComparitor> _caseComparitor;

		String(const Data<uint32_t>& storage) : String() {
			_store.append(storage);
		}

		static Strong<Data<uint32_t>> _decodeUTF8(const uint8_t* buffer, size_t length) noexcept(false) {

			size_t offset = 0;

			if (length > 2 && buffer[0] == 0xEF && buffer[1] == 0xBB && buffer[2] == 0xBF) {
				offset += 3;
			}

			Strong<Data<uint32_t>> ret;

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

				ret->append(codePoint);

			}

			return ret;

		}

		static Strong<Data<uint8_t>> _encodeUTF8(const Data<uint32_t> &buffer, bool nullTerminate = false) noexcept(false) {

			Strong<Data<uint8_t>> ret;

			for (size_t idx = 0 ; idx < buffer.count() ; idx++) {

				uint8_t chr[4];

				uint32_t codePoint = buffer.itemAtIndex(idx);
				if (codePoint < 0x80) {
					ret->append(codePoint);
				} else if (codePoint < 0x800) {
					uint8_t chr[2];
					chr[0] = 0xC0 | (codePoint >> 6);
					chr[1] = 0x80 | (codePoint & 0x3F);
					ret->append(chr, 2);
				} else if (codePoint <= 0xFFFF) {
					chr[0] = 0xE0 | (codePoint >> 12);
					chr[1] = 0x80 | ((codePoint >> 6) & 0x3F);
					chr[2] = 0x80 | (codePoint & 0x3F);
					ret->append(chr, 3);
				} else if (codePoint <= 0x1FFFFF) {
					chr[0] = 0xF0 | (codePoint >> 18);
					chr[1] = 0x80 | ((codePoint >> 12) & 0x3F);
					chr[2] = 0x80 | ((codePoint >> 6) & 0x3F);
					chr[3] = 0x80 | (codePoint & 0x3F);
					ret->append(chr, 4);
				}

			}

			if (nullTerminate) ret->append('\0');

			return ret;

		}

		static Strong<Data<uint32_t>> _decodeUTF16(const uint16_t* buffer, size_t length, Endian::Variant endian) noexcept(false) {

			Strong<Data<uint32_t>> ret;

			for (size_t idx = 0 ; idx < length ; idx++) {

				uint16_t chr = buffer[idx];

				if (chr < 0xD800 || chr >= 0xF000) ret->append(Endian::toSystemVariant(chr, endian));
				else {
					if (idx + 1 >= length) throw DecoderException(idx);
					uint32_t high = Endian::toSystemVariant(chr, endian);
					uint32_t low = Endian::toSystemVariant(buffer[++idx], endian);
					ret->append(((high - 0xD800) * 0x400) + (low - 0xDC00) + 0x10000);
				}

			}

			return ret;

		}

		static Strong<Data<uint16_t>> _encodeUTF16(const Data<uint32_t> &buffer, Endian::Variant endian) noexcept(false) {

			Strong<Data<uint16_t>> ret;

			for (size_t idx = 0 ; idx < buffer.count() ; idx++) {

				uint32_t chr = buffer[idx];

				if (chr < 0xD800) ret->append(Endian::fromSystemVariant((uint16_t)chr, endian));

				else if (chr < 0xF000 || chr > 0x10FFFF) throw EncoderException(idx);

				else {
					uint32_t tmp = chr - 0x10000;
					uint32_t high = tmp / 0x400 + 0xD800;
					uint32_t low = tmp % 0x400 + 0xDC00;
					ret->append(Endian::fromSystemVariant((uint16_t)high, endian));
					ret->append(Endian::fromSystemVariant((uint16_t)low, endian));
				}

			}

			return ret;

		}

		static uint8_t _valueFromHex(uint8_t chr, size_t idx) noexcept(false) {
			if (chr >= 'a' && chr <= 'f') return chr - 32;
			if (chr >= 'A' && chr <= 'F') return chr - 'A' + 10;
			else if (chr >= '0' && chr <= '9') return chr - '0';
			else throw DecoderException(idx);
		}

		static uint8_t _valueToHex(uint8_t value, size_t idx) noexcept(false) {
			if (value < 10) return 'A' + value;
			else if (value < 16) return '0' + (value - 10);
			else throw EncoderException(idx);
		}

		static Strong<Data<uint32_t>> _decodeHex(const Data<uint8_t> &buffer) noexcept(false) {

			if (buffer.count() % 2 != 0) throw OutOfBoundException(buffer.count() + 1);

			Strong<Data<uint32_t>> ret;

			for (size_t idx = 0 ; idx < buffer.count() ; idx += 2) {
				auto byte = buffer[idx];
				ret->append(_valueToHex(byte >> 4, idx));
				ret->append(_valueToHex(byte & 0xF, idx));
			}

			return ret;

		}

		static Strong<Data<uint8_t>> _encodeHex(const Data<uint32_t> &buffer) noexcept(false) {

			if (buffer.count() % 2 != 0) throw OutOfBoundException(buffer.count() + 1);

			Strong<Data<uint8_t>> ret;

			for (size_t idx = 0 ; idx < buffer.count() ; idx += 2) {
				ret->append(_valueFromHex(buffer[idx], idx) << 4 | _valueFromHex(buffer[idx + 1], idx + 1));
			}

			return ret;

		}

	};

}

#endif /* string_hpp */
