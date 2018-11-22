//
//  dictionary.hpp
//  fart
//
//  Created by Kristian Trenskow on 09/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef dictionary_hpp
#define dictionary_hpp

#include "array.hpp"
#include "type.hpp"

namespace fart {
    
    namespace types {
        
        template<class Key, class Value>
        class Dictionary : public Type {
            
            static_assert(std::is_base_of<Object, Key>::value, "T must be a subclass of class Object.");
            static_assert(std::is_base_of<Hashable, Key>::value, "T must be a subclass of class Hashable.");
            static_assert(std::is_base_of<Object, Value>::value, "T must be a subclass of class Object.");

        private:
            Array<Key> _keys;
            Array<Value> _values;
            
        public:
            Dictionary() {}
            Dictionary(const Dictionary<Key,Value>& other) : _keys(other._keys), _values(other._values) {}
            virtual ~Dictionary() {}
            
            void set(const Key& key, Strong<Value> value) {
                _keys.append(Strong<String>(key));
                _values.append(value);
            }
            
            Strong<Value> get(const Key& key) const throw(KeyNotFoundException<Key>) {
                ssize_t keyIndex = _keys.indexOf(key);
                if (keyIndex == -1) {
                    throw KeyNotFoundException<Key>(key);
                }
                return _values[keyIndex];
            }
            
            const size_t getCount() const {
                return _keys.getCount();
            }
            
            const uint64_t getHash() const {
                return _keys.getHash() ^ _values.getHash();
            }
            
            virtual const Kind getKind() const {
                return Kind::dictionary;
            }
            
            bool operator==(const Dictionary<Key,Value>& other) const {
                if (!Type::operator==(other)) return false;
                if (_keys.getCount() != other._keys.getCount) return false;
                for (size_t idx = 0 ; idx < _keys.getCount() ; idx++) {
                    if (_keys[idx] != other._keys[idx] || _values[idx] != other._values[idx]) {
                        return false;
                    }
                }
                return true;
            }
                        
        };
        
    }
    
}

#endif /* dictionary_hpp */
