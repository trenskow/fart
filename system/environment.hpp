//
//  environment.hpp
//  fart
//
//  Created by Kristian Trenskow on 16/01/2021.
//  Copyright © 2020 Kristian Trenskow. All rights reserved.
//

#ifndef environment_hpp
#define environment_hpp

#include <stdlib.h>

#include "../types/string.hpp"

namespace fart::system {

	class Environment {

	public:
		static Strong<String> getVariable(const String& name) {
			return Strong<String>(name.mapCString<const char*>([](const char* name) {
				return getenv(name);
			}));
		}

	};

}

#endif /* environment_hpp */