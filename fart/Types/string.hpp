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

namespace fart {
    
    using namespace memory;
    using namespace exceptions;
    
    namespace types {
        
        class String : public Type {
            
        private:
            Data<uint32_t> _store;
            
            Strong<Data<uint32_t>> _decodeUTF8(const char* buffer, size_t length) const throw(DecoderException);
            Strong<Data<uint32_t>> _decodeUTF8(const Data<uint8_t> &buffer) const;
            Strong<Data<uint8_t>> _encodeUTF8(const Data<uint32_t> &buffer) const;
            
        public:
            
            typedef enum {
                EncodingUTF8 = 0
            } Encoding;
            
            String();
            String(const char* string, const Encoding encoding) throw(DecoderException);
            String(const char* string) throw(DecoderException);
            String(const String& other);
            virtual ~String();
            
            size_t getLength() const;
            const char* getCString(Encoding encoding = EncodingUTF8) const;
            
            Strong<Data<uint8_t>> getData(Encoding encoding = EncodingUTF8) const;
            
            void append(const String& other);
            
            virtual const uint64_t getHash() const;
            
            virtual const Kind getKind() const;
            
            virtual bool operator==(const String& other) const;
            
            operator const char*() const;
            
        };
        
    }
}

#endif /* string_hpp */
