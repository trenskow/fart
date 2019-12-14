//
//  data.hpp
//  fart
//
//  Created by Kristian Trenskow on 17/08/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef data_hpp
#define data_hpp

#include <cstdlib>
#include <cstring>

#include "../threading/mutex.hpp"
#include "../exceptions/exception.hpp"
#include "../memory/strong.hpp"
#include "./type.hpp"
#include "./array.hpp"

#define ARRAY_STORE_BLOCK_SIZE 4096

using namespace fart::memory;
using namespace fart::exceptions::types;

namespace fart::types {
    
    template<typename T>
    class Data : public Type {
        
    private:
        T* _store;
        size_t _count;
        size_t _storeCount;
        mutable uint64_t _hash;
        mutable bool _hashIsDirty;
        mutable Mutex _mutex;
        
        void ensureStoreSize(size_t count) {
            if (_storeCount < count) {
                _storeCount = (count / ARRAY_STORE_BLOCK_SIZE) + 1 * ARRAY_STORE_BLOCK_SIZE;
                _store = (T*) realloc(_store, sizeof(T) * _storeCount);
            }
        }
        
    public:
        
        Data(const T* items, size_t count) : _store(nullptr), _storeCount(0), _count(0), _hash(0), _hashIsDirty(true) {
            append(items, count);
        }
        
        Data() : Data(nullptr, 0) {}
        
        Data(const Data<T>& other) : Data(nullptr, 0) {
            append(other);
        }
        
        virtual ~Data() {
            _mutex.locked([this]() {
                if (_store != nullptr) {
                    free(_store);
                    _store = nullptr;
                }
            });
        }
        
        void append(const T* items, size_t count) {
            if (!count) return;
            _mutex.locked([this,items,count]() {
                this->ensureStoreSize(this->_count + count);
                memcpy(&this->_store[this->_count], items, sizeof(T) * count);
                this->_count += count;
                this->_hashIsDirty = true;
            });
        }
        
        void append(T element) {
            append(&element, 1);
        }
                    
        void append(const Data<T>& data) {
            append(data.getItems(), data.getCount());
        }
        
        const T removeItemAtIndex(size_t index) noexcept(false) {
            return _mutex.lockedValue([this,index]() {
                if (index >= _count) throw OutOfBoundException(index);
                T element = _store[index];
                for (size_t idx = index ; idx < _count - 1 ; idx++) {
                    _store[idx] = _store[idx + 1];
                }
                _count--;
                _hashIsDirty = true;
                return element;
            });
        }
        
        size_t getCount() const {
            return _mutex.lockedValue([this]() {
                return this->_count;
            });
        }
        
        const T getItemAtIndex(const size_t index) const noexcept(false) {
            return _mutex.lockedValue([this,index]() {
                if (index >= _count) throw OutOfBoundException(index);
                return _store[index];
            });
        }
        
        const T* getItems() const {
            return _mutex.lockedValue([this]() {
                return _store;
            });
        }
        
        const T operator[](const size_t index) const noexcept(false) {
            return getItemAtIndex(index);
        }
        
        const ssize_t indexOf(const Data<T>& other, const size_t offset = 0) const {
            return _mutex.lockedValue([this,other,offset]() {
                for (size_t hidx = offset ; hidx < this->_count ; hidx++) {
                    bool found = true;
                    for (size_t nidx = 0 ; nidx < other._count ; nidx++) {
                        if (this->_store[hidx + nidx] != other[nidx]) {
                            found = false;
                            break;
                        }
                    }
                    if (found) return (ssize_t)hidx;
                }
                return (ssize_t)-1;
            });
        }
        
        const ssize_t indexOf(const T other, const size_t offset = 0) const {
            return indexOf(Data<T>(&other, 1), offset);
        }
        
        T replace(T element, const size_t index) const {
            return _mutex.lockedValue([this,element,index]() {
                T removed = _store[index];
                _store[index] = element;
                return removed;
            });
        }
        
        Strong<Data<T>> subdata(const size_t offset, const size_t length) const {
            return _mutex.lockedValue([this,offset,length]{
                return Strong<Data<T>>(&_store[offset], length);
            });
        }
        
        Strong<Data<T>> remove(const size_t offset, const size_t length) {
            return _mutex.lockedValue([this,offset,length]() {
                if (offset + length > _count) throw OutOfBoundException(offset + length);
                Strong<Data<uint8_t>> result(&_store[offset], length);
                size_t moveCount = _count - (offset + length);
                for (size_t idx = 0 ; idx < moveCount ; idx++) {
                    _store[offset + idx] = _store[offset + length + idx];
                }
                _count -= length;
                return result;
            });
        }
        
        void drain() {
            _mutex.locked([this]() {
                _count = 0;
            });
        };
        
        Strong<Array<Data<T>>> split(const Data<T>& separator, size_t max = 0) const {
            return _mutex.lockedValue([this,separator,max]() {
                Strong<Array<Data<T>>> result;
                ssize_t idx = 0;
                while (result->getCount() < max - 1) {
                    ssize_t next = indexOf(separator, idx);
                    if (next == -1) break;
                    result->append(subdata(idx, next - idx));
                    idx = next + separator.getCount();
                }
                result->append(subdata(idx, getCount() - idx));
                return result;
            });
        }
        
        Strong<Array<Data<T>>> split(const T* seperator, size_t length, size_t max = 0) const {
            return split(Data<T>(seperator, length), max);
        }
        
        static Strong<Data<T>> join(Array<Data<T>>& datas, Data<T>* seperator) {
            return datas.reduce(Strong<Data<T>>(), [seperator](Strong<Data<T>> result, Data<T>& current) {
                result->append(current);
                if (seperator != nullptr) result->append(*seperator);
                return result;
            });
        }
        
        static Strong<Data<T>> join(Array<Data<T>>& datas) {
            return join(datas, nullptr);
        }
        
        static Strong<Data<T>> join(Array<Data<T>>& datas, Data<T>& seperator) {
            return join(datas, &seperator);
        }
        
        virtual const uint64_t getHash() const {
            return _mutex.lockedValue([this]() {
                if (_hashIsDirty) {
                    _hash = 5381;
                    for (size_t idx = 0 ; idx < _count ; idx++) {
                        _hash = ((_hash << 5) + _hash) + (uint64_t)_store[idx]; /* hash * 33 + c */
                    }
                    _hashIsDirty = false;
                }
                return _hash;
            });
        }
        
        virtual const Kind getKind() const {
            return Kind::data;
        }
        
        bool operator ==(const Data<T>& other) const {
            if (!Type::operator==(other)) return false;
            return _mutex.lockedValue([this,other]() {
                if (this->_count != other._count) return false;
                for (size_t idx = 0 ; idx < this->_count ; idx++) {
                    if (this->_store[idx] != other._store[idx]) return false;
                }
                return true;
            });
        }
        
    };

}

#endif /* data_hpp */
