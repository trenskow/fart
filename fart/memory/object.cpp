//
//  object.cpp
//  fart
//
//  Created by Kristian Trenskow on 17/08/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#include <assert.h>

#include "./weak.hpp"
#include "./object.hpp"

#define WEAK_REFERENCES_BLOCK_SIZE 128

using namespace fart::memory;

Object::Object() : _retainCount(0), _weakReferences(nullptr), _weakReferencesSize(0), _weakReferencesCount(0) {}

Object::~Object() {
    _mutex.locked([this]() {
        assert(_retainCount == 0);
        for (size_t idx = 0 ; idx < _weakReferencesCount ; idx++) {
            ((Weak<Object>*)_weakReferences[0])->_object = nullptr;
        }
    });
}

Object::Object(const Object& other) : Object() {}

void* Object::operator new(size_t size) noexcept(false) {
    void *mem = calloc(size, sizeof(uint8_t));
    if (!mem) throw AllocationException(size);
    return mem;
}

void Object::operator delete(void *ptr) throw() {
    free(ptr);
}

void Object::addWeakReference(void *weakReference) const {
    _mutex.locked([this,weakReference]() {
        if (_weakReferencesSize < _weakReferencesCount + 1) {
            _weakReferencesSize = ((_weakReferencesCount + 1) / WEAK_REFERENCES_BLOCK_SIZE) + 1 * WEAK_REFERENCES_BLOCK_SIZE;
            _weakReferences = (void**)realloc(_weakReferences, _weakReferencesSize);
            _weakReferences[_weakReferencesCount++] = weakReference;
        }
    });
}

void Object::removeWeakReference(void *weakReference) const {
    _mutex.locked([this,weakReference]() {
        for (size_t idx = 0 ; idx < _weakReferencesCount ; idx++) {
            if (_weakReferences[idx] == weakReference) {
                for (size_t midx = idx ; midx <= _weakReferencesCount - 1 ; midx++) {
                    _weakReferences[midx] = _weakReferences[midx + 1];
                }
                _weakReferencesCount--;
            }
        }
    });
}

void Object::retain() const {
    _mutex.locked([this]() {
        this->_retainCount++;
    });
}

void Object::release() const {
    bool destroy = _mutex.lockedValue([this]() {
        this->_retainCount--;
        return (this->_retainCount == 0);
    });
    if (destroy) delete this;
}

size_t Object::retainCount() const {
    return _mutex.lockedValue([this]() {
        return this->_retainCount;
    });
}
