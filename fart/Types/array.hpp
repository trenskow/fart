//
//  array.hpp
//  fart
//
//  Created by Kristian Trenskow on 04/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef array_hpp
#define array_hpp

#include "data.hpp"
#include "type.hpp"
#include "mutex.hpp"

using namespace fart::memory;
using namespace fart::threading;

namespace fart {
    
    namespace types {
        
        template<class T>
        class Array : public Type {
            
            static_assert(std::is_base_of<Object, T>::value, "T must be a subclass of class Object.");
            
        private:
            Data<T*> _storage;
            mutable uint64_t _hash;
            mutable bool _hashIsDirty;
            Mutex _hashMutex;
            
        public:
            Array() : _hash(0), _hashIsDirty(true) {}
            Array(const Array<T>& other) : _storage(other._storage.getItems(), other._storage.getCount()), _hash(0), _hashIsDirty(true) {
                for (size_t idx = 0 ; idx < _storage.getCount() ; idx++) {
                    _storage.getItemAtIndex(idx)->retain();
                }
            }
            
            virtual ~Array() {
                for (size_t idx = 0 ; idx < _storage.getCount() ; idx++) {
                    _storage.getItemAtIndex(idx)->release();
                }
            }
            
            const size_t getCount() const {
                return _storage.getCount();
            }
            
            Strong<T> getItemAtIndex(size_t index) const throw(OutOfBoundException) {
                return Strong<T>(_storage.getItemAtIndex(index));
            }
            
            Strong<T> operator[](const size_t index) const throw(OutOfBoundException) {
                return Strong<T>(this->getItemAtIndex(index));
            }
            
            void append(Strong<T> item) {
                item->retain();
                _storage.append(item);
                _hashMutex.locked([this]() {
                    _hashIsDirty = true;
                });
            }
            
            void removeItemAtIndex(size_t index) throw(OutOfBoundException) {
                _storage.removeItemAtIndex(index)->release();
            }
            
            const ssize_t indexOf(const T& item) const {
                for (size_t idx = 0 ; idx < _storage.getCount() ; idx++) {
                    if (*_storage[idx] == item) return idx;
                }
                return -1;
            }
            
            const uint64_t getHash() const {
                return _hashMutex.lockedValue([this]() {
                    if (_hashIsDirty) {
                        _hash = 5381;
                        for (size_t idx = 0 ; idx < _storage.getCount() ; idx++) {
                            _hash = ((_hash << 5) + _hash) + _storage[idx]->getHash(); /* hash * 33 + c */
                        }
                        _hashIsDirty = false;
                    }
                    return _hash;
                });
            }
            
            virtual const Kind getKind() const {
                return Kind::array;
            }
            
            bool operator==(const Array<T>& other) const {
                if (!Type::operator==(other)) return false;
                for (size_t idx = 0 ; idx < _storage.getCount() ; idx++) {
                    if (*_storage[idx] != *other[idx]) return false;
                }
                return true;
            }
            
        };
        
    }
    
}

#endif
