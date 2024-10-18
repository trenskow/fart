//
// data.hpp
// fart
//
// Created by Kristian Trenskow on 2018/08/17.
// See license in LICENSE.
//

#ifndef data_hpp
#define data_hpp

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "../memory/strong.hpp"
#include "../exceptions/exception.hpp"
#include "./type.hpp"
#include "./array.hpp"

#define MAX(x, y) (x > y ? x : y)

using namespace fart::memory;
using namespace fart::exceptions::types;
using namespace fart::tools;

namespace fart::types {

	class String;

	enum class IncludeSeparator {
		none,
		prefix,
		suffix,
		both
	};

	template<typename T = uint8_t>
	class Data : public Type, public Comparable<Data<T>> {

	public:

		static Type::Kind typeKind() {
			return Type::Kind::data;
		}

		using Tester = function<bool(T item)>;
		using TesterIndex = function<bool(T item, size_t idx)>;

		template<typename R>
		using Reducer = function<R(R result, T item)>;
		template<typename R>
		using ReducerIndex = function<R(R result, T item, size_t idx)>;
		template<typename R>
		using ReducerIndexStop = function<R(R result, T item, size_t idx, bool* stop)>;

		static Data<T> fromCBuffer(const function<size_t(T*,size_t)>& todo, size_t length) {
			T buffer[length];
			size_t read = todo(buffer, length);
			return Data<T>((T*)buffer, math::min(read, length));
		}

		Data(const T* items, size_t length) : Type(), _storage(new Storage()), _offset(0), _length(0), _size(sizeof(T)), _hashIsDirty(true), _hash(0) {
			append(items, length);
		}

		Data() : Data(nullptr, 0) {}

		Data(size_t capacity) : Data() {
			this->_ensureStorageSize(capacity);
		}

		Data(size_t repeating, T item) : Data() {
			for (size_t idx = 0 ; idx < repeating ; idx++) {
				append(item);
			}
		}

		Data(std::initializer_list<T> items) : Data() {
			for (T item : items) {
				append(item);
			}
		}

		Data(const Data<T>& other, size_t offset, size_t length) : _storage(other._storage->retain()), _offset(other._offset + offset), _length(length), _size(sizeof(T)), _hashIsDirty(offset == 0 && length == other._length), _hash(other._hash) { }

		Data(const Data<T>& other) : Data(other, 0, other._length) { }

		Data(Data<T>&& other) : Type() {
			this->_storage = other._storage;
			this->_offset = other._offset;
			this->_length = other._length;
			this->_size = other._size;
			this->_hashIsDirty = other._hashIsDirty;
			this->_hash = other._hash;
			other._storage = nullptr;
			other._offset = 0;
			other._length = 0;
			other._size = 0;
			other._hashIsDirty = true;
		}

		virtual ~Data() {
			Storage::release(&this->_storage);
			this->_offset = 0;
			this->_length = 0;
			this->_size = 0;
		}

		void append(const T* items, size_t length) {
			if (!length) return;
			this->insertItemsAtIndex(items, length, this->length());
		}

		inline void append(T element) {
			append(&element, 1);
		}

		inline void append(const Data<T>& data) {
			append(data.items(), data.length());
		}

		inline Strong<Data<T>> appending(T element) {
			Strong<Data<T>> result = *this;
			result->append(element);
			return result;
		}

		Strong<Data<T>> appending(const Data<T>& other) const {
			Strong<Data<T>> result = *this;
			result->append(other);
			return result;
		}

		T removeItemAtIndex(size_t index) noexcept(false) {

			if (index >= this->length()) throw OutOfBoundException(index);

			this->_ensureStorageOwnership();

			T element = this->_get(index);

			for (size_t idx = index ; idx < this->length() - 1 ; idx++) {
				this->_set(idx, this->_get(idx + 1));
			}

			this->_length--;
			_hashIsDirty = true;

			return element;

		}

		T removeLast() noexcept(false) {
			return this->removeItemAtIndex(this->length() - 1);
		}

		void moveItemAtIndex(size_t srcIndex, size_t dstIndex) noexcept(false) {

			if (srcIndex == dstIndex) return;

			if (srcIndex >= this->length()) throw OutOfBoundException(srcIndex);
			if (dstIndex >= this->length()) throw OutOfBoundException(dstIndex);

			this->_ensureStorageOwnership();

			T src = this->_get(srcIndex);

			if (srcIndex < dstIndex) {
				for (size_t idx = srcIndex + 1 ; idx <= dstIndex ; idx++) {
					this->_set(idx - 1, this->_get(idx));
				}
			} else {
				for (size_t idx = srcIndex ; idx > dstIndex ; idx--) {
					this->_set(idx, this->_get(idx - 1));
				}
			}

			this->_set(dstIndex, src);

		}

		void swapItemsAtIndices(size_t index1, size_t index2) noexcept(false) {

			if (index1 == index2) return;

			if (index1 >= this->length()) throw OutOfBoundException(index1);
			if (index2 >= this->length()) throw OutOfBoundException(index2);

			this->_ensureStorageOwnership();

			T reg = this->_get(index1);
			this->_set(index1, this->_get(index2));
			this->_set(index2, reg);

		}

		void insertItemsAtIndex(const T* items, size_t length, size_t dstIndex) noexcept(false) {

			if (dstIndex > this->length()) throw OutOfBoundException(dstIndex);

			this->_ensureStorageSize(this->length() + length);

			for (size_t idx = this->length() ; idx > dstIndex ; idx--) {
				this->_set(idx + length - 1, this->_get(idx - 1));
			}

			for (size_t idx = 0 ; idx < length ; idx++) {
				this->_set(idx + dstIndex, items[idx]);
			}

			this->_length += length;

		}

		inline void insertItemAtIndex(T item, size_t dstIndex) noexcept(false) {
			this->insertItemsAtIndex(&item, 1, dstIndex);
		}

		inline const T* items() const {
			return *_storage + _offset;
		}

		inline size_t length() const {
			return this->_length;
		}

		inline size_t size() const {
			return this->_size;
		}

		size_t count(function<bool(T& item)> tester) const {
			size_t result = 0;
			this->forEach([&tester,&result](T& item) {
				if (tester(item)) result++;
			});
			return result;
		}

		size_t count(const T& item) const {
			return this->count([&item](const T& other) {
				return item == other;
			});
		}

		T itemAtIndex(size_t index) const noexcept(false) {
			if (index >= this->length()) throw OutOfBoundException(index);
			return this->_get(index);
		}

		inline T operator[](size_t index) const noexcept(false) {
			return itemAtIndex(index);
		}

		T first() const noexcept(false) {
			if (this->length() == 0) {
				throw NotFoundException();
			}
			return this->_get(0);
		}

		T first(Tester tester) const noexcept(false) {
			for (size_t idx = 0 ; idx < this->length() ; idx++) {
				T current = this->_get(idx);
				if (tester(current)) return current;
			}
			throw NotFoundException();
		}

		T last() const noexcept(false) {
			if (this->length() == 0) {
				throw NotFoundException();
			}
			return this->_get(this->length() - 1);
		}

		inline size_t lastIndex() const {
			if (this->length() == 0) return NotFound;
			return this->length() - 1;
		}

		inline size_t lastIndex(Tester tester) const {
			return reduce<size_t>(NotFound, [&tester](size_t result, T& item, const size_t idx) {
				if (tester(item)) return idx;
				return result;
			});
		}

		bool needle(const Data<T>& other, size_t offset) const {

			if (other.length() + offset > this->length()) return false;

			for (size_t idx = 0 ; idx < other.length() ; idx++) {
				if (this->_get(offset + idx) != other._get(idx)) {
					return false;
				}
			}

			return true;

		}

		size_t indexOf(const Data<T>& other, size_t offset = 0) const {
			for (size_t index = offset ; index < this->length() ; index++) {
				if (this->needle(other, index)) return index;
			}
			return NotFound;
		}

		inline size_t indexOf(T other, size_t offset = 0) const {
			return indexOf(Data<T>(&other, 1), offset);
		}

		size_t lastIndexOf(const Data<T>& other) const {
			for (ssize_t index = this->length() - other.length(); index >= 0 ; index--) {
				if (this->needle(other, (size_t)index)) return index;
			}
			return NotFound;
		}

		inline bool contains(const Data<T>& other, size_t offset = 0) const {
			return indexOf(other, offset) != NotFound;
		}

		inline bool contains(T other, size_t offset = 0) const {
			return indexOf(other, offset) != NotFound;
		}

		bool hasPrefix(const Data<T>& other) const {
			if (other.length() > this->length()) return false;
			return this->subdata(0, other.length())
				->equals(other);
		}

		bool hasSuffix(const Data<T>& other) const {
			if (other.length() > this->length()) return false;
			return this->subdata(this->length() - other.length(), other.length())
				->equals(other);
		}

		T replace(T element, size_t index) {
			this->_ensureStorageOwnership();
			T removed = this->_get(index);
			this->_set(index, element);
			return removed;
		}

		T replace(size_t index, function<T(const T&)> todo) {
			this->_ensureStorageOwnership();
			T oldValue = this->itemAtIndex(index);
			T newValue = todo(oldValue);
			this->replace(newValue, index);
			return oldValue;
		}

		Strong<Data<T>> subdata(size_t offset, size_t length = NotFound) const {
			if (length == NotFound) length = this->_length;
			length = (size_t)math::min((ssize_t)this->length() - (ssize_t)offset, (ssize_t)length);
			return Strong<Data<T>>(*this, offset, length);
		}

		Strong<Data<T>> slicing(ssize_t start = 0, ssize_t end = math::limit<ssize_t>()) const {

			if (start < 0) {
				start = (this->length() - 1) + start;
			}

			if (end < 0) {
				end = (this->length() - 1) + end;
			}

			end = math::min<ssize_t>((ssize_t)this->length() - 1, math::max<ssize_t>(-1, end));
			start = math::max<ssize_t>(0, math::min<ssize_t>((ssize_t)this->length(), start));

			return Strong<Data<T>>(*this, start, math::min<ssize_t>((ssize_t)this->length(), end - start + 1));

		}

		Strong<Data<T>> remove(size_t offset, size_t length) {

			if (offset + length > this->length()) throw OutOfBoundException(offset + length);

			this->_ensureStorageOwnership();

			Strong<Data<T>> result = this->subdata(offset, length);

			size_t moveCount = this->length() - (offset + length);

			for (size_t idx = 0 ; idx < moveCount ; idx++) {
				this->_set(offset + idx, this->_get(offset + length + idx));
			}

			this->_length -= length;

			return result;

		}

		Strong<Data<T>> reversed() const {
			Strong<Data<T>> result;
			for (size_t idx = this->length() ; idx > 0 ; idx--) {
				result->append(this->_get(idx - 1));
			}
			return result;
		}

		void drain() {
			Storage::release(&this->_storage);
			_storage = new Storage();
			_length = 0;
			_offset = 0;
		};

		size_t copy(void* bytes, size_t length, size_t offset = 0) {
			if (offset > this->length()) return 0;
			this->ensureStorageOwnership();
			length = math::min(length, this->length() - offset);
			memcpy(bytes, this->items(), sizeof(T) * length);
			return length;
		}

		inline Strong<Array<Data<T>>> split() const {
			return this->mapToArray<Data<T>>([](T item) {
				return Strong<Data<T>>(&item, 1);
			});
		}

		Strong<Array<Data<T>>> split(const Array<Data<T>>& separators, IncludeSeparator includeSeparator = IncludeSeparator::none, size_t max = 0) const {
			Strong<Array<Data<T>>> result;
			size_t idx = 0;
			while (result->count() < max - 1) {
				if (!separators.some([this,&idx,&result,&includeSeparator](const Data<T>& separator) {
					size_t next = NotFound;
					switch (includeSeparator) {
						case IncludeSeparator::none:
						case IncludeSeparator::suffix:
							next = indexOf(separator, idx);
							break;
						case IncludeSeparator::prefix:
						case IncludeSeparator::both:
							next = indexOf(separator, idx + separator.length());
							break;
					}
					if (next == NotFound) return false;
					switch (includeSeparator) {
						case IncludeSeparator::none:
						case IncludeSeparator::prefix:
							result->append(subdata(idx, next - idx));
							break;
						case IncludeSeparator::suffix:
						case IncludeSeparator::both:
							result->append(subdata(idx, next - idx + separator.length()));
							break;
					}
					switch (includeSeparator) {
						case IncludeSeparator::none:
						case IncludeSeparator::suffix:
							idx = next + separator.length();
							break;
						case IncludeSeparator::prefix:
						case IncludeSeparator::both:
							idx = next;
							break;
					}
					return true;
				})) break;
			}
			result->append(subdata(idx, length() - idx));
			return result;
		}

		inline Strong<Array<Data<T>>> split(const Data<T>& separator, IncludeSeparator includeSeparator = IncludeSeparator::none, size_t max = 0) const {
			return split(Array<Data<T>>(separator, 1), includeSeparator, max);
		}

		inline Strong<Array<Data<T>>> split(T* separator, size_t length, IncludeSeparator includeSeparator = IncludeSeparator::none, size_t max = 0) const {
			return split(Data<T>(separator, length), includeSeparator, max);
		}

		inline static Strong<Data<T>> join(const Array<Data<T>>& items, const Data<T>* separator) {
			return items.template reduce<Strong<Data<T>>>(Strong<Data<T>>(), [items, separator](Strong<Data<T>> result, Data<T>&, size_t idx) {
				result->append(items[idx]);
				if (separator != nullptr && idx != items.count() - 1) result->append(*separator);
				return result;
			});
		}

		inline static Strong<Data<T>> join(const Array<Data<T>>& items) {
			return join(items, nullptr);
		}

		inline static Strong<Data<T>> join(const Array<Data<T>>& items, const Data<T>& separator) {
			return join(items, &separator);
		}

		template<typename O>
		inline Strong<Data<O>> as() const {
			return Strong<Data<O>>((const O*)((T*)*this->_storage), (this->length() * sizeof(T)) / sizeof(O));
		}

		void forEach(function<void(T& item, size_t idx)> todo) const {
			for (size_t idx = 0 ; idx < this->length() ; idx++) {
				todo(this->_get(idx), idx);
			}
		}

		inline void forEach(function<void(T& item)> todo) const {
			this->forEach([&todo](T& item, size_t) {
				todo(item);
			});
		}

		template<typename R>
		R reduce(R initial, ReducerIndexStop<R> todo) const {
			R result = initial;
			bool stop = false;
			for (size_t idx = 0 ; idx < this->length() ; idx++) {
				result = todo(result, this->_get(idx), idx, &stop);
				if (stop) break;
			}
			return result;
		}

		template<typename R>
		R reduce(R initial, ReducerIndex<R> todo) const {
			return reduce<R>(initial, [&todo](R result, T item, size_t idx, bool*) {
				return todo(result, item, idx);
			});
		}

		template<typename R>
		inline R reduce(R initial, Reducer<R> todo) const {
			return reduce<R>(initial, [&todo](R result, T item, size_t) {
				return todo(result, item);
			});
		}

		Strong<Data<T>> filter(const TesterIndex& test) const {
			Strong<Data<T>> result;
			for (size_t idx = 0 ; idx < this->length() ; idx++) {
				if (test(this->_get(idx), idx)) result->append(this->_get(idx));
			}
			return result;
		}

		inline Strong<Data<T>> filter(const Tester& test) const {
			return filter([&test](T item, const size_t) {
				return test(item);
			});
		}

		template<typename O>
		Strong<Data<O>> map(const function<O(T item, size_t idx)>& transform) const {
			Strong<Data<O>> result;
			for (size_t idx = 0 ; idx < this->length() ; idx++) {
				result->append(transform(this->_get(idx), idx));
			}
			return result;
		}

		template<typename O>
		inline Strong<Data<O>> map(const function<O(T item)>& transform) const {
			return map<O>([&transform](T item, const size_t) {
				return transform(item);
			});
		}

		template<typename O>
		Strong<Array<O>> mapToArray(const function<O(T item, size_t idx)>& transform) const {
			Strong<Array<O>> result;
			for (size_t idx = 0 ; idx < this->length(); idx++) {
				result->append(transform(this->_get(idx), idx));
			}
			return result;
		}

		template<typename O>
		Strong<Array<O>> mapToArray(const function<O(T item)>& transform) const {
			return mapToArray<O>([&transform](T item, const size_t) {
				return transform(item);
			});
		}

		bool some(const TesterIndex& test, bool def = false) const {
			if (this->length() == 0) return def;
			for (size_t idx = 0 ; idx < this->length() ; idx++) {
				if (test(this->_get(idx), idx)) return true;
			}
			return false;
		}

		inline bool some(const Tester& test, bool def = false) const {
			return some([&test](T item, const size_t) {
				return test(item);
			}, def);
		}

		bool every(const TesterIndex& test, bool def = true) const {
			if (this->length() == 0) return def;
			return !this->some([&test](T item, const size_t idx) {
				return !test(item, idx);
			});
		}

		inline bool every(const Tester& test, bool def = true) const {
			return every([&test](T item, const size_t) {
				return test(item);
			}, def);
		}

		virtual uint64_t hash() const override {
			if (_hashIsDirty) {
				Hashable::Builder builder;
				for (size_t idx = 0 ; idx < this->length() ; idx++) {
					builder.add(this->hashForItem(this->_get(idx)));
				}
				_hash = builder;
				_hashIsDirty = false;
			}
			return _hash;
		}

		virtual Kind kind() const override {
			return Kind::data;
		}

		bool equals(const Data<T>& other) const {
			if (this->length() != other.length()) return false;
			for (size_t idx = 0 ; idx < this->length() ; idx++) {
				if (this->_get(idx) != other._get(idx)) return false;
			}
			return true;
		}

		bool operator==(const Type& other) const override {
			if (other.kind() != Kind::data) return false;
			if (this->_size != ((const Data<>&)other).size()) return false;
			return this->equals((const Data<T>&)other);
		}

		bool operator>(const Data<T>& other) const override {
			for (size_t idx = 0 ; idx < this->length() ; idx++) {
				if (idx >= other.length()) return true;
				T left = this->_get(idx);
				T right = other._get(idx);
				if (left < right) return false;
				if (left > right) return true;
			}
			return false;
		}

		Data& operator=(const Data<T>& other) {
			Storage::release(&this->_storage);
			this->_storage = other._storage->retain();
			this->_length = other._length;
			this->_size = other._size;
			this->_offset = other._offset;
			this->_hashIsDirty = other._hashIsDirty;
			this->_hash = other._hash;
			Type::operator=(other);
			return *this;
		}

		Data& operator=(Data<T>&& other) {
			Storage::release(&this->_storage);
			this->_storage = other._storage;
			this->_length = other._length;
			this->_size = other._size;
			this->_offset = other._offset;
			this->_hashIsDirty = other._hashIsDirty;
			this->_hash = other._hash;
			other._storage = nullptr;
			other._length = 0;
			other._size = 0;
			other._offset = 0;
			other._hashIsDirty = true;
			Type::operator=(std::move(other));
			return *this;
		}

	protected:

		virtual inline uint64_t hashForItem(const T&) const {
			return 0;
		}

	private:

		class Storage: public Allocator {

		public:
			Storage(const size_t length = 0) : _ptr(nullptr), _length(0), _retainCount(1) {
				this->ensureStorageSize(length);
			}

			~Storage() {
				if (this->_ptr != nullptr) {
					free(this->_ptr);
					this->_ptr = nullptr;
				}
			}

			Storage* retain() const {
				this->_retainCount++;
				return (Storage*)this;
			}

			static bool own(Storage** store, size_t length, size_t offset = 0) {
				bool replaced = false;
				if (*store != nullptr) *store = (*store)->own(length, offset, &replaced);
				return replaced;
			}

			inline static void release(Storage** store) {
				if (*store != nullptr) *store = (*store)->release();
			}

			inline T& get(size_t index) const {
				return this->_ptr[index];
			}

			inline T& set(size_t index, const T& value) {
				return this->_ptr[index] = value;
			}

			inline operator T*() const {
				return this->_ptr;
			}

			void ensureStorageSize(size_t length) {
				if (this->_length < length) {
					this->_length = Allocator::calculateBufferLength(length);
					this->_ptr = (T*) realloc(this->_ptr, sizeof(T) * this->_length);
				}
			}

		private:

			Storage* own(size_t length, size_t offset, bool* replaced) const {
				*replaced = false;
				if (this->_retainCount == 1) return (Storage*)this;
				this->release();
				*replaced = true;
				return new Storage(*this, offset, length);
			}

			Storage* release() const {
				this->_retainCount--;
				if (this->_retainCount == 0) {
					delete(this);
				}
				return nullptr;
			}

			T* _ptr;
			size_t _length;
			mutable std::atomic<size_t> _retainCount;

			Storage(const Storage& other, size_t offset, size_t length) : Storage(length) {
				for (size_t idx = 0 ; idx < length ; idx++) {
					this->_ptr[idx] = other._ptr[idx + offset];
				}
			}

		};

		Storage* _storage;
		size_t _offset;
		size_t _length;
		size_t _size;
		mutable bool _hashIsDirty;
		mutable uint64_t _hash;

		void _ensureStorageSize(size_t length) {
			if (this->_storage == nullptr) this->_storage = new Storage(length);
			else {
				this->_ensureStorageOwnership();
				this->_storage->ensureStorageSize(length);
			}
		}

		inline void _ensureStorageOwnership() {
			if (Storage::own(&this->_storage, this->_length, this->_offset)) {
				this->_offset = 0;
			}
		}

		inline size_t _index(size_t index) const {
			return this->_offset + index;
		}

		inline T& _get(size_t index) const {
			return this->_storage->get(this->_index(index));
		}

		inline T& _set(size_t index, const T& value) {
			return this->_storage->set(this->_index(index), value);
		}

	};

	template<typename T>
	class DataValue: public Data<T> {

	public:

		DataValue() : Data<T>() {}
		DataValue(const T* items, size_t length) : Data<T>(items, length) {}
		DataValue(size_t capacity) : Data<T>(capacity) {}
		DataValue(const DataValue& other) : Data<T>(other) {}
		DataValue(const Data<T>& other) : Data<T>(other) {}

		virtual ~DataValue() {}

		DataValue& operator=(const DataValue& other) {
			Data<T>::operator=(other);
			return *this;
		}

		DataValue& operator=(DataValue&& other) {
			Data<T>::operator=(std::move(other));
			return *this;
		}

	protected:

		virtual inline uint64_t hashForItem(const T& item) const override {
			return item;
		}

	};

}

#endif /* data_hpp */
