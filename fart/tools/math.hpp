//
//  math.hpp
//  fart
//
//  Created by Kristian Trenskow on 05/04/2020.
//  Copyright © 2020 Kristian Trenskow. All rights reserved.
//

#ifndef math_hpp
#define math_hpp

namespace fart::tools {

	namespace math {

		template<typename T>
		inline const T max(T a, T b) {
			return a > b ? a : b;
		}

		template<typename T>
		inline const T min(T a, T b) {
			return a < b ? a : b;
		}

	}

}

#endif /* math_hpp */
