//
//  number.hpp
//  fart
//
//  Created by Kristian Trenskow on 09/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef number_hpp
#define number_hpp

#include "./type.hpp"

namespace fart::types {

	enum class Subtype {
		boolean,
		floatingPoint,
		integer
	};

	template<typename T>
	class Number : public Type {

	private:
		T _value;
		Subtype _subType;

	public:

		Number() : _value(0), _subType(Subtype::floatingPoint) {}
		Number(const T value, const Subtype subType = Subtype::integer) : _value(value), _subType(subType) {}
		Number(const Number<T>& other, const Subtype subType = Subtype::integer) : Number(other._value, subType) {}
		template<typename N>
		Number(const Number<N>& other) : _value(other.value()), _subType(other.subType()) {}
		virtual ~Number() {}

		operator T() const {
			return _value;
		}

		template<typename N>
		operator Number<N>() const {
			return Number<N>(_value);
		}

		const T value() const {
			return _value;
		}

		Subtype subType() const {
			return _subType;
		}

		virtual uint64_t hash() const override {
			return (uint64_t)_value;
		}

		virtual Kind kind() const override {
			return Kind::number;
		}

		bool operator==(const Number<T>& other) const {
			if (!Type::operator==(other)) return false;
			return _value == other._value;
		}

		bool operator>(const Number<T>& other) const {
			return _value > other._value;
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
