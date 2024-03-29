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
		inline const T max(const T& a, const T& b) {
			return (a > b ? a : b);
		}

		template<typename T>
		inline const T min(const T& a, const T& b) {
			return (a < b ? a : b);
		}

		template<typename T>
		inline const T limit() {
			T result;
			memset(&result, 0xFF, sizeof(T));
			return result;
		}

	}

}

#endif /* math_hpp */
