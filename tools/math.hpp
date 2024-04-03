//
// math.hpp
// fart
//
// Created by Kristian Trenskow on 2020/04/05.
// See license in LICENSE.
//

#ifndef math_hpp
#define math_hpp

#include <string.h>

namespace fart::tools {

	namespace math {

		template<typename T>
		inline const T max(T a, T b) {
			return (a > b ? a : b);
		}

		template<typename T>
		inline const T min(T a, T b) {
			return (a < b ? a : b);
		}

		template<typename T>
		inline const T limit(bool sign = false) {
			T result;
			memset(&result, 0xFF, sizeof(T));
			if (sign) {
				result >>= 1;
			}
			return result;
		}

	}

}

#endif /* math_hpp */
