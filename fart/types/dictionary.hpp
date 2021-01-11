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
		Dictionary(const Dictionary<Key,Value>& other) : _keys(other._keys), _values(other._values) {}
		virtual ~Dictionary() {}

		void set(const Key& key, const Value& value) {
			ssize_t index = _keys.indexOf(key);
			if (index > -1) {
				_keys.replace(key, index);
				_values.replace(value, index);
			} else {
				_keys.append(key);
				_values.append(value);
			}
		}

		Strong<Array<Key>> keys() const {
			return _keys;
		}

		Strong<Array<Value>> values() const {
			return _values;
		}

		const bool hasKey(const Key& key) const {
			return _keys.indexOf(key) > -1;
		}

		Strong<Value> get(const Key& key) const noexcept(false) {
			ssize_t keyIndex = _keys.indexOf(key);
			if (keyIndex == -1) {
				throw KeyNotFoundException<Key>(key);
			}
			return _values[keyIndex];
		}

		Strong<Value> get(const Key& key, const Value& defaultValue) const {
			if (!this->hasKey(key)) return defaultValue;
			return this->get(key);
		}

		Strong<Value> get(const Key& key, const bool store, const Value& defaultValue) {
			Strong<Value> value = this->get(key, defaultValue);
			if (store) this->set(key, value);
			return value;
		}

		Strong<Value> operator[](const Key& key) const noexcept(false) {
			return get(key);
		}

		const size_t count() const {
			return _keys.count();
		}

		void forEach(function<void(const Key&, Value&)> todo) const {
			for (size_t idx = 0 ; idx < _keys.count() ; idx++) {
				todo(_keys[idx], _values[idx]);
			}
		}

		template<typename OtherValue>
		Strong<OtherValue> transformValue(const Key& key, function<Strong<OtherValue>(Value&)> todo) const {
			if (!this->hasKey(key)) return nullptr;
			return todo(this->get(key));
		}

		Strong<Value> transformValue(const Key& key) const {
			return this->transformValue<Value>(key, [](Value& value) {
				return value;
			});
		}

		template<typename OtherKey>
		Strong<Dictionary<OtherKey, Value>> mapKeys(function<Strong<OtherKey>(const Key& key, Value& value)> todo) {
			Strong<Dictionary<OtherKey, Value>> result;
			this->_keys.forEach([&todo,&result,this](const Key& key) {
				Strong<Value> value = this->get(key);
				result->set(todo(key, value), value);
			});
			return result;
		}

		template<typename OtherValue>
		Strong<Dictionary<Key, OtherValue>> mapValues(function<Strong<OtherValue>(const Key& key, Value& value)> todo) {
			Strong<Dictionary<Key, OtherValue>> result;
			this->_keys.forEach([&todo,&result,this](const Key& key) {
				Strong<Value> value = this->get(key);
				result->set(key, todo(key, value));
			});
			return result;
		}

		virtual const uint64_t hash() const override {
			return Hashable::Builder()
			.add(_keys.hash())
			.add(_values.hash());
		}

		virtual const Kind kind() const override {
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

	};

}

#endif /* dictionary_hpp */
