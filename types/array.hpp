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

	public:

		typedef function<bool(T& item1, T& item2)> Comparer;
		typedef function<bool(T& item, const size_t idx)> TesterIndex;
		typedef function<bool(T& item)> Tester;

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

		static void _insertionSort(Array& array, Comparer comparer) {
			size_t sortedIdx = 0;
			while (sortedIdx < array.count() - 1) {
				sortedIdx++;
				for (size_t idx = sortedIdx ; idx > 0 ; idx--) {
					if (comparer((T&)array[idx - 1], (T&)array[idx])) array.swapItemAtIndexes(idx, idx);
				}
			}
		}

		static void _quickSort(Array& array, size_t offset, size_t count, Comparer comparer) {
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

		void _setHashDirty() const {
			_hashMutex.locked([this]() {
				_hashIsDirty = true;
			});
		}

	public:

		Array() : Type(), _hash(0), _hashIsDirty(true) {}

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
			_setHashDirty();
		}

		void removeItemAtIndex(size_t index) noexcept(false) {
			_storage.removeItemAtIndex(index)->release();
			_setHashDirty();
		}

		void removeItem(TesterIndex test) noexcept(false) {
			size_t idx = indexOf(test);
			if (idx == NotFound) throw NotFoundException();
			removeItemAtIndex(idx);
		}

		void removeItem(Tester test) noexcept(false) {
			return removeItem([&test](T& item, const size_t idx) {
				return test(item);
			});
		}

		void removeItem(const T& item1) noexcept(false) {
			return removeItem([&item1](T& item2) {
				return item1 == item2;
			});
		}

		void replace(const T& item, size_t index) noexcept(false) {
			Strong<T> heapItem = (T*)&item;
			heapItem->retain();
			_storage.replace(heapItem, index)->release();
		}

		size_t indexOf(TesterIndex test) const {
			for (size_t idx = 0 ; idx < _storage.count() ; idx++) {
				if (test(*_storage[idx], idx)) return idx;
			}
			return NotFound;
		}

		size_t indexOf(Tester test) const {
			return indexOf([&test](T& item, const size_t idx) {
				return test(item);
			});
		}

		size_t indexOf(const T& item) const {
			return indexOf([&item](const T& stored) {
				return stored == item;
			});
		}

		bool contains(TesterIndex test) const {
			return indexOf(test) != NotFound;
		}

		bool contains(Tester test) const {
			return indexOf(test) != NotFound;
		}

		bool contains(const T& item) const {
			return indexOf(item) != NotFound;
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
		Strong<Array<R>> map(function<Strong<R>(T& item, const size_t idx)> transform) const {
			return this->reduce<Strong<Array<R>>>(Strong<Array<R>>(), [&transform](Strong<Array<R>> result, T& item, const size_t idx) {
				result->append(transform(item, idx));
				return result;
			});
		}

		template<typename R>
		Strong<Array<R>> map(function<Strong<R>(T& value)> transform) const {
			return map<R>([&transform](T& item, const size_t idx) {
				return transform(item);
			});
		}

		Strong<Array<T>> filter(TesterIndex test) const {
			return Strong<Array<T>>(this->_storage.filter([&test](T* item, const size_t idx) {
				return test(*item, idx);
			}));
		}

		Strong<Array<T>> filter(Tester test) const {
			return filter([&test](T& item, const size_t idx) {
				return test(item);
			});
		}

		bool some(TesterIndex test, bool def = false) const {
			return this->_storage.some([&test](T* item, const size_t idx) {
				return test(*item, idx);
			}, def);
		}

		bool some(Tester test, bool def = false) const {
			return some([&test](T& item, const size_t idx) {
				return test(item);
			}, def);
		}

		bool every(TesterIndex test, bool def = true) const {
			return this->_storage.every([&test](T* item, const size_t idx) {
				return test(*item, idx);
			}, def);
		}

		bool every(Tester test, bool def = true) const {
			return every([&test](T& item, const size_t idx) {
				return test(item);
			}, def);
		}

		bool are(Type::Kind kind) const {
			return this->every([&kind](const T& value) {
				return value.is(kind);
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

		Strong<Array<T>> unique(Comparer comparer) const {
			Strong<Array<T>> result;
			this->forEach([&result,&comparer](T& item1) {
				if (!result->contains([&item1,&comparer](T& item2) { return comparer(item1, item2); })) {
					result->append(item1);
				}
			});
			return result;
		}

		Strong<Array<T>> unique() const {
			return this->unique([](T& item1, T& item2) { return item1 == item2; });
		}

		Strong<Array<T>> nonUnique(Comparer comparer, bool flatten) const {
			Strong<Array<T>> result;
			for (size_t idx1 = 0 ; idx1 < count() ; idx1++) {
				for (size_t idx2 = idx1 + 1 ; idx2 < count() ; idx2++) {
					if (comparer(*_storage[idx1], *_storage[idx2])) {
						result->append(*_storage[idx1]);
						break;
					}
				}
			}
			if (!flatten) return result;
			return result->unique(comparer);
		}

		Strong<Array<T>> nonUnique(bool flatten) const {
			return nonUnique([](const T& item1, const T& item2) { return item1 == item2; }, flatten);
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
			_setHashDirty();
		}

		void sort(Comparer comparer) {
			if (this->count() <= 10) _insertionSort(*this, comparer);
			else _quickSort(*this, 0, this->count(), comparer);
		}

		void sort() {
			this->sort([](const T& item1, const T& item2) { return item1 > item2; });
		}

		virtual uint64_t hash() const override {
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

		Array<T>& operator=(const Array<T>& other) {
			Type::operator=(other);
			for (size_t idx = 0 ; idx < _storage.count() ; idx++) {
				_storage[idx]->release();
			}
			_storage.drain();
			for (size_t idx = 0 ; idx < other._storage.count() ; idx++) {
				other._storage[idx]->retain();
				_storage.append(other._storage[idx]);
			}
			_hash = other._hash;
			_hashIsDirty = other._hashIsDirty;
			return *this;
		}

	};

}

#endif /* array_hpp */
