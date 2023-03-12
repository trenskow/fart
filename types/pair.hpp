
//
//  pair.hpp
//  fart
//
//  Created by Kristian Trenskow on 2021/01/24.
//  Copyright © 2018-2023 Kristian Trenskow. All rights reserved.
//

#ifndef pair_hpp
#define pair_hpp

namespace fart::types {

	template<typename First, typename Second>
	class Pair: public Type {

		static_assert(std::is_base_of<Object, First>::value);
		static_assert(std::is_base_of<Hashable, First>::value);
		static_assert(std::is_base_of<Object, Second>::value);
		static_assert(std::is_base_of<Hashable, First>::value);

	public:

		static Type::Kind typeKind() {
			return Type::Kind::pair;
		}

		Pair(First& first, Second& second) : Type(), _first(first), _second(second) { }
		Pair(const Pair<First, Second>& other) : Type(), _first(other._first), _second(other._second) { }
		Pair(const std::pair<First&, Second&> pair) : Type(), _first(pair.first), _second(pair.second) { }

		First& first() const {
			return *_first;
		}

		void setFirst(const First& first) {
			_first = first;
		}

		Second& second() const {
			return *_second;
		}

		void setSecond(const Second& second) {
			_second = second;
		}

		virtual uint64_t hash() const override {
			return Builder()
				.add(*_first)
				.add(*_second);
		}

		virtual Kind kind() const override {
			return Kind::pair;
		}

		bool operator==(const Pair<First, Second>& other) const {
			if (!Type::operator==(other)) return false;
			return _first == other._first && _second == other.second;
		}

		Pair<First, Second>& operator=(const Pair<First, Second>& other) {
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

#endif /* pair_hpp */
