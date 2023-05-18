//
//  comparable.hpp
//  fart
//
//  Created by Kristian Trenskow on 2023/01/21.
//  Copyright © 2018-2023 Kristian Trenskow. All rights reserved.
//

#ifndef comparable_hpp
#define comparable_hpp

#include "./comparison.hpp"
#include "./equatable.hpp"

namespace fart::types {

	template<typename T>
	class Comparable : public Equatable<T> {

		public:

			virtual bool operator>(const T& other) const = 0;

			inline bool operator<(const T& other) const {
				return !(other >= (T&)*this);
			}

			inline bool operator>=(const T& other) const {
				return !other.operator<((T&)*this);
			}

			inline bool operator<=(const T& other) const {
				return !this->operator>(other);
			}

			Comparison compareTo(const T& other) const {
				if (this->operator>(other)) return Comparison::biggerThan;
				if (other.operator>((T&)*this)) return Comparison::lessThan;
				return Comparison::equals;
			}

	};

}

#endif /* comparable_hpp */
