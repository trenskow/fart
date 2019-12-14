//
//  string.hpp
//  fart
//
//  Created by Kristian Trenskow on 17/08/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef string_hpp
#define string_hpp

#include "../memory/strong.hpp"
#include "../exceptions/exception.hpp"
#include "./data.hpp"
#include "./type.hpp"
#include "./array.hpp"

using namespace fart::memory;
using namespace fart::exceptions;

namespace fart::types {
    
    class String : public Type {
        
    public:
        
        enum Encoding {
            EncodingUTF8 = 0
        };
        
        enum Comparison {
            ComparisonCaseSensitive = 0,
            ComparisonCaseInsensitive
        };
        
        String();
        String(const char* string, const Encoding encoding = EncodingUTF8) noexcept(false);
        String(const Data<uint8_t>& buffer, const Encoding encoding = EncodingUTF8) noexcept(false);
        String(const String& other);
        virtual ~String();
        
        static Strong<String> format(const char* format, ...);
        
        void setComparison(Comparison comparison);
        
        size_t getLength() const;
        const char* getCString(Encoding encoding = EncodingUTF8) const;
        
        Strong<Data<uint8_t>> getData(Encoding encoding = EncodingUTF8) const;
        
        void append(const String& other);
        void append(const uint32_t character);
        void append(const char* string, Encoding encoding = EncodingUTF8);
        
        Strong<Array<String>> split(const char *seperator, size_t max = 0) const;
        Strong<Array<String>> split(String& seperator, size_t max = 0) const;
        static Strong<String> join(Array<String>& strings);
        static Strong<String> join(Array<String>& strings, const char *seperator);
        static Strong<String> join(Array<String>& strings, String* seperator);
        static Strong<String> join(Array<String>& strings, String& seperator);
        
        const int64_t parseNumber() const;
        
        virtual const uint64_t getHash() const;
        
        virtual const Kind getKind() const;
        
        virtual bool operator==(const String& other) const;
        bool operator==(const char* other) const;
        const uint32_t operator[](size_t idx) const;
        void operator=(const String& other);
        
        operator const char*() const;
        
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
        
        Strong<Data<uint32_t>> _decodeUTF8(const char* buffer, size_t length) const noexcept(false);
        Strong<Data<uint32_t>> _decodeUTF8(const Data<uint8_t> &buffer) const;
        Strong<Data<uint8_t>> _encodeUTF8(const Data<uint32_t> &buffer, bool nullTerminate = false) const;
            
    };
    
}

#endif /* string_hpp */
