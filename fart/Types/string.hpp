//
//  string.hpp
//  fart
//
//  Created by Kristian Trenskow on 17/08/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef string_hpp
#define string_hpp

#include "data.hpp"
#include "strong.hpp"
#include "exception.hpp"
#include "type.hpp"
#include "array.hpp"

using namespace fart::memory;
using namespace fart::exceptions;

namespace fart::types {
    
    class String : public Type {
        
    private:
        Data<uint32_t> _store;
        
        String(const Data<uint32_t>& storage);
        
        Strong<Data<uint32_t>> _decodeUTF8(const char* buffer, size_t length) const noexcept(false);
        Strong<Data<uint32_t>> _decodeUTF8(const Data<uint8_t> &buffer) const;
        Strong<Data<uint8_t>> _encodeUTF8(const Data<uint32_t> &buffer) const;
        
        friend class Strong<String>;
        
    public:
        
        typedef enum {
            EncodingUTF8 = 0
        } Encoding;
        
        String();
        String(const char* string, const Encoding encoding = EncodingUTF8) noexcept(false);
        String(const Data<uint8_t>& buffer, const Encoding encoding = EncodingUTF8) noexcept(false);
        String(const String& other);
        virtual ~String();
        
        size_t getLength() const;
        const char* getCString(Encoding encoding = EncodingUTF8) const;
        
        Strong<Data<uint8_t>> getData(Encoding encoding = EncodingUTF8) const;
        
        void append(const String& other);
        
        Strong<Array<String>> split(const char *seperator, size_t max = 0) const;
        Strong<Array<String>> split(String& seperator, size_t max = 0) const;
        static Strong<String> join(Array<String>& strings);
        static Strong<String> join(Array<String>& strings, const char *seperator);
        static Strong<String> join(Array<String>& strings, String* seperator);
        static Strong<String> join(Array<String>& strings, String& seperator);

        virtual const uint64_t getHash() const;
        
        virtual const Kind getKind() const;
        
        virtual bool operator==(const String& other) const;
        
        operator const char*() const;
        
    };
    
}

#endif /* string_hpp */
