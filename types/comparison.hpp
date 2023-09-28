//
// comparison.hpp
// fart
//
// Created by Kristian Trenskow on 2023/01/21.
// See license in LICENSE.
//

#ifndef comparison_hpp
#define comparison_hpp

namespace fart::types {

	enum class Comparison : int8_t {
		lessThan = -1,
		equals = 0,
		biggerThan = 1
	};

};

#endif /* comparison_hpp */
