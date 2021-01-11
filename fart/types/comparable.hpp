//
//  comparable.hpp
//  fart
//
//  Created by Kristian Trenskow on 09/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef comparable_hpp
#define comparable_hpp

#include "../exceptions/exception.hpp"

using namespace fart::exceptions;

namespace fart::types {

	template<class T>
	class Comparable {

	public:

		virtual const bool operator>(const T& other) const {
			throw NotImplementedException();
		}

		virtual const bool operator==(const T& other) const {
			throw NotImplementedException();
		}

		virtual const bool operator<(const T& other) const {
			return !this->operator>(other) && !this->operator==(other);
		}

	};

}

#endif /* comparable_hpp */
