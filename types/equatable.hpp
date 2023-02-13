//
//  equatable.hpp
//  fart
//
//  Created by Kristian Trenskow on 2023/01/21.
//  Copyright © 2018-2023 Kristian Trenskow. All rights reserved.
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
