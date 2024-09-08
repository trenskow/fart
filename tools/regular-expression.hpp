//
// regular-expression.hpp
// fart
//
// Created by Kristian Trenskow on 2020/04/05.
// See license in LICENSE.
//

#ifndef regular_expression_hpp
#define regular_expression_hpp

#include <regex.h>

#include "../types/string.hpp"
#include "../exceptions/exception.hpp"

using namespace fart::types;
using namespace fart::exceptions::tools;

namespace fart::tools {

	class RegularExpression : public Object {

		public:

			enum Options {
				IGNORE_CASE = 1 << 0
			};

			RegularExpression(
				const String& pattern,
				Options options = (Options)0
			) noexcept(false) : _pattern(pattern) {

				int flags = REG_EXTENDED;

				if (options & IGNORE_CASE) {
					flags |= REG_ICASE;
				}

				int ret = pattern
					.mapCString<int>([&](const char* pattern) {
						return regcomp(&this->_regex, pattern, flags);
					});

				if (ret) {
					throw InvalidPatternException();
				}

			}

			~RegularExpression() {
				regfree(&this->_regex);
			}

			String pattern() const noexcept {
				return this->_pattern;
			}

			bool test(
				const String& string
			) const noexcept {
				return string.mapCString<bool>([&](const char* string) {
					return regexec(&this->_regex, string, 0, nullptr, 0) == 0;
				});
			}

		private:

			String _pattern;
			regex_t _regex;

	};

}

#endif /* regular_expression_hpp */