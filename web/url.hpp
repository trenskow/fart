//
//  url.hpp
//  fart
//
//  Created by Kristian Trenskow on 12/12/2019.
//  Copyright © 2019 Kristian Trenskow. All rights reserved.
//

#ifndef url_hpp
#define url_hpp

#include "../memory/object.hpp"
#include "../memory/strong.hpp"
#include "../types/string.hpp"
#include "../exceptions/exception.hpp"

using namespace fart::types;
using namespace fart::memory;
using namespace fart::exceptions::web;

namespace fart::web {

	class Url : public Object {

	public:

		static String decode(const String& url) {
			String result;
			for (size_t idx = 0 ; idx < url.length() ; idx++) {
				uint32_t chr = url[idx];
				if (chr == '+') chr = ' ';
				else if (chr == '%') {
					if (idx >= url.length() - 2) throw UrlDecodingException(chr);
					chr = url.substring(idx + 1, 2).hexData()[0];
					idx += 2;
				}
				result.append(chr);
			}
			return result;
		}


	};

}

#endif /* url_hpp */
