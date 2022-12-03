//
//  dictionary.hpp
//  fart
//
//  Created by Kristian Trenskow on 09/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef dictionary_hpp
#define dictionary_hpp

#include <thread>
#include "./array.hpp"
#include "./couple.hpp"
#include "./type.hpp"

namespace fart::types {

	template<class Key, class Value>
	class Dictionary : public Type {

		static_assert(std::is_base_of<Object, Key>::value);
		static_assert(std::is_base_of<Hashable, Key>::value);
		static_assert(std::is_base_of<Object, Value>::value);

	private:
		Array<Key> _keys;
		Array<Value> _values;

	public:
		Dictionary() {}
		Dictionary(const Dictionary<Key,Value>& other) : Type(other), _keys(other._keys), _values(other._values) {}
		Dictionary(Dictionary<Key,Value>&& other) : Type(std::move(other)), _keys(std::move(other._keys)), _values(std::move(other._values)) {}

		Dictionary(const Couple<Key, Value>& keyValue) : Type(), _keys({ keyValue.first }), _values({ keyValue.second }) {}

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

		void remove(const Key& key) {
			size_t index = _keys.indexOf(key);
			if (index == NotFound) throw KeyNotFoundException<Key>(key);
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
				throw KeyNotFoundException<Key>(key);
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

		void forEach(const function<void(const Key&, Value&)>& todo) const {
			for (size_t idx = 0 ; idx < _keys.count() ; idx++) {
				todo(_keys[idx], _values[idx]);
			}
		}

		template<typename OtherValue>
		OtherValue transformValue(const Key& key, const function<OtherValue(Value&)>& todo) const {
			if (!this->hasKey(key)) return nullptr;
			return todo(this->get(key));
		}

		inline Value transformValue(const Key& key) const {
			return this->transformValue<Value>(key, [](Value& value) {
				return value;
			});
		}

		Strong<Dictionary<Key, Value>> filter(const function<bool(const Key& key, const Value& value)>& todo) {
			Strong<Dictionary<Key, Value>> result;
			for (size_t idx = 0 ; idx < this->_keys.count() ; idx++) {
				Strong<Key> key = this->_keys[idx];
				Strong<Value> value = this->_values[idx];
				if (todo(key, value)) result->set(key, value);
			}
			return result;
		}

		template<typename OtherKey>
		Strong<Dictionary<OtherKey, Value>> mapKeys(const function<OtherKey(const Key& key, Value& value)>& todo) {
			Strong<Dictionary<OtherKey, Value>> result;
			this->_keys.forEach([&todo,&result,this](const Key& key) {
				Strong<Value> value = this->get(key);
				result->set(todo(key, value), value);
			});
			return result;
		}

		template<typename OtherValue>
		Strong<Dictionary<Key, OtherValue>> mapValues(const function<OtherValue(const Key& key, Value& value)>& todo) {
			Strong<Dictionary<Key, OtherValue>> result;
			this->_keys.forEach([&todo,&result,this](const Key& key) {
				Strong<Value> value = this->get(key);
				result->set(key, todo(key, value));
			});
			return result;
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
			for (size_t idx = 0 ; idx < _keys.count() ; idx++) {
				if (*_keys[idx] != *other._keys[idx] || *_values[idx] != *other._values[idx]) {
					return false;
				}
			}
			return true;
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
