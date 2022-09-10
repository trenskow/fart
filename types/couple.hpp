
//
//  couple.hpp
//  fart
//
//  Created by Kristian Trenskow on 24/01/2021.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef couple_hpp
#define couple_hpp

namespace fart::types {

	template<typename First, typename Second>
	class Couple: public Type {

	public:
		Couple(First& first, Second& second) : Type(), _first(first), _second(second) { }
		Couple(const Couple<First, Second>& other): Type(), _first(other._first), _second(other._second) { }

		Strong<First> first() const {
			return _first;
		}

		Strong<Second> second() const {
			return _second;
		}

		virtual uint64_t hash() const override {
			return Builder()
				.add(*_first)
				.add(*_second);
		}

		virtual Kind kind() const override {
			return Kind::couple;
		}

		bool operator==(const Couple<First, Second>& other) const {
			if (!Type::operator==(other)) return false;
			return _first == other._first && _second == other.second;
		}

		Couple<First, Second>& operator=(const Couple<First, Second>& other) {
			Type::operator=(other);
			_first = other._first;
			_second = other._second;
			return *this;
		}

	private:
		Strong<First> _first;
		Strong<Second> _second;

	};

}

#endif /* couple_hpp */
