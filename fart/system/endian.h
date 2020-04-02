//
//  endian.h
//  fart
//
//  Created by Kristian Trenskow on 01/04/2020.
//  Copyright © 2020 Kristian Trenskow. All rights reserved.
//

#ifndef endian_h
#define endian_h

#include <stdint.h>

static uint32_t _endianTest = 0x00000001;

namespace fart::system {
    
    class Endian {
        
    private:
        template<typename T>
        static const T swap(T val) {
            T ret;
            uint8_t* valP = (uint8_t *)&val;
            uint8_t* retP = (uint8_t *)&ret;
            for (size_t idx = 0 ; idx < sizeof(T) ; idx++) {
                retP[idx] = valP[sizeof(T) - idx - 1];
            }
            return ret;
        }
        
    public:
        
        enum Variant {
            little,
            big
        };
        
        static inline Variant getSystemVariant() {
            static const uint32_t test = 0x00000001;
            return *((uint8_t *)&test) == 0x01 ? Variant::little : Variant::big;
        }
        
        template<typename T>
        static inline const T convert(T val, Variant from, Variant to) {
            if (from == to) return val;
            return swap(val);
        }
        
        template<typename T>
        static const T toSystemVariant(T val, Variant from) {
            return convert(val, from, getSystemVariant());
        }
        
        template<typename T>
        static const T fromSystemVariant(T val, Variant to) {
            return convert(val, getSystemVariant(), to);
        }
        
    };
    
}

#endif /* endian_h */
