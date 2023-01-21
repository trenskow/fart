//
//  comparison.hpp
//  fart
//
//  Created by Kristian Trenskow on 21/01/2023.
//  Copyright © 2018-2023 Kristian Trenskow. All rights reserved.
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
