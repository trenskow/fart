//
//  number.hpp
//  fart
//
//  Created by Kristian Trenskow on 09/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef number_hpp
#define number_hpp

#include "type.hpp"

namespace fart {
    
    namespace types {
        
        template<typename T>
        class Number : public Type {
            
        private:
            T _value;
            
        public:
            Number() : _value(0) {}
            Number(const T value) : _value(value) {}
            Number(const Number<T>& other) : _value(other._value) {}
            template<typename N>
            Number(const Number<N>& other) : _value(other.getValue()) {}
            virtual ~Number() {}
            
            const operator T() const {
                return _value;
            }
            
            template<typename N>
            const operator Number<N>() const {
                return Number<N>(_value);
            }
            
            const T getValue() const {
                return _value;
            }
            
            virtual const uint64_t getHash() const {
                return (uint64_t)_value;
            }
            
            virtual const Kind getKind() const {
                return Kind::number;
            }
            
            bool operator==(const Number<T>& other) const {
                if (!Type::operator==(other)) return false;
                return _value == other._value;
            }
            
        };
        
    }
    
}

#endif /* number_hpp */
