//
//  string.hpp
//  fart
//
//  Created by Kristian Trenskow on 17/08/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef string_hpp
#define string_hpp

#include <cstdio>

#include "../system/endian.h"
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
                
        enum Comparison {
            ComparisonCaseSensitive = 0,
            ComparisonCaseInsensitive
        };
        
        String();
        String(const char* string) noexcept(false);
        String(const Data<uint8_t>& buffer) noexcept(false);
        String(const Data<uint16_t>& buffer, Endian::Variant endian) noexcept(false);
        String(const Data<uint16_t>& buffer) noexcept(false);
        String(const String& other);
        
        template<typename F>
        static Strong<String> fromCString(const F& todo, size_t size = Data<uint32_t>::blockSize) {
            return Strong<String>(Data<uint8_t>::fromCBuffer([&todo,&size](void* buffer, size_t length) {
                return todo((char*)buffer, length);
            }));
        }
        
        virtual ~String();
        
        static
        __attribute__ ((format (printf, 1, 0)))
        Strong<String> format(const char* format, ...);
        
        void setComparison(Comparison comparison);
        
        size_t length() const;
        
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
        
        Strong<Data<uint8_t>> UTF8Data(bool nullTerminate = false) const;
        Strong<Data<uint16_t>> UTF16Data(Endian::Variant endian = Endian::Variant::big) const;
        
        static Strong<String> fromHex(const Data<uint8_t>& data);
        Strong<Data<uint8_t>> hexData() const;
        
        void append(const String& other);
        void append(const uint32_t character);
        void append(const char* string);
        
        Strong<Array<String>> split(const char *separator, size_t max = 0) const;
        Strong<Array<String>> split(String& separator, size_t max = 0) const;
        Strong<Array<String>> split(const Array<String>& separators, size_t max = 0) const;
        static Strong<String> join(Array<String>& strings);
        static Strong<String> join(Array<String>& strings, const char *seperator);
        static Strong<String> join(Array<String>& strings, String* seperator);
        static Strong<String> join(Array<String>& strings, String& seperator);
        
        const int64_t toInteger(size_t startIndex = 0, size_t* consumed = nullptr) const;
        
        ssize_t indexOf(const String& other, size_t offset = 0) const;
        Strong<String> substring(size_t offset, ssize_t length = -1) const;
                
        virtual const uint64_t hash() const override;
        
        virtual const Kind kind() const override;
        
        virtual bool operator==(const String& other) const;
        bool operator==(const char* other) const;
        const uint32_t operator[](size_t idx) const;
        void operator=(const String& other);
                
    private:
        friend class Strong<String>;
        
        class CaseComparitor : public Data<uint32_t>::Comparitor {
            
        public:
            virtual ~CaseComparitor();
            
            virtual const uint32_t transform(uint32_t value) const;
            
            void setComparison(Comparison comparison);
            
        private:
            
            Comparison _comparison;
            
        };
        
        Data<uint32_t> _store;
        Strong<CaseComparitor> _caseComparitor;
        
        String(const Data<uint32_t>& storage);
        
        static Strong<Data<uint32_t>> _decodeUTF8(const uint8_t* buffer, size_t length) noexcept(false);
        static Strong<Data<uint8_t>> _encodeUTF8(const Data<uint32_t> &buffer, bool nullTerminate = false) noexcept(false);
        
        static Strong<Data<uint32_t>> _decodeUTF16(const uint16_t* buffer, size_t length, Endian::Variant endian) noexcept(false);
        static Strong<Data<uint16_t>> _encodeUTF16(const Data<uint32_t> &buffer, Endian::Variant endian) noexcept(false);
        
        static const uint8_t _valueFromHex(uint8_t chr, size_t idx) noexcept(false);
        static const uint8_t _valueToHex(uint8_t value, size_t idx) noexcept(false);
        static Strong<Data<uint32_t>> _decodeHex(const Data<uint8_t> &buffer) noexcept(false);
        static Strong<Data<uint8_t>> _encodeHex(const Data<uint32_t> &buffer) noexcept(false);

    };
    
}

#endif /* string_hpp */
