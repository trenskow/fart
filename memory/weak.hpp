//
//  weak.hpp
//  fart
//
//  Created by Kristian Trenskow on 2018/09/04.
//  Copyright © 2018-2023 Kristian Trenskow. All rights reserved.
//

#ifndef weak_hpp
#define weak_hpp

#include <type_traits>
#include <atomic>

#include "./allocator.hpp"

namespace fart::memory {

	class Object;

	template<typename T>
	class Strong;

	template<typename T>
	class Weak
#ifdef FART_ALLOW_MANUAL_HEAP
	: public Allocator
#else
	: public NoAllocator
#endif
	{

		static_assert(std::is_base_of<Object, T>::value);

		friend class Object;

	private:
		std::atomic<T*> _object;

		void _setObject(T* object) {
			if (_object != nullptr) {
				_object->removeWeakReference(this);
			}
			_object = object;
			if (_object) {
				_object->addWeakReference(this);
			}
		}

	public:
		Weak() : Weak(nullptr) {}
		Weak(T& object) : Weak(&object) {}

		Weak(T* object) : _object(nullptr) {
			_setObject(object);
		}

		Weak(std::nullptr_t) : _object(nullptr) {
			_setObject(nullptr);
		}

		Weak(const Strong<T>& other) : Weak(other) {}

		Weak(Weak<T>&& other) : _object(other._object) {
			other._object = nullptr;
		}

		~Weak() {
			_setObject(nullptr);
		}

		bool isNull() const {
			return _object == nullptr;
		}

		operator T&() const {
			return *_object;
		}

		operator T*() const {
			return _object;
		}

		Weak<T>& operator =(T& object) {
			_setObject(&object);
			return *this;
		}

		Weak<T>& operator =(T&& other) {
			_setObject(other._object);
			other._setObject(nullptr);
			return *this;
		}

		Weak<T>& operator =(T* object) {
			_setObject(object);
			return *this;
		}

		Weak<T>& operator =(Strong<T> object) {
			_setObject(object);
			return *this;
		}

		T* operator ->() const {
			return _object;
		}

		bool operator==(std::nullptr_t n) {
			return this->_object == n;
		}

		bool operator!=(std::nullptr_t n) {
			return !(this == n);
		}

	};

}

#endif /* weak_hpp */
