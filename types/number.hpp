//
//  number.hpp
//  fart
//
//  Created by Kristian Trenskow on 09/09/2018.
//  Copyright © 2018-2023 Kristian Trenskow. All rights reserved.
//

#ifndef number_hpp
#define number_hpp

#include <type_traits>

#include "./type.hpp"
#include "./comparable.hpp"

namespace fart::types {

	enum class Subtype {
		boolean,
		floatingPoint,
		integer
	};

	template<typename T>
	class Number : public Type, public Comparable<Number<T>> {

	private:
		Subtype _subtype;
		T _value;

	public:

		using Comparable<Number<T>>::operator==;

		static T getValue(const Number<T>& number) {
			switch (number.subType()) {
				case Subtype::integer:
					return (T)number.template as<Number<int64_t>>().value();
				case Subtype::floatingPoint:
					return (T)number.template as<Number<double>>().value();
				case Subtype::boolean:
					return (T)(number.template as<Number<bool>>().value() != 0);
				default:
					throw TypeConversionException();
			}
		}

		static T getValue(const Type& value) noexcept(false) {
			if (!value.is(Type::Kind::number)) throw TypeConversionException();
			return Number<T>::getValue(value.as<Number<T>>());
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
		Number(const Number<N>& other) : Type(), _subtype(other.subType()), _value(other.value()) {}

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

		virtual bool operator>(const Number<T>& other) const override {
			return _value > other._value;
		}

		operator T() {
			return value();
		}

	};

	class Boolean: public Number<bool> {

	public:
		Boolean(const bool value) : Number(value, Subtype::boolean) {}
		virtual ~Boolean() {}
	};

	class Float: public Number<double> {

	public:
		Float(const double value) : Number(value, Subtype::floatingPoint) {}
		virtual ~Float() {}

	};

	class Integer: public Number<int64_t> {

	public:
		Integer(const int64_t value) : Number(value, Subtype::integer) {}
		virtual ~Integer() {}

	};

}

#endif /* number_hpp */
