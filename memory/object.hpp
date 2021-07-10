//
//  object.hpp
//  fart
//
//  Created by Kristian Trenskow on 17/08/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef object_hpp
#define object_hpp

#include <assert.h>
#include <unistd.h>
#include <atomic>

#include "../threading/mutex.hpp"
#include "../exceptions/exception.hpp"
#include "./weak.hpp"

#define WEAK_REFERENCES_BLOCK_SIZE 128

using namespace fart::threading;
using namespace fart::exceptions::memory;

namespace fart::memory {

	class Object {

		template<class T>
		friend class Strong;

		template<class T>
		friend class Weak;

	private:

		mutable std::atomic<size_t> _retainCount;
		mutable void** _weakReferences;
		mutable size_t _weakReferencesSize;
		mutable size_t _weakReferencesCount;
		mutable Mutex _mutex;

		void addWeakReference(void* weakReference) const {
			_mutex.locked([this,weakReference]() {
				if (_weakReferencesSize < _weakReferencesCount + 1) {
					_weakReferencesSize = ((_weakReferencesCount + 1) / WEAK_REFERENCES_BLOCK_SIZE) + 1 * WEAK_REFERENCES_BLOCK_SIZE;
					_weakReferences = (void**)realloc(_weakReferences, _weakReferencesSize);
					_weakReferences[_weakReferencesCount++] = weakReference;
				}
			});
		}

		void removeWeakReference(void* weakReference) const {
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

#ifdef FART_ALLOW_MANUAL_HEAP
	public:
#else
	protected:
#endif

		void *operator new(size_t size) noexcept(false) {
			void *mem = calloc(size, sizeof(uint8_t));
			if (!mem) throw AllocationException(size);
			return mem;
		}

		void operator delete(void *ptr) throw() {
			free(ptr);
		}

	public:

		Object() : _retainCount(0), _weakReferences(nullptr), _weakReferencesSize(0), _weakReferencesCount(0) {}

		Object(const Object& other) : Object() {}
		Object(Object&& other) { }

		virtual ~Object() {
			_mutex.locked([this]() {
				assert(_retainCount == 0);
				for (size_t idx = 0 ; idx < _weakReferencesCount ; idx++) {
					((Weak<Object>*)_weakReferences[0])->_object = nullptr;
				}
			});
		}

		void retain() const {
			this->_retainCount++;
		}

		void release() const {
			if (--this->_retainCount == 0) delete this;
		}

		size_t retainCount() const {
			return this->_retainCount;
		}

		Object& operator=(const Object&) {
			return *this;
		}

		Object& operator=(Object&& other) {
			return *this;
		}

	};

}

#endif /* object_hpp */
