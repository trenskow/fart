//
//  array.hpp
//  fart
//
//  Created by Kristian Trenskow on 04/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef array_hpp
#define array_hpp

#include <type_traits>

#include "../threading/mutex.hpp"
#include "../memory/strong.hpp"
#include "./type.hpp"

using namespace fart::memory;
using namespace fart::threading;
using namespace fart::exceptions::types;

namespace fart::types {

	template<typename T>
	class Data;

	template<class T>
	class Array : public Type {

		static_assert(std::is_base_of<Object, T>::value);

		friend class Strong<Array<T>>;

	private:

		Array(const Data<T*>& other) : _storage(other), _hash(0), _hashIsDirty(true) {
			for (size_t idx = 0 ; idx < this->_storage.count() ; idx++) {
				this->_storage.itemAtIndex(idx)->retain();
			}
		}
		
		Data<T*> _storage;
		mutable uint64_t _hash;
		mutable bool _hashIsDirty;
		Mutex _hashMutex;

		static void _insertionSort(Array& array, function<bool(const T& item1, const T& items2)> comparer) {
			size_t sortedIdx = 0;
			while (sortedIdx < array.count() - 1) {
				sortedIdx++;
				for (ssize_t idx = sortedIdx - 1 ; idx >= 0 ; idx--) {
					if (comparer((T&)array[idx], (T&)array[idx + 1])) array.swapItemAtIndexes(idx + 1, idx);
				}
			}
		}

		static void _quickSort(Array& array, size_t offset, size_t count, function<bool(const T& item1, const T& items2)> comparer) {
			if (count == 0) return;
			size_t pivot = count - 1;
			size_t idx = 0;
			while (idx < pivot) {
				if (comparer((T&)array[offset + idx], (T&)array[offset + pivot])) {
					if (idx == pivot - 1) {
						array.swapItemAtIndexes(offset + idx, offset + pivot);
					} else {
						array.swapItemAtIndexes(offset + pivot - 1, offset + pivot);
						array.swapItemAtIndexes(offset + pivot, offset + idx);
					}
					pivot--;
				} else {
					idx++;
				}
			}
			if (pivot > 0) _quickSort(array, offset, pivot, comparer);
			_quickSort(array, offset + pivot + 1, count - (pivot + 1), comparer);
		}

	public:
		
		Array() : _hash(0), _hashIsDirty(true) {}

		Array(const Array<T>& other) : Array(other._storage) {}

		Array(Strong<T> repeating, size_t count) : Array() {
			for (size_t idx = 0 ; idx < count ; idx++) {
				append(repeating);
			}
		}

		Array(size_t count, Strong<T> items[]) : Array() {
			for (size_t idx = 0 ; idx < count ; idx++) {
				append(items[idx]);
			}
		}

		Array(size_t capacity) : _storage(capacity), _hash(0), _hashIsDirty(true) {}

		virtual ~Array() {
			for (size_t idx = 0 ; idx < _storage.count() ; idx++) {
				_storage.itemAtIndex(idx)->release();
			}
		}

		size_t count() const {
			return _storage.count();
		}

		Strong<T> itemAtIndex(size_t index) const noexcept(false) {
			return Strong<T>(_storage.itemAtIndex(index));
		}

		Strong<T> operator[](const size_t index) const noexcept(false) {
			return this->itemAtIndex(index);
		}

		void append(const T& item) {
			// Strong garantees object on the heap.
			Strong<T> heapItem = (T*)&item;
			heapItem->retain();
			_storage.append(heapItem);
			_hashMutex.locked([this]() {
				_hashIsDirty = true;
			});
		}

		void removeItemAtIndex(size_t index) noexcept(false) {
			_storage.removeItemAtIndex(index)->release();
			_hashMutex.locked([this]() {
				_hashIsDirty = true;
			});
		}

		void removeItem(const T& item) {
			ssize_t idx = indexOf(item);
			if (idx == -1) throw NotFoundException<T>(item);
			removeItemAtIndex(idx);
		}

		void replace(const T& item, size_t index) {
			Strong<T> heapItem = (T*)&item;
			heapItem->retain();
			_storage.replace(heapItem, index)->release();
		}

		ssize_t indexOf(const T& item) const {
			for (size_t idx = 0 ; idx < _storage.count() ; idx++) {
				if (*_storage[idx] == item) return idx;
			}
			return -1;
		}

		void forEach(function<void(T& value)> todo) const {
			this->_storage.forEach([&todo](T* item) {
				todo(*item);
			});
		}

		template<typename R, typename F>
		R reduce(R initial, F todo) const {
			return this->_storage.reduce(initial, [&todo](R result, T* item, size_t idx) {
				return todo(result, *item, idx);
			});
		}

		template<typename R>
		Strong<Array<R>> map(function<Strong<R>(T& value, const size_t idx)> todo) const {
			return this->reduce<Strong<Array<R>>>(Strong<Array<R>>(), [&todo](Strong<Array<R>> result, T& value, const size_t idx) {
				result->append(todo(value, idx));
				return result;
			});
		}

		Strong<Array<T>> filter(function<bool(T& value, size_t idx)> todo) const {
			return Strong<Array<T>>(this->_storage.filter([&todo](T* item, size_t idx) {
				return *todo(*item, idx);
			}));
		}

		bool some(function<bool(T& value)> todo) const {
			return this->_storage.some([&todo](T* item) {
				return todo(*item);
			});
		}

		Strong<Array<T>> subarray(size_t index, size_t length) const {
			return Strong<Array<T>>(this->_storage.subdata(index, length));
		}

		Strong<Array<T>> subarray(size_t index) const {
			return subarray(index, count() - index);
		}

		Strong<Array<T>> reversed() const {
			return Strong<Array<T>>(this->_storage.reversed());
		}

		void randomize() {
			srand((uint32_t)time(NULL));
			Data<T*> result;
			while (_storage.count() > 0) {
				size_t idx = rand() % _storage.count();
				result.append(_storage[idx]);
				_storage.removeItemAtIndex(idx);
			}
			_storage = result;
		}

		void moveItemAtIndex(size_t srcIndex, size_t dstIndex) noexcept(false) {
			this->_storage.moveItemAtIndex(srcIndex, dstIndex);
		}

		void swapItemAtIndexes(size_t index1, size_t index2) noexcept(false) {
			this->_storage.swapItemsAtIndexes(index1, index2);
		}

		void insertItemAtIndex(const T& item, size_t dstIndex) noexcept(false) {
			if (dstIndex > this->count()) throw OutOfBoundException(dstIndex);
			Strong<T> heapItem = item;
			heapItem->retain();
			_storage.insertItemAtIndex(heapItem, dstIndex);
			_hashMutex.locked([this]() {
				_hashIsDirty = true;
			});
		}

		void sort(function<bool(const T& item1, const T& item2)> comparer) {
			if (this->count() <= 10) _insertionSort(*this, comparer);
			else _quickSort(*this, 0, this->count(), comparer);
		}

		void sort() {
			this->sort([](const T& item1, const T& item2) { return item1 > item2; });
		}

		uint64_t hash() const override {
			return _hashMutex.lockedValue([this]() {
				if (_hashIsDirty) {
					Hashable::Builder builder;
					for (size_t idx = 0 ; idx < _storage.count() ; idx++) {
						builder.add(_storage[idx]->hash());
					}
					_hash = builder;
					_hashIsDirty = false;
				}
				return _hash;
			});
		}

		virtual Kind kind() const override {
			return Kind::array;
		}

		bool operator==(const Array<T>& other) const {
			if (!Type::operator==(other)) return false;
			for (size_t idx = 0 ; idx < _storage.count() ; idx++) {
				if (*_storage[idx] != *other[idx]) return false;
			}
			return true;
		}

	};

}

#endif /* array_hpp */
