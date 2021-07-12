
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

	template<typename T1, typename T2>
	class Couple: public Type {

	public:
		Couple(T1& first, T2& second) : Type(), _first(first), _second(second) { }
		Couple(const Couple<T1, T2>& other): Type(), _first(other._first), _second(other._second) { }

		T1& first() const {
			return _first;
		}

		T2& second() const {
			return _second;
		}

		virtual uint64_t hash() const override {
			return Builder()
				.add(_first->hash())
				.add(_second->hash());
		}

		virtual Kind kind() const override {
			return Kind::couple;
		}

		bool operator==(const Couple<T1, T2>& other) const {
			if (!Type::operator==(other)) return false;
			return _first == other._first && _second == other.second;
		}

		Couple<T1, T2>& operator=(const Couple<T1, T2>& other) {
			Type::operator=(other);
			_first = other._first;
			_second = other._second;
			return *this;
		}

	private:
		Strong<T1> _first;
		Strong<T2> _second;

	};

}

#endif /* couple_hpp */
