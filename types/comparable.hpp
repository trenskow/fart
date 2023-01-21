//
//  data.hpp
//  fart
//
//  Created by Kristian Trenskow on 17/08/2018.
//  Copyright © 2018-2023 Kristian Trenskow. All rights reserved.
//

#ifndef comparable_hpp
#define comparable_hpp

namespace fart::types {

	template<typename T>
	class Comparable {

		public:

			virtual bool operator>(const T& other) const = 0;

			inline bool operator<(const T& other) const {
				return (other > (T&)*this);
			}

			inline bool operator==(const T& other) const {
				return !this->operator<(other) && !this->operator>(other);
			}

			inline bool operator!=(const T& other) const {
				return !this->operator==(other);
			}

			inline bool operator>=(const T& other) const {
				return !this->operator<(other);
			}

			inline bool operator<=(const T& other) const {
				return !this->operator>(other);
			}

	};

}

#endif /* comparable_hpp */
