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

#include "../memory/strong.hpp"
#include "./type.hpp"

using namespace fart::memory;
using namespace fart::exceptions::types;

namespace fart::types {

	template<typename T>
	class Data;

	template<class T>
	class Array : public Type {

		static_assert(std::is_base_of<Object, T>::value);
		static_assert(std::is_base_of<Hashable, T>::value);

		friend class Strong<Array<T>>;

	public:

		typedef function<bool(T& item1, T& item2)> Comparer;
		typedef function<bool(T& item, const size_t idx)> TesterIndex;
		typedef function<bool(T& item)> Tester;

	private:

		template<typename S>
		class Storage: public Data<S> {

		public:

			Storage() : Data<S>() {}

			Storage(const Data<S>& other) : Data<S>(other) { }

			Storage(const Storage& other) : Data<S>(other) { }
			Storage(Storage&& other) : Data<S>(std::move(other)) { }

			virtual ~Storage() { }

			Storage& operator=(const Storage& other) {
				Data<S>::operator=(other);
				return *this;
			}

			Storage& operator=(Storage&& other) {
				Data<S>::operator=(std::move(other));
				return *this;
			}

		protected:

			virtual inline uint64_t hashForItem(const S& item) const override {
				return item->hash();
			}

		};

		Array(const Data<T*>& other) : _storage(other) {
			for (size_t idx = 0 ; idx < this->_storage.count() ; idx++) {
				this->_storage.itemAtIndex(idx)->retain();
			}
		}

		Storage<T*> _storage;

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

	public:

		Array() : Type() {}

		Array(const Array<T>& other) : Array(other._storage) {}

		Array(Array&& other) : Type(std::move(other)), _storage(std::move(other._storage)) { }

		Array(Strong<T> repeating, size_t count) : Array() {
			for (size_t idx = 0 ; idx < count ; idx++) {
				append(repeating);
			}
		}

		Array(const T& repeating, size_t count) : Array(Strong<T>(repeating), count) {}

		Array(size_t count, T* items) : Array() {
			for (size_t idx = 0 ; idx < count ; idx++) {
				append(items[idx]);
			}
		}

		Array(size_t capacity) : _storage(capacity) {}

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
		}

		Array<T> appending(const T& item) const {
			Array<T> result(*this);
			result.append(item);
			return result;
		}

		void removeItemAtIndex(size_t index) noexcept(false) {
			_storage.removeItemAtIndex(index)->release();
		}

		Array<T> removingItemAtIndex(size_t index) const noexcept(false) {
			Array<T> result(*this);
			result.removeItemAtIndex(index);
			return result;
		}

		void removeItem(TesterIndex test) noexcept(false) {
			size_t idx = indexOf(test);
			if (idx == NotFound) throw NotFoundException();
			removeItemAtIndex(idx);
		}

		Array<T> removingItem(TesterIndex test) const noexcept(false) {
			Array<T> result(*this);
			result.removeItem(test);
			return result;
		}

		void removeItem(Tester test) noexcept(false) {
			return removeItem([&test](T& item, const size_t idx) {
				return test(item);
			});
		}

		Array<T> removingItem(Tester test) const noexcept(false) {
			Array<T> result(*this);
			result.removeItem(test);
			return result;
		}

		void removeItem(const T& item1) noexcept(false) {
			return removeItem([&item1](T& item2) {
				return item1 == item2;
			});
		}

		Array<T> removingItem(const T& item) const noexcept(false) {
			Array<T> result(*this);
			result.removeItem(item);
			return result;
		}

		void replace(const T& item, size_t index) noexcept(false) {
			Strong<T> heapItem = (T*)&item;
			heapItem->retain();
			_storage.replace(heapItem, index)->release();
		}

		Array<T> replacing(const T& item, size_t idx) const noexcept(false) {
			Array<T> result(*this);
			result.replace(item, idx);
			return result;
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

		Strong<T> first() const noexcept(false) {
			return Strong<T>(_storage.first());
		}

		Strong<T> last() const noexcept(false) {
			return Strong<T>(_storage.last());
		}

		void forEach(function<void(T& value)> todo) const {
			this->_storage.forEach([&todo](T* item) {
				todo(*item);
			});
		}

		template<typename R, typename F>
		R reduce(R initial, F todo) const {
			return this->_storage.reduce(initial, [&todo](R& result, T* item, size_t idx) {
				return todo(result, *item, idx);
			});
		}

		template<typename R>
		Array<R> map(function<R(T& item, const size_t idx)> transform) const {
			return this->reduce<Array<R>>(Array<R>(), [&transform](Array<R> result, T& item, const size_t idx) {
				result.append(transform(item, idx));
				return result;
			});
		}

		template<typename R>
		Array<R> map(function<R(T& value)> transform) const {
			return map<R>([&transform](T& item, const size_t idx) {
				return transform(item);
			});
		}

		Array<T> filter(TesterIndex test) const {
			return Array<T>(this->_storage.filter([&test](T* item, const size_t idx) {
				return test(*item, idx);
			}));
		}

		Array<T> filter(Tester test) const {
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

		Array<T> subarray(size_t index, size_t length) const {
			return Array<T>(this->_storage.subdata(index, length));
		}

		Array<T> subarray(size_t index) const {
			return subarray(index, count() - index);
		}

		Array<T> reversed() const {
			return Array<T>(this->_storage.reversed());
		}

		Array<T> unique(Comparer comparer) const {
			Array<T> result;
			this->forEach([&result,&comparer](T& item1) {
				if (!result.contains([&item1,&comparer](T& item2) { return comparer(item1, item2); })) {
					result.append(item1);
				}
			});
			return result;
		}

		Array<T> unique() const {
			return this->unique([](const T& item1, const T& item2) { return item1 == item2; });
		}

		Array<T> nonUnique(Comparer comparer) const {
			return this->filter([this,&comparer](T& item1) {
				return this->filter([&comparer,&item1](T& item2) {
					return comparer(item1, item2);
				})->count() > 1;
			});
		}

		Array<T> nonUnique() const {
			return nonUnique([](const T& item1, const T& item2) { return item1 == item2; });
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
		}

		void sort(Comparer comparer) {
			if (this->count() <= 10) _insertionSort(*this, comparer);
			else _quickSort(*this, 0, this->count(), comparer);
		}

		void sort() {
			this->sort([](const T& item1, const T& item2) { return item1 > item2; });
		}

		virtual uint64_t hash() const override {
			return this->_storage.hash();
		}

		virtual Kind kind() const override {
			return Kind::array;
		}

		bool operator==(const Array<T>& other) const {
			if (!Type::operator==(other)) return false;
			for (size_t idx = 0 ; idx < _storage.count() ; idx++) {
				if (!(*_storage[idx] == *other[idx])) return false;
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
			return *this;
		}

		Array<T>& operator=(Array<T>&& other) {
			for (size_t idx = 0 ; idx < this->_storage.count() ; idx++) {
				this->_storage.itemAtIndex(idx)->release();
			}
			_storage = std::move(other._storage);
			Type::operator=(std::move(other));
			return *this;
		}

	};

}

#endif /* array_hpp */
