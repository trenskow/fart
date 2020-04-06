//
//  string.cpp
//  fart
//
//  Created by Kristian Trenskow on 17/08/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#include <cstdarg>
#include <cstdio>

#include "../exceptions/exception.hpp"
#include "./string.hpp"

using namespace fart::system;
using namespace fart::memory;
using namespace fart::types;
using namespace fart::exceptions::types;

String::String() {
    _store.setComparitor(_caseComparitor);
    _caseComparitor->setComparison(ComparisonCaseInsensitive);
}

String::String(const Data<uint32_t>& storage) : String() {
    _store.append(storage);
}

String::String(const char* string) noexcept(false) : String() {
    _store.append(_decodeUTF8((const uint8_t*)string, strlen(string)));
}

String::String(const Data<uint8_t>& data) noexcept(false) : String() {
    _store.append(_decodeUTF8(data.items(), data.count()));
}

String::String(const Data<uint16_t>& data, Endian::Variant endian) noexcept(false) : String() {
    _store.append(_decodeUTF16(data.items(), data.count(), endian));
}

String::String(const Data<uint16_t>& data) noexcept(false) : String() {
    Strong<Data<uint16_t>> parseData(data);
    Endian::Variant endian = Endian::Variant::big;
    if (data.count() > 1) {
        Data<uint8_t> bom = data.subdata(0, 2)->as<uint8_t>();
        uint8_t b0 = bom[0];
        uint8_t b1 = bom[1];
        if ((b0 == 0xFF || b0 == 0xFE) && (b1 == 0xFF || b1 == 0xFE)) {
            if (b0 == 0xFE && b1 == 0xFF) endian = system::Endian::big;
            else if (b0 == 0xFF && b1 == 0xFE) endian = system::Endian::little;
            else throw DecoderException(0);
            parseData = parseData->subdata(2);
        }
    }
    _store.append(_decodeUTF16(parseData->items(), parseData->count(), endian));
}

String::String(const String& other) : String(other._store) {}

String::~String() {}

Strong<Data<uint32_t>> String::_decodeUTF8(const uint8_t* buffer, size_t length) {
    
    Strong<Data<uint32_t>> ret;
    
    for (size_t idx = 0 ; idx < length ; idx++) {
        
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

Strong<Data<uint8_t>> String::_encodeUTF8(const Data<uint32_t> &buffer, bool nullTerminate) {
    
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

Strong<Data<uint32_t>> String::_decodeUTF16(const uint16_t *buffer, size_t length, Endian::Variant endian) {
    
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

Strong<Data<uint16_t>> String::_encodeUTF16(const Data<uint32_t> &buffer, Endian::Variant endian) {
    
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

const uint8_t String::_valueFromHex(uint8_t chr, size_t idx) {
    if (chr >= 'a' && chr <= 'f') return chr - 32;
    if (chr >= 'A' && chr <= 'F') return chr - 'A' + 10;
    else if (chr >= '0' && chr <= '9') return chr - '0';
    else throw DecoderException(idx);
}

const uint8_t String::_valueToHex(uint8_t value, size_t idx) {
    if (value < 10) return 'A' + value;
    else if (value < 16) return '0' + (value - 10);
    else throw EncoderException(idx);
}

Strong<Data<uint32_t>> String::_decodeHex(const Data<uint8_t> &buffer) {
    
    if (buffer.count() % 2 != 0) throw OutOfBoundException(buffer.count() + 1);
    
    Strong<Data<uint32_t>> ret;
    
    for (size_t idx = 0 ; idx < buffer.count() ; idx += 2) {
        auto byte = buffer[idx];
        ret->append(_valueToHex(byte >> 4, idx));
        ret->append(_valueToHex(byte & 0xF, idx));
    }
    
    return ret;
    
}

Strong<Data<uint8_t>> String::_encodeHex(const Data<uint32_t> &buffer) {
    
    if (buffer.count() % 2 != 0) throw OutOfBoundException(buffer.count() + 1);
    
    Strong<Data<uint8_t>> ret;
    
    for (size_t idx = 0 ; idx < buffer.count() ; idx += 2) {
        ret->append(_valueFromHex(buffer[idx], idx) << 4 | _valueFromHex(buffer[idx + 1], idx + 1));
    }
    
    return ret;
    
}

Strong<String> String::format(const char* format, ...) {
    
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

void String::setComparison(Comparison comparison) {
    _caseComparitor->setComparison(comparison);
}

size_t String::length() const {
    return _store.count();
}

Strong<Data<uint8_t>> String::UTF8Data(bool nullTerminate) const {
    return _encodeUTF8(_store, nullTerminate);
}

Strong<Data<uint16_t>> String::UTF16Data(Endian::Variant endian) const {
    return _encodeUTF16(_store, endian);
}

Strong<String> String::fromHex(const Data<uint8_t> &data) {
    return Strong<String>(_decodeHex(data));
}

Strong<Data<uint8_t>> String::hexData() const {
    return _encodeHex(_store);
}

void String::append(const String &other) {
    _store.append(other._store);
}

void String::append(const uint32_t character) {
    _store.append(character);
}

void String::append(const char *string) {
    _store.append(_decodeUTF8((const uint8_t*)string, strlen(string)));
}

Strong<Array<String>> String::split(const char *seperator, size_t max) const {
    String sep(seperator);
    return split(sep, max);
}

Strong<Array<String>> String::split(String &seperator, size_t max) const {
    return _store.split(seperator._store, max)->map<String>([](Data<uint32_t>& current) {
        return String(current);
    });
}

Strong<Array<String>> String::split(const Array<String>& separators, size_t max) const {
    auto stores = separators.map<Data<uint32_t>>([](const String& current) {
        return current._store;
    });
    return _store.split(stores, max)->map<String>([](Data<uint32_t>& current) {
        return String(current);
    });
}

Strong<String> String::join(Array<String>& strings) {
    return Strong<String>(Data<uint32_t>::join(strings.map<Data<uint32_t>>([](String& current) {
        return current._store;
    })));
}

Strong<String> String::join(Array<String>& strings, const char *seperator) {
    String sep(seperator);
    return String::join(strings, sep);
}

Strong<String> String::join(Array<String> &strings, String &seperator) {
    return Strong<String>(Data<uint32_t>::join(strings.map<Data<uint32_t>>([](String& current) {
        return current._store;
    }), seperator._store));
}

const int64_t String::toInteger(size_t startIndex, size_t* consumed) const {
    if (_store.count() <= startIndex) throw DecoderException(startIndex);
    if (_store[startIndex] != '-' && (_store[startIndex] < '0' || _store[startIndex] > '9')) throw DecoderException(startIndex);
    int64_t multiplier = 1;
    int64_t result = 0;
    size_t idx;
    for (idx = startIndex ; idx < length() ; idx++) {
        if (idx == startIndex && _store[idx] == '-') {
            multiplier = -1;
            continue;
        }
        if (idx == startIndex && _store[idx] == '+') continue;
        uint32_t character = _store[idx];
        if (character < '0' || character > '9') break;
        result = result * 10 + (character - '0');
    }
    if (consumed != nullptr) *consumed = idx - startIndex;
    return result * multiplier;
}

ssize_t String::indexOf(const String& other, size_t offset) const {
    return this->_store.indexOf(other._store, offset);
}

Strong<String> String::substring(size_t offset, ssize_t length) const {
    return Strong<String>(_store.subdata(offset, length));
}

const uint64_t String::hash() const {
    return _store.hash();
}

const Type::Kind String::kind() const {
    return Kind::string;
}

bool String::operator==(const String& other) const {
    if (!Type::operator==(other)) return false;
    return _store == other._store;
}

bool String::operator==(const char* other) const {
    return *this == String(other);
}

const uint32_t String::operator[](size_t idx) const {
    return _store[idx];
}

void String::operator=(const String &other) {
    _store.drain();
    _store.append(other._store);
}

String::CaseComparitor::~CaseComparitor() {}

const uint32_t String::CaseComparitor::transform(uint32_t value) const {
    switch (_comparison) {
        case ComparisonCaseInsensitive:
            if (value >= 'A' && value <= 'Z') return value - ('A' - 'a');
        default:
            return value;
    }
}

void String::CaseComparitor::setComparison(Comparison comparison) {
    _comparison = comparison;
}
