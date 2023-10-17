//
// dictionary.hpp
// fart
//
// Created by Kristian Trenskow on 2018/09/09.
// See license in LICENSE.
//

#ifndef dictionary_hpp
#define dictionary_hpp

#include <thread>
#include "./array.hpp"
#include "./pair.hpp"
#include "./type.hpp"

namespace fart::types {

	template<typename Key, class Value = Type>
	class Dictionary : public Type {

		static_assert(std::is_base_of<Object, Key>::value);
		static_assert(std::is_base_of<Hashable, Key>::value);
		static_assert(std::is_base_of<Object, Value>::value);

	private:
		Array<Key> _keys;
		Array<Value> _values;

	public:

		static Type::Kind typeKind() {
			return Type::Kind::dictionary;
		}

		Dictionary() : Type(), _keys(), _values() {}
		Dictionary(const Dictionary<Key,Value>& other) : Type(), _keys(other._keys), _values(other._values) {}
		Dictionary(Dictionary<Key,Value>&& other) : Type(), _keys(std::move(other._keys)), _values(std::move(other._values)) {}

		Dictionary(const Pair<Key, Value>& keyValue) : Type(), _keys({ keyValue.first }), _values({ keyValue.second }) {}

		Dictionary(const Array<Pair<Key, Value>>& keyValues) : Type(), _keys(), _values() {
			_keys = keyValues.template map<Key>([](const Pair<Key, Value>& keyValue) {
				return Strong<Key>(keyValue.first());
			});
			_values = keyValues.template map<Value>([](const Pair<Key, Value>& keyValue) {
				return Strong<Value>(keyValue.second());
			});
		}

		Dictionary(std::initializer_list<std::pair<Key&, Value&>> keyValues) : Type(), _keys(), _values() {
			for (auto keyValue : keyValues) {
				_keys.append(keyValue.first);
				_values.append(keyValue.second);
			}
		}

		virtual ~Dictionary() {}

		void set(Strong<Key> key, Strong<Value> value) {
			size_t index = _keys.indexOf(key);
			if (index != NotFound) {
				_keys.replace(key, index);
				_values.replace(value, index);
			} else {
				_keys.append(key);
				_values.append(value);
			}
		}

		Strong<Dictionary<Key, Value>> setting(Strong<Key> key, Strong<Value> value) {
			Strong<Dictionary<Key, Value>> result;
			result->setting(*this);
			result->set(key, value);
			return result;
		}

		void set(const std::pair<Key&, Value&> keyValue) {
			set(keyValue.first, keyValue.second);
		}

		void set(const Dictionary<Key, Value>& dictionary) {
			dictionary.iterate()->forEach([&](const Pair<Key, Value>& element) {
				set(element.first(), element.second());
			});
		}

		Strong<Dictionary<Key, Value>> setting(const Dictionary<Key, Value>& dictionary) {
			Strong<Dictionary<Key, Value>> result;
			result->set(*this);
			result->set(dictionary);
			return result;
		}

		void remove(Strong<Key> key) {
			size_t index = _keys.indexOf(key);
			if (index == NotFound) {
				throw KeyNotFoundException();
			}
			_keys.removeItemAtIndex(index);
			_values.removeItemAtIndex(index);
		}

		inline Strong<Array<Key>> keys() const {
			return Strong<Array<Key>>(_keys);
		}

		inline Strong<Array<Value>> values() const {
			return Strong<Array<Value>>(_values);
		}

		inline bool hasKey(const Key& key) const {
			return _keys.indexOf(key) != NotFound;
		}

		Strong<Value> get(const Key& key) const noexcept(false) {
			size_t keyIndex = _keys.indexOf(key);
			if (keyIndex == NotFound) {
				throw KeyNotFoundException();
			}
			return _values[keyIndex];
		}

		Strong<Value> get(const Key& key, const Value& defaultValue) const {
			if (!this->hasKey(key)) return defaultValue;
			return this->get(key);
		}

		Strong<Value> get(const Key& key, const bool& store, const Value& defaultValue) {
			Strong<Value> value = this->get(key, defaultValue);
			if (store) this->set(key, value);
			return value;
		}

		inline Strong<Value> operator[](const Key& key) const noexcept(false) {
			return get(key);
		}

		inline size_t count() const {
			return _keys.count();
		}

		Strong<Array<Pair<Key, Value>>> keyValues() const {
			return this->map<Pair<Key, Value>>([](const Pair<Key, Value>& keyValue) {
				return keyValue;
			});
		}

		void forEach(const function<void(const Pair<Key, Value>&)>& todo) const {
			for (size_t idx = 0 ; idx < _keys.count() ; idx++) {
				todo(Pair<Key, Value>(_keys[idx], _values[idx]));
			}
		}

		template<typename OtherValue>
		OtherValue transformValue(const Key& key, const function<OtherValue(Value&)>& todo) const {
			if (!this->hasKey(key)) return nullptr;
			return todo(this->get(key));
		}

		inline Strong<Value> transformValue(const Key& key) const {
			return this->transformValue<Value>(key, [](Value& value) {
				return value;
			});
		}

		Strong<Dictionary<Key, Value>> filter(const function<bool(const Pair<Key, Value>&)>& todo) const {
			Strong<Dictionary<Key, Value>> result;
			for (size_t idx = 0 ; idx < this->_keys.count() ; idx++) {
				Strong<Key> key = this->_keys[idx];
				Strong<Value> value = this->_values[idx];
				if (todo(Pair<Key, Value>(key, value))) result->set(key, value);
			}
			return result;
		}

		template<typename OtherKey>
		Strong<Dictionary<OtherKey, Value>> mapKeys(const function<Strong<OtherKey>(const Pair<Key, Value>&)>& todo) const {
			Strong<Dictionary<OtherKey, Value>> result;
			this->_keys.forEach([&todo,&result,this](Key& key) {
				Strong<Value> value = this->get(key);
				result->set(todo(Pair<Key, Value>(key, value)), value);
			});
			return result;
		}

		template<typename OtherValue>
		Strong<Dictionary<Key, OtherValue>> mapValues(const function<Strong<OtherValue>(const Pair<Key, Value>&)>& todo) const {
			Strong<Dictionary<Key, OtherValue>> result;
			this->_keys.forEach([&todo,&result,this](const Key& key) {
				Strong<Value> value = this->get(key);
				result->set(key, todo(Pair<Key, Value>(key, value)));
			});
			return result;
		}

		template<typename OtherValue>
		Strong<Array<OtherValue>> map(const function<Strong<OtherValue>(const Pair<Key, Value>&)> todo) const {
			Strong<Array<OtherValue>> result;
			this->forEach([&todo,&result](const Pair<Key, Value>& element) {
				result->append(todo(element));
			});
			return result;
		}

		Strong<Array<Pair<Key, Value>>> iterate() const {
			return this->_keys.template map<Pair<Key, Value>>([&](Strong<Key> key, size_t idx) {
				return Strong<Pair<Key, Value>>(key, this->_values[idx]);
			});
		}

		virtual uint64_t hash() const override {
			auto builder = Builder();
			this->_keys.forEach([&](const Key& key) {
				builder.add(key);
			});
			return builder;
		}

		virtual Kind kind() const override {
			return Kind::dictionary;
		}

		bool operator==(const Dictionary<Key,Value>& other) const {

			if (!Type::operator==(other)) return false;

			if (_keys.count() != other._keys.count()) return false;

			if (_keys.some([&](const Key& key) {
				return !other._keys.contains(key);
			})) return false;

			if constexpr (std::is_base_of<Hashable, Value>::value) {
				return _keys.every([&](const Key& key) {
					return this->get(key) == other.get(key);
				});
			} else {
				return true;
			}

		}

		bool operator!=(const Dictionary<Key, Value>& other) const {
			return !(this->operator==(other));
		}

		virtual bool operator==(const Type& other) const override {
			if (!other.is(Type::Kind::dictionary)) return false;
			return this->operator==((const Dictionary<Key, Value>&)other);
		}

		virtual bool operator!=(const Type& other) const override {
			if (!other.is(Type::Kind::dictionary)) return true;
			return this->operator!=((const Dictionary<Key, Value>&)other);
		}

		Dictionary<Key, Value>& operator=(const Dictionary<Key, Value>& other) {
			Type::operator=(other);
			_keys = other._keys;
			_values = other._values;
			return *this;
		}

		Dictionary<Key, Value>& operator=(Dictionary<Key, Value>&& other) {
			Type::operator=(std::move(other));
			_keys = std::move(other._keys);
			_values = std::move(other._values);
			return *this;
		}

	};

}

#endif /* dictionary_hpp */
