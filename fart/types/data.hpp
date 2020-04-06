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
#include "../tools/math.hpp"

using namespace fart::memory;
using namespace fart::exceptions::types;
using namespace fart::tools::math;

namespace fart::types {
    
    class String;
    
    enum IncludeSeparator {
        none,
        prefix,
        suffix
    };
    
    template<typename T = uint8_t>
    class Data : public Type {
        
    public:
        
        static const size_t blockSize = 4096;
        
        class Comparitor : public Object {
            
        public:
            virtual ~Comparitor() {}
            
            virtual const T transform(T value) const {
                return value;
            }
            
        };
        
        template<typename F>
        static Strong<Data<T>> fromCBuffer(const F& todo, size_t count = blockSize) {
            T buffer[count];
            size_t length = todo(buffer, count);
            return Strong<Data<T>>((T*)buffer, length);
        }
        
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
            append(data.items(), data.count());
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
        
        size_t count() const {
            return _mutex.lockedValue([this]() {
                return this->_count;
            });
        }
        
        const T itemAtIndex(const size_t index) const noexcept(false) {
            return _mutex.lockedValue([this,index]() {
                if (index >= _count) throw OutOfBoundException(index);
                return _store[index];
            });
        }
        
        const T* items() const {
            return _mutex.lockedValue([this]() {
                return _store;
            });
        }
        
        const T operator[](const size_t index) const noexcept(false) {
            return itemAtIndex(index);
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
        
        Strong<Data<T>> subdata(const size_t offset, const ssize_t length = -1) const {
            return _mutex.lockedValue([this,offset,length]{
                ssize_t l = length;
                if (l < 0) l = _count - offset;
                return Strong<Data<T>>(&_store[offset], l);
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
        
        const size_t copy(void* bytes, size_t count, size_t offset = 0) {
            return _mutex.lockedValue([this,bytes,&count,&offset](){
                ssize_t toCopy = tools::math::max<ssize_t>(0, tools::math::min<ssize_t>(count, (ssize_t)_count - (ssize_t)offset));
                memcpy(bytes, _store, sizeof(T) * count);
                return toCopy;
            });
        }
        
        Strong<Array<Data<T>>> split(const Array<Data<T>>& separators, IncludeSeparator includeSeparator = IncludeSeparator::none, size_t max = 0) const {
            return _mutex.lockedValue([this,separators,&includeSeparator,max]() {
                Strong<Array<Data<T>>> result;
                ssize_t idx = 0;
                while (result->count() < max - 1) {
                    if (!separators.some([this,&idx,&result,&includeSeparator](const Data<T>& separator) {
                        ssize_t next = indexOf(separator, idx + (includeSeparator == prefix ? separator.count() : 0));
                        if (next == -1) return false;
                        switch (includeSeparator) {
                            case none:
                            case prefix:
                                result->append(subdata(idx, next - idx));
                                break;
                            case suffix:
                                result->append(subdata(idx, next + separator.count()));
                                break;
                        }
                        switch (includeSeparator) {
                            case none:
                            case suffix:
                                idx = next + separator.count();
                                break;
                            case prefix:
                                idx = next;
                                break;
                        }
                        return true;
                    })) break;
                }
                result->append(subdata(idx, count() - idx));
                return result;
            });
        }
        
        Strong<Array<Data<T>>> split(Strong<Data<T>> separator, IncludeSeparator includeSeparator = IncludeSeparator::none, size_t max = 0) const {
            return split(Array<Data<T>>(separator, 1), includeSeparator, max);
        }
        
        Strong<Array<Data<T>>> split(const T* seperator, size_t length, IncludeSeparator includeSeparator = IncludeSeparator::none, size_t max = 0) const {
            return split(Strong<Data<T>>(seperator, length), includeSeparator, max);
        }
        
        static Strong<Data<T>> join(Array<Data<T>>& datas, Data<T>* seperator) {
            return datas.reduceIndex(Strong<Data<T>>(), [datas, seperator](Strong<Data<T>> result, const size_t idx) {
                result->append(datas.itemAtIndex(idx));
                if (seperator != nullptr && idx != datas.count() - 1) result->append(*seperator);
                return result;
            });
        }
        
        static Strong<Data<T>> join(Array<Data<T>>& datas) {
            return join(datas, nullptr);
        }
        
        static Strong<Data<T>> join(Array<Data<T>>& datas, Data<T>& seperator) {
            return join(datas, &seperator);
        }
        
        template<typename O>
        Strong<Data<O>> as() {
            return _mutex.lockedValue([this]() {
                return Strong<Data<O>>((const O*)this->_store, (this->_count * sizeof(T)) / sizeof(O));
            });
        }
        
        virtual const uint64_t hash() const override {
            return _mutex.lockedValue([this]() {
                if (_hashIsDirty) {
                    _hash = 5381;
                    for (size_t idx = 0 ; idx < _count ; idx++) {
                        uint64_t value = (uint64_t)_comparitor->transform(_store[idx]);
                        _hash = ((_hash << 5) + _hash) + value; /* hash * 33 + c */
                    }
                    _hashIsDirty = false;
                }
                return _hash;
            });
        }
        
        virtual const Kind kind() const override {
            return Kind::data;
        }
        
        bool operator ==(const Data<T>& other) const {
            if (!Type::operator==(other)) return false;
            return _mutex.lockedValue([this,other]() {
                if (this->_count != other._count) return false;
                for (size_t idx = 0 ; idx < this->_count ; idx++) {
                    const T lhs = _comparitor->transform(_store[idx]);
                    const T rhs = _comparitor->transform(other.itemAtIndex(idx));
                    if (lhs != rhs) return false;
                }
                return true;
            });
        }
        
        void operator=(const Data<T>& other) {
            _mutex.locked([this,other]() {
                _count = 0;
                append(other);
            });
        }
        
        void setComparitor(Comparitor& comparitor) {
            _comparitor = comparitor;
        }
        
    private:
        
        T* _store;
        size_t _count;
        size_t _storeCount;
        mutable uint64_t _hash;
        mutable bool _hashIsDirty;
        mutable Mutex _mutex;
        
        Strong<Comparitor> _comparitor;
        
        void ensureStoreSize(size_t count) {
            if (_storeCount < count) {
                _storeCount = ((count / blockSize) + 1) * blockSize;
                _store = (T*) realloc(_store, sizeof(T) * _storeCount);
            }
        }
            
    };

}

#endif /* data_hpp */
