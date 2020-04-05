//
//  dictionary.hpp
//  fart
//
//  Created by Kristian Trenskow on 09/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef dictionary_hpp
#define dictionary_hpp

#include <thread>
#include "./array.hpp"
#include "./type.hpp"

namespace fart::types {
            
    template<class Key, class Value>
    class Dictionary : public Type {
        
        static_assert(std::is_base_of<Object, Key>::value);
        static_assert(std::is_base_of<Hashable, Key>::value);
        static_assert(std::is_base_of<Object, Value>::value);

    private:
        Array<Key> _keys;
        Array<Value> _values;
        
    public:
        Dictionary() {}
        Dictionary(const Dictionary<Key,Value>& other) : _keys(other._keys), _values(other._values) {}
        virtual ~Dictionary() {}
        
        void set(const Key& key, Strong<Value> value) {
            ssize_t index = _keys.indexOf(key);
            if (index > -1) {
                _keys.replace(key, index);
                _values.replace(value, index);
            } else {
                _keys.append(key);
                _values.append(value);
            }
        }
        
        Strong<Array<Key>> getKeys() const {
            return Strong<Array<Key>>(_keys);
        }
        
        const bool hasKey(const Key& key) const {
            return _keys.indexOf(key) > -1;
        }
        
        Strong<Value> get(const Key& key) const noexcept(false) {
            ssize_t keyIndex = _keys.indexOf(key);
            if (keyIndex == -1) {
                throw KeyNotFoundException<Key>(key);
            }
            return _values[keyIndex];
        }
                
        Strong<Value> operator[](const Key& key) const noexcept(false) {
            return get(key);
        }
        
        const size_t getCount() const {
            return _keys.getCount();
        }
        
        void forEach(function<void(const Key&, const Value&)> todo) const {
            for (size_t idx = 0 ; idx < _keys.getCount() ; idx++) {
                todo(_keys[idx], _values[idx]);
            }
        }
        
        virtual const uint64_t getHash() const override {
            return _keys.getHash() ^ _values.getHash();
        }
        
        virtual const Kind getKind() const override {
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

#endif /* dictionary_hpp */
