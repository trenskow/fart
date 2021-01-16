//
//  key-value-pair.hpp
//  fart
//
//  Created by Kristian Trenskow on 01/04/2020.
//  Copyright © 2020 Kristian Trenskow. All rights reserved.
//

#ifndef key_value_pair_hpp
#define key_value_pair_hpp

#include "type.hpp"

namespace fart::types {

	template<class Key, class Value>
	class KeyValuePair: public Type {

		static_assert(std::is_base_of<Object, Key>::value);
		static_assert(std::is_base_of<Object, Value>::value);

	public:

		KeyValuePair(const Key& key, const Value& value) : _key(key), _value(value) {}
		KeyValuePair(const KeyValuePair<Key, Value>& other) : KeyValuePair(other._key, other._value) {}

		virtual Kind kind() const override {
			return Kind::keyValuePair;
		}

		bool operator==(const KeyValuePair<Key, Value>& other) const {
			return *this->_key == other._key && *this->_value == other._value;
		}

		virtual uint64_t hash() const override {
			return Hashable::Builder()
			.add(this->_key->hash())
			.add(this->_value->hash());
		}

	private:

		Strong<Key> _key;
		Strong<Value> _value;

	};

}

#endif /* key_value_pair_hpp */
