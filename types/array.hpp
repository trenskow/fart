//
// array.hpp
// fart
//
// Created by Kristian Trenskow on 2018/09/04.
// See license in LICENSE.
//

#ifndef array_hpp
#define array_hpp

#include <type_traits>

#include "../memory/strong.hpp"
#include "./type.hpp"
#include "./comparable.hpp"

using namespace fart::memory;
using namespace fart::exceptions::types;

namespace fart::types {

	template<typename T>
	class Data;

	template<typename T = Type>
	class Array : public Type {

		static_assert(std::is_base_of<Object, T>::value);

		friend class Strong<Array<T>>;

	public:

		static Type::Kind typeKind() {
			return Type::Kind::array;
		}

		using Comparer = function<bool(T&, T&)>;
		using ComparerIndex = function<bool(size_t, size_t)>;

		using TesterIndex = function<bool(T&, size_t)>;
		using Tester = function<bool(T&)>;

		template<typename R>
		using Reducer = function<R(R, T&)>;
		template<typename R>
		using ReducerIndex = function<R(R, T&, size_t)>;
		template<typename R>
		using ReducerIndexStop = function<R(R, T&, size_t, bool*)>;

	private:

		class Storage: public Data<T*> {

		public:

			Storage() : Data<T*>() {}

			Storage(const Data<T*>& other) : Data<T*>(other) { }

			Storage(const Storage& other) : Data<T*>(other) { }
			Storage(Storage&& other) : Data<T*>(std::move(other)) { }

			virtual ~Storage() { }

			Storage& operator=(const Storage& other) {
				Data<T*>::operator=(other);
				return *this;
			}

			Storage& operator=(Storage&& other) {
				Data<T*>::operator=(std::move(other));
				return *this;
			}

		};

		Array(const Data<T*>& other) : _storage(other) {
			for (size_t idx = 0 ; idx < this->_storage.length() ; idx++) {
				this->_storage[idx]->retain();
			}
		}

		Storage _storage;

		static void _insertionSort(Array& array, Comparer comparer) {
			for (size_t idx = 1 ; idx < array.count() ; idx++) {
				size_t j = idx;
				while (j > 0 && comparer(array[j - 1], array[j])) {
					array.swapItemAtIndices(j - 1, j);
					j--;
				}
			}
		}

		static void _quickSort(Array& array, const size_t& offset, const size_t& count, Comparer comparer) {
			if (count == 0) return;
			size_t pivot = count - 1;
			size_t idx = 0;
			while (idx < pivot) {
				if (comparer(array[offset + idx], array[offset + pivot])) {
					if (idx == pivot - 1) {
						array.swapItemAtIndices(offset + idx, offset + pivot);
					} else {
						array.swapItemAtIndices(offset + pivot - 1, offset + pivot);
						array.swapItemAtIndices(offset + pivot, offset + idx);
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

		static Strong<Array<T>> flatten(const Array<Array<T>>& arrays) {
			return arrays.template reduce<Strong<Array<T>>>(Strong<Array<T>>(), [](Strong<Array<T>> result, const Array& array) {
				return result->appendingAll(array);
			});
		}

		Array() : Type() {}

		Array(const Array<T>& other) : Array(other._storage) {}

		Array(Array&& other) : Type(), _storage(std::move(other._storage)) { }

		Array(Strong<T> repeatedItem, size_t count = 1) : Array() {
			for (size_t idx = 0 ; idx < count ; idx++) {
				append(repeatedItem);
			}
		}

		Array(const T& repeating, size_t count = 1) : Array(Strong<T>(repeating), count) {}

		Array(size_t count, T* items) : Array() {
			for (size_t idx = 0 ; idx < count ; idx++) {
				append(items[idx]);
			}
		}

		Array(size_t capacity) : _storage(capacity) {}

		Array(std::initializer_list<Strong<T>> items) : Array() {
			for (Strong<T> item : items) {
				append(item);
			}
		}

		Array(std::initializer_list<T> items) : Array() {
			for (Strong<T> item : items) {
				append(item);
			}
		}

		virtual ~Array() {
			for (size_t idx = 0 ; idx < _storage.length() ; idx++) {
				_storage[idx]->release();
			}
		}

		inline size_t count() const {
			return _storage.length();
		}

		inline size_t count(Tester tester) const {
			return _storage.count([&tester](T* other) {
				return tester(*other);
			});
		}

		inline size_t count(const T& item) const {
			return this->count([&item](T* other) {
				return *other == item;
			});
		}

		inline Strong<T> itemAtIndex(const size_t& index) const noexcept(false) {
			return _storage[index];
		}

		inline Strong<T> operator[](const size_t& index) const noexcept(false) {
			return this->itemAtIndex(index);
		}

		void append(Strong<T> item) {
			item->retain();
			_storage.append(item);
		}

		Strong<Array<T>> appending(Strong<T> item) const {
			Strong<Array<T>> result(*this);
			result->append(item);
			return result;
		}

		Strong<Array<T>> appending(T item) const {
			return appending(Strong<T>(item));
		}

		void appendAll(const Array<T>& other) {
			other.forEach([&](const T& item) {
				this->append(Strong<T>(item));
			});
		}

		Strong<Array<T>> appendingAll(const Array<T>& other) const {
			Strong<Array<T>> result(*this);
			other.forEach([&](const T& item) {
				result->append(item);
			});
			return result;
		}

		inline void removeItemAtIndex(const size_t& index) noexcept(false) {
			_storage.removeItemAtIndex(index)->release();
		}

		Strong<Array<T>> removingItemAtIndex(const size_t& index) const noexcept(false) {
			Strong<Array<T>> result(*this);
			result->removeItemAtIndex(index);
			return result;
		}

		void removeItem(const TesterIndex& test) noexcept(false) {
			size_t idx = indexOf(test);
			if (idx == NotFound) {
				throw NotFoundException();
			}
			removeItemAtIndex(idx);
		}

		Strong<Array<T>> removingItem(const TesterIndex& test) const noexcept(false) {
			Strong<Array<T>> result(*this);
			result->removeItem(test);
			return result;
		}

		inline void removeItem(const Tester& test) noexcept(false) {
			return removeItem([&test](T& item, const size_t) {
				return test(item);
			});
		}

		Strong<Array<T>> removingItem(const Tester& test) const noexcept(false) {
			Strong<Array<T>> result(*this);
			result->removeItem(test);
			return result;
		}

		inline void removeItem(const T& item1) noexcept(false) {
			return removeItem([&item1](T& item2) {
				return item1 == item2;
			});
		}

		Strong<Array<T>> removingItem(const T& item) const noexcept(false) {
			Strong<Array<T>> result(*this);
			result->removeItem(item);
			return result;
		}

		Strong<T> removeLast() noexcept(false) {
			if (this->count() == 0) throw OutOfBoundException(0);
			auto last = this->last();
			this->removeItemAtIndex(this->count() - 1);
			return last;
		}

		void replace(const T& item, const size_t& index) noexcept(false) {
			Strong<T> reference = item;
			reference->retain();
			_storage.replace(reference, index)->release();
		}

		Strong<Array<T>> replacing(const T& item, const size_t& idx) const noexcept(false) {
			Strong<Array<T>> result(*this);
			result->replace(item, idx);
			return result;
		}

		Array<T>& replaceMap(const Tester& test, const function<Strong<T>(T&)> transform) {
			size_t index = this->indexOf(test);
			if (index == NotFound) return *this;
			this->replace(transform(this->itemAtIndex(index)), index);
			return *this;
		}

		size_t indexOf(const TesterIndex& test) const {
			for (size_t idx = 0 ; idx < _storage.length() ; idx++) {
				if (test(*_storage[idx], idx)) return idx;
			}
			return NotFound;
		}

		inline size_t indexOf(const Tester& test) const {
			return indexOf([&test](T& item, const size_t) {
				return test(item);
			});
		}

		inline size_t indexOf(const T& item) const {
			return indexOf([&item](const T& stored) {
				return stored == item;
			});
		}

		inline bool contains(const TesterIndex& test) const {
			return indexOf(test) != NotFound;
		}

		inline bool contains(const Tester& test) const {
			return indexOf(test) != NotFound;
		}

		inline bool contains(const T& item) const {
			return indexOf(item) != NotFound;
		}

		inline Strong<T> first() const noexcept(false) {
			return _storage.first();
		}

		inline Strong<T> first(Strong<T> defaultValue) const {
			if (this->count() == 0) return defaultValue;
			return this->first();
		}

		inline Strong<T> first(Tester tester) const noexcept(false) {
			return _storage.first([&tester](T* item) {
				return tester(*item);
			});
		}

		inline Strong<T> last() const noexcept(false) {
			return _storage.last();
		}

		inline size_t firstIndex() const {
			if (count() > 0) return 0;
			return NotFound;
		}

		inline size_t firstIndex(Tester tester) const {
			for (size_t idx = 0 ; idx < count() ; idx++) {
				if (tester(itemAtIndex(idx))) return idx;
			}
			return NotFound;
		}

		inline size_t lastIndex() const {
			return _storage.lastIndex();
		}

		inline size_t lastIndex(Tester tester) const {
			return _storage.lastIndex([&tester](T* item) {
				return tester(*item);
			});
		}

		inline void forEach(const function<void(T& value)>& todo) const {
			this->forEach([&todo](T& value, size_t) {
				todo(value);
			});
		}

		inline void forEach(const function<void(T& value, size_t idx)>& todo) const {
			this->_storage.forEach([&todo](T* item, size_t idx) {
				todo(*item, idx);
			});
		}

		template<typename R>
		inline R reduce(R initial, ReducerIndexStop<R> todo) const {
			return this->_storage.template reduce<R>(initial, [&todo](R result, T* item, size_t idx, bool* stop) {
				return todo(result, *item, idx, stop);
			});
		}

		template<typename R>
		inline R reduce(R initial, ReducerIndex<R> todo) const {
			return reduce<R>(initial, [&todo](R result, T& item, size_t idx, bool*) {
				return todo(result, item, idx);
			});
		}

		template<typename R>
		inline R reduce(R initial, Reducer<R> todo) const {
			return reduce<R>(initial, [&todo](R result, T& item, size_t) {
				return todo(result, item);
			});
		}

		template<typename R>
		inline Strong<Array<R>> map(const function<Strong<R>(T&, size_t)>& transform) const {
			return reduce<Strong<Array<R>>>(Strong<Array<R>>(), [&transform](Strong<Array<R>> result, T& item, const size_t& idx) {
				return result->appending(transform(item, idx));
			});
		}

		template<typename R>
		inline Strong<Array<R>> map(const function<Strong<R>(T&)>& transform) const {
			return map<R>([&transform](T& item, size_t) {
				return transform(item);
			});
		}

		template<typename R>
		inline Strong<Data<R>> mapToData(const function<R(T&, size_t)>& transform) const {
			return reduce<Strong<Data<R>>>(
				Strong<Data<R>>(),
				[&transform](Strong<Data<R>> result, T& item, size_t idx) {
					return result->appending(transform(item, idx));
				});
		}

		template<typename R>
		inline Strong<Data<R>> mapToData(const function<R(T&)>& transform) const {
			return mapToData<R>([&transform](T& item, size_t) {
				return transform(item);
			});
		}

		inline Strong<Array<T>> filter(const TesterIndex& test) const {
			return Strong<Array<T>>(this->_storage.filter([&test](T* item, const size_t& idx) {
				return test(*item, idx);
			}));
		}

		inline Strong<Array<T>> filter(const Tester& test) const {
			return filter([&test](T& item, const size_t&) {
				return test(item);
			});
		}

		inline bool some(const TesterIndex& test, bool def = false) const {
			return this->_storage.some([&test](T* item, const size_t& idx) {
				return test(*item, idx);
			}, def);
		}

		inline bool some(const Tester& test, bool def = false) const {
			return some([&test](T& item, const size_t&) {
				return test(item);
			}, def);
		}

		inline bool every(const TesterIndex& test, bool def = true) const {
			return this->_storage.every([&test](T* item, const size_t& idx) {
				return test(*item, idx);
			}, def);
		}

		inline bool every(const Tester& test, bool def = true) const {
			return every([&test](T& item, const size_t&) {
				return test(item);
			}, def);
		}

		inline Strong<Array<T>> subarray(const size_t& index, const size_t& length) const {
			return Strong<Array<T>>(this->_storage.subdata(index, length));
		}

		inline Strong<Array<T>> subarray(const size_t& index) const {
			return subarray(index, count() - index);
		}

		inline Strong<Array<T>> reversed() const {
			return Strong<Array<T>>(this->_storage.reversed());
		}

		Strong<Array<T>> unique(const Comparer& comparer) const {
			Strong<Array<T>> result;
			this->forEach([&result,&comparer](T& item1) {
				if (!result->contains([&item1,&comparer](T& item2) { return comparer(item1, item2); })) {
					result->append(item1);
				}
			});
			return result;
		}

		inline Strong<Array<T>> unique() const {
			return this->unique([](const T& item1, const T& item2) { return item1 == item2; });
		}

		inline Strong<Array<T>> nonUnique(const Comparer& comparer) const {
			return this->filter([this,&comparer](T& item1) {
				return this->filter([&comparer,&item1](T& item2) {
					return comparer(item1, item2);
				})->count() > 1;
			});
		}

		inline Strong<Array<T>> nonUnique() const {
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

		inline void moveItemAtIndex(const size_t& srcIndex, const size_t& dstIndex) noexcept(false) {
			this->_storage.moveItemAtIndex(srcIndex, dstIndex);
		}

		inline void swapItemAtIndices(const size_t& index1, const size_t& index2) noexcept(false) {
			this->_storage.swapItemsAtIndices(index1, index2);
		}

		void insertItemAtIndex(Strong<T> item, const size_t& dstIndex) noexcept(false) {
			if (dstIndex > this->count()) throw OutOfBoundException(dstIndex);
			item->retain();
			_storage.insertItemAtIndex(item, dstIndex);
		}

		void sort(const Comparer comparer) {
			if (this->count() <= 10) _insertionSort(*this, comparer);
			else _quickSort(*this, 0, this->count(), comparer);
		}

		inline void sort() {
			this->sort([](const T& item1, const T& item2) { return item1 > item2; });
		}

		Strong<Array<T>> sorted(Comparer comparer) const {
			Strong<Array<T>> result = *this;
			result->sort(comparer);
			return result;
		}

		Strong<Array<T>> sorted() {
			return this->sorted([](const T& item1, const T& item2) { return item1 > item2; });
		}

		Strong<Array<Array<T>>> grouped(function<bool(const T&, const T&)> tester) const {
			Strong<Array<Array<T>>> result;
			if (this->count() == 0) return result;
			Strong<Array<T>> current(this->first(), 1);
			result->append(current);
			for (size_t idx = 1 ; idx < this->count() ; idx++) {
				Strong<T> item = this->itemAtIndex(idx);
				if (tester(*this->itemAtIndex(idx - 1), this->itemAtIndex(idx))) {
					current->append(item);
				} else {
					current = Strong<Array<T>>(item, 1);
					result->append(current);
				}
			}
			return result;
		}

		bool are(Type::Kind kind) const {
			return this->every([&](T& item) {
				return item.kind() == kind;
			});
		}

		inline virtual uint64_t hash() const override {
			return 0;
		}

		inline virtual Kind kind() const override {
			return Kind::array;
		}

		bool operator==(const Array<T>& other) const {
			if (!Type::operator==(other)) return false;
			if (this->count() != other.count()) return false;
			for (size_t idx = 0 ; idx < _storage.length() ; idx++) {
				if (!(*_storage[idx] == *other._storage[idx])) return false;
			}
			return true;
		}

		bool operator!=(const Array<T>& other) const {
			return !(this->operator==(other));
		}

		Array<T>& operator=(const Array<T>& other) {
			Type::operator=(other);
			for (size_t idx = 0 ; idx < _storage.length() ; idx++) {
				_storage[idx]->release();
			}
			_storage.drain();
			for (size_t idx = 0 ; idx < other._storage.length() ; idx++) {
				other._storage[idx]->retain();
				_storage.append(other._storage[idx]);
			}
			return *this;
		}

		Array<T>& operator=(Array<T>&& other) {
			for (size_t idx = 0 ; idx < this->_storage.length() ; idx++) {
				this->_storage[idx]->release();
			}
			_storage = std::move(other._storage);
			Type::operator=(std::move(other));
			return *this;
		}

		Strong<Array<T>> operator+(const Array<T>& other) const {
			return Array<T>::flatten(
				Strong<Array<Array<T>>>({
					Strong<Array<T>>(*this),
					Strong<Array<T>>(other) }));
		}

	};

}

#endif /* array_hpp */
