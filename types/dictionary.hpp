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
		virtual ~Dictionary() {}

		void set(const Key& key, const Value& value) {
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

		Array<Key> keys() const {
			return Array(_keys);
		}

		Array<Value> values() const {
			return Array(_values);
		}

		bool hasKey(const Key& key) const {
			return _keys.indexOf(key) != NotFound;
		}

		Value& get(const Key& key) const noexcept(false) {
			size_t keyIndex = _keys.indexOf(key);
			if (keyIndex == NotFound) {
				throw KeyNotFoundException<Key>(key);
			}
			return _values[keyIndex];
		}

		Value& get(const Key& key, const Value& defaultValue) const {
			if (!this->hasKey(key)) return defaultValue;
			return this->get(key);
		}

		Value& get(const Key& key, const bool store, const Value& defaultValue) {
			Strong<Value> value = this->get(key, defaultValue);
			if (store) this->set(key, value);
			return value;
		}

		Value& operator[](const Key& key) const noexcept(false) {
			return get(key);
		}

		size_t count() const {
			return _keys.count();
		}

		void forEach(function<void(const Key&, Value&)> todo) const {
			for (size_t idx = 0 ; idx < _keys.count() ; idx++) {
				todo(_keys[idx], _values[idx]);
			}
		}

		template<typename OtherValue>
		OtherValue transformValue(const Key& key, function<OtherValue(Value&)> todo) const {
			if (!this->hasKey(key)) return nullptr;
			return todo(this->get(key));
		}

		Value transformValue(const Key& key) const {
			return this->transformValue<Value>(key, [](Value& value) {
				return value;
			});
		}

		Dictionary<Key, Value> filter(function<bool(const Key& key, const Value& value)> todo) {
			Dictionary<Key, Value> result;
			for (size_t idx = 0 ; idx < this->_keys.count() ; idx++) {
				Strong<Key> key = this->_keys[idx];
				Strong<Value> value = this->_values[idx];
				if (todo(key, value)) result.set(key, value);
			}
			return result;
		}

		template<typename OtherKey>
		Dictionary<OtherKey, Value> mapKeys(function<OtherKey(const Key& key, Value& value)> todo) {
			Dictionary<OtherKey, Value> result;
			this->_keys.forEach([&todo,&result,this](const Key& key) {
				Strong<Value> value = this->get(key);
				result.set(todo(key, value), value);
			});
			return result;
		}

		template<typename OtherValue>
		Dictionary<Key, OtherValue> mapValues(function<OtherValue(const Key& key, Value& value)> todo) {
			Dictionary<Key, OtherValue> result;
			this->_keys.forEach([&todo,&result,this](const Key& key) {
				Strong<Value> value = this->get(key);
				result.set(key, todo(key, value));
			});
			return result;
		}

		virtual uint64_t hash() const override {
			return Builder()
				.add(_keys.hash())
				.add(_values.hash());
		}

		virtual Kind kind() const override {
			return Kind::dictionary;
		}

		bool operator==(const Dictionary<Key,Value>& other) const {
			if (!Type::operator==(other)) return false;
			if (_keys.count() != other._keys.count) return false;
			for (size_t idx = 0 ; idx < _keys.count() ; idx++) {
				if (_keys[idx] != other._keys[idx] || _values[idx] != other._values[idx]) {
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
