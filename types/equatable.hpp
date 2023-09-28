//
// equatable.hpp
// fart
//
// Created by Kristian Trenskow on 2023/01/21.
// See license in LICENSE.
//

#ifndef equatable_hpp
#define equatable_hpp

namespace fart::types {

	template<typename T>
	class Equatable {

		public:

			virtual bool operator==(const T& other) const = 0;

			inline bool operator!=(const T& other) const {
				return !(this->operator==(other));
			}

	};

}

#endif /* equatable_hpp */
