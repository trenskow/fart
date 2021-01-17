//
//  number.hpp
//  fart
//
//  Created by Kristian Trenskow on 09/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef number_hpp
#define number_hpp

#include <type_traits>

#include "./type.hpp"

namespace fart::types {

	enum class Subtype {
		boolean,
		floatingPoint,
		integer
	};

	template<typename T>
	class Number;

	typedef Number<bool> Boolean;
	typedef Number<double> Float;
	typedef Number<int64_t> Integer;

	template<typename T>
	class Number : public Type {

	private:
		Subtype _subtype;
		T _value;

	public:

		static T getValue(const Number<T>& number) {
			switch (number.subType()) {
				case Subtype::integer:
					return (T)number.as<Integer>().value();
				case Subtype::floatingPoint:
					return (T)number.as<Float>().value();
				case Subtype::boolean:
					return (T)(number.as<Boolean>().value() != 0);
			}
		}

		static T getValue(const Type& value) noexcept(false) {
			if (!value.is(Type::Kind::number)) throw TypeConversionException();
			return Number<uint64_t>::getValue(value.as<Number<uint64_t>>());
		}

		static bool is(const Type& value) {
			if (!value.is(Type::Kind::number)) return false;
			if (std::is_same<T, bool>::value) return value.as<Number<uint64_t>>().subType() == Subtype::boolean;
			if (std::is_same<T, double>::value) return value.as<Number<uint64_t>>().subType() == Subtype::floatingPoint;
			if (std::is_same<T, float>::value) return value.as<Number<uint64_t>>().subType() == Subtype::floatingPoint;
			return value.as<Number<uint64_t>>().subType() == Subtype::integer;
		}

		Number() : _value(0), _subtype(Subtype::floatingPoint) {}

		Number(const T value, const Subtype subType) : _subtype(subType), _value(value) {}

		Number(const T value) : _value(value) {
			if (std::is_same<T, bool>::value) _subtype = Subtype::boolean;
			if (std::is_same<T, double>::value) _subtype = Subtype::floatingPoint;
			if (std::is_same<T, float>::value) _subtype = Subtype::floatingPoint;
			_subtype = Subtype::integer;
		}

		Number(const Number<T>& other, const Subtype subType = Subtype::integer) : Number(other._value, subType) {}

		template<typename N>
		Number(const Number<N>& other) : _subtype(other.subType()), _value(other.value()) {}

		virtual ~Number() {}

		operator T() const {
			return _value;
		}

		template<typename N>
		operator Number<N>() const {
			return Number<N>(_value);
		}

		T value() const {
			return _value;
		}

		Subtype subType() const {
			return _subtype;
		}

		virtual uint64_t hash() const override {
			return (uint64_t)_value;
		}

		virtual Kind kind() const override {
			return Kind::number;
		}

		bool is(Subtype subtype) const {
			return _subtype == subtype;
		}

		bool operator==(const Number<T>& other) const {
			if (!Type::operator==(other)) return false;
			return _value == other._value;
		}

		bool operator>(const Number<T>& other) const {
			return _value > other._value;
		}

		operator T() {
			return value();
		}

	};

}

#endif /* number_hpp */
