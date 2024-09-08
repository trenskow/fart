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

			RegularExpression(
				const String& pattern
			) noexcept(false) : _pattern(pattern) {

				pattern.withCString([](const char* pattern) {
					printf("%s\n", pattern);
				});

				int flags = REG_EXTENDED;

				if (pattern.length() < 2 || *pattern.substring(0, 1) != String("/")) {
					throw InvalidPatternException();
				}

				size_t lastSlash = pattern.lastIndexOf("/");

				if (lastSlash == NotFound || lastSlash == 0) {
					throw InvalidPatternException();
				}

				String options = pattern.substring(lastSlash + 1);
				String regexPattern = pattern.substring(1, lastSlash - 1);

				options.withCString([](const char* options) {
					printf("%s\n", options);
				});

				regexPattern.withCString([](const char* pattern) {
					printf("%s\n", pattern);
				});

				if (options.contains("i")) {
					flags |= REG_ICASE;
				}

				int ret = regexPattern
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
