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

String::String(const char* string, const Encoding encoding) noexcept(false) : String() {
    switch (encoding) {
        case EncodingUTF8:
            _store.append(_decodeUTF8(string, strlen(string)));
            break;
    }
}

String::String(const Data<uint8_t>& data, const Encoding encoding) noexcept(false) : String() {
    switch (encoding) {
        case EncodingUTF8:
            _store.append(_decodeUTF8(data));
            break;
    }
}

String::String(const String& other) : String(other._store) {}

String::~String() {}

Strong<Data<uint32_t>> String::_decodeUTF8(const char* buffer, size_t length) const noexcept(false) {
    
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
        }
        
        ret->append(codePoint);
        
    }
    
    return ret;
    
}

Strong<Data<uint32_t>> String::_decodeUTF8(const Data<uint8_t> &buffer) const {
    return _decodeUTF8((const char *)buffer.getItems(), buffer.getCount());
}

Strong<Data<uint8_t>> String::_encodeUTF8(const Data<uint32_t> &buffer, bool nullTerminate) const {
    
    Strong<Data<uint8_t>> ret;
    
    for (size_t idx = 0 ; idx < buffer.getCount() ; idx++) {
        
        uint8_t chr[4];
        
        uint32_t codePoint = buffer.getItemAtIndex(idx);
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

size_t String::getLength() const {
    return _store.getCount();
}

const char* String::getCString(Encoding encoding) const {
    switch (encoding) {
        case EncodingUTF8:
            return (const char *)_encodeUTF8(_store, true)->getItems();
    }
}

Strong<Data<uint8_t>> String::getData(Encoding encoding) const {
    switch (encoding) {
        case EncodingUTF8:
            return _encodeUTF8(_store);
    }
}

void String::append(const String &other) {
    _store.append(other._store);
}

void String::append(const uint32_t character) {
    _store.append(character);
}

void String::append(const char *string, Encoding encoding) {
    switch (encoding) {
        case EncodingUTF8:
        _store.append(_decodeUTF8(Data<uint8_t>((const uint8_t*)string, strlen(string))));
    }
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

const int64_t String::parseNumber() const {
    if (_store.getCount() == 0) throw DecoderException(0);
    int64_t multiplier = 1;
    int64_t result = 0;
    for (size_t idx = 0 ; idx < getLength() ; idx++) {
        if (idx == 0 && _store[idx] == '-') {
            multiplier = -1;
            continue;
        }
        uint32_t character = _store[idx];
        if (character < '0' || character > '9') throw DecoderException(idx);
        result = result * 10 + (character - '0');
    }
    return result * multiplier;
}

const uint64_t String::getHash() const {
    return _store.getHash();
}

const Type::Kind String::getKind() const {
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

String::operator const char *() const {
    return this->getCString();
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
