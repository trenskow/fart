//
//  type.hpp
//  fart
//
//  Created by Kristian Trenskow on 10/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef type_hpp
#define type_hpp

#include <stdint.h>

#include "../memory/object.hpp"
#include "./hashable.hpp"
#include "../exceptions/exception.hpp"
#include "../tools/math.hpp"

using namespace fart::memory;
using namespace fart::exceptions::types;
using namespace fart::tools;

namespace fart::types {

	static const size_t NotFound = math::limit<size_t>();

	class Type : public Object, public Hashable {

	public:

		enum class Kind : uint8_t {
			data,
			string,
			number,
			array,
			dictionary,
			date,
			couple,
			uuid,
			null
		};

		Type() : Object(), Hashable() { }
		Type(const Type& other) : Object(other), Hashable(other) { }
		Type(Type&& other) : Object(std::move(other)), Hashable(std::move(other)) { }

		virtual Kind kind() const {
			throw NotImplementedException();
		}

		Type& operator=(const Type& other) {
			Object::operator=(other);
			Hashable::operator=(other);
			return *this;
		}

		Type& operator=(Type&& other) {
			Object::operator=(std::move(other));
			Hashable::operator=(std::move(other));
			return *this;
		}

		virtual bool operator==(const Type& other) const {
			if (this->kind() != other.kind()) return false;
			return this->hash() == other.hash();
		}

		virtual bool operator==(const Type* other) const {
			if (other == nullptr) return false;
			return *this == *other;
		}

		virtual bool operator!=(const Type& other) const {
			return !(*this == other);
		}

		virtual bool operator!=(const Type* other) const {
			return !(*this == other);
		}

		bool is(Kind kind) const {
			return kind == this->kind();
		}

		template<class T>
		T& as() const {
			return (T&)*this;
		}

		template<class T>
		T& as(Kind kind) const {
			if (!this->is(kind)) throw TypeConversionException();
			return this->as<T>();
		}

	};

}

#endif /* type_hpp */
