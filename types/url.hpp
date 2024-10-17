//
// url.hpp
// fart
//
// Created by Kristian Trenskow on 2019/12/12.
// See license in LICENSE.
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

	class URL : public Type {

	public:

		static String escapeDecode(const String& string) {
			String result;
			for (size_t idx = 0 ; idx < string.length() ; idx++) {
				uint32_t chr = string[idx];
				if (chr == '+') chr = ' ';
				else if (chr == '%') {
					if (idx >= string.length() - 2) throw UrlDecodingException(chr);
					chr = string.substring(idx + 1, 2)->hexData()->itemAtIndex(0);
					idx += 2;
				}
				result.append(chr);
			}
			return result;
		}

		static String escapeEncode(const String& string) {

			String result;

			Data<uint8_t> utf8Data = string.UTF8Data();

			for (size_t idx = 0 ; idx < utf8Data.length() ; idx++) {
				uint8_t codePoint = utf8Data[idx];
				if (!(codePoint >= 'a' && codePoint <= 'z') && !(codePoint >= 'A' && codePoint < 'Z') && !(codePoint >= '0' && codePoint <= '9')) {
					result.append(String::fromHex(Data<uint8_t>({ codePoint }))
						->mapCString<Strong<String>>([](const char* hex) {
							return String::format("%%%s", hex);
						}));
				} else {
					result.append(String(Data<uint8_t>({ codePoint })));
				}
			}

			return result;

		}

		static Strong<URL> cwd() {
			return fromString(
				String::fromCString([](char* buffer, size_t length) {
					getcwd(buffer, length);
					return strlen(buffer);
				}, 4096)
				.mapCString<String>([](const char* cwd) {
					return String::format("file://%s", cwd);
				}));
		}

		static Strong<URL> fromString(
			const String& urlString
		) noexcept(false) {

			Strong<String> string = urlString;

			size_t schemeDividerIdx = string->indexOf(":");

			if (schemeDividerIdx == NotFound) {
				throw URLMalformedException();
			}

			String scheme = string->substring(0, schemeDividerIdx);
			Strong<String> userInfo = nullptr;
			Strong<String> host = nullptr;
			uint16_t port = 0;
			String path;
			Array<Array<String>> query;
			Strong<String> fragment = nullptr;

			string = string->substring(schemeDividerIdx + 1);

			if (string->length() == 0) {
				throw URLMalformedException();
			}

			path = string;

			if ((*string)[0] == '/') {
				if (string->length() > 1 && (*string)[1] == '/') {

					string = string->substring(2);

					size_t authorityDividerIdx = string->indexOf("/");

					if (authorityDividerIdx == NotFound) {
						authorityDividerIdx = string->length();
					}

					String authority = string->substring(0, authorityDividerIdx);

					path = string->substring(authorityDividerIdx);

					size_t userInfoDividerIdx = authority.lastIndexOf("@");

					if (userInfoDividerIdx != NotFound) {
						authority = authority.substring(userInfoDividerIdx + 1);
						userInfo = string->substring(0, userInfoDividerIdx);
					}

					size_t portDividerIdx = authority.lastIndexOf(':');

					if (portDividerIdx != NotFound) {
						authority.substring(portDividerIdx + 1)
							->withCString([&](const char* portString) {
								if (sscanf(portString, "%hu", &port) == 0) {
									throw URLMalformedException();
								}
							});
						authority = authority.substring(0, portDividerIdx);
					}

					if (authorityDividerIdx > 0) {
						host = authority;
					}

				}

			}

			size_t fragmentDividerIdx = path.indexOf("#");

			if (fragmentDividerIdx != NotFound) {
				fragment = path.substring(fragmentDividerIdx + 1);
				path = path.substring(0, fragmentDividerIdx);
			}

			size_t queryDividerIdx = path.indexOf("?");

			if (queryDividerIdx != NotFound) {

				String queryString = path.substring(queryDividerIdx + 1);

				path = path.substring(0, queryDividerIdx);

				query = Array<Array<String>>(queryString
					.split("&")
					->map<Array<String>>([](const String& parameter) {
						return parameter.split("=")
							->map<String>([](const String& part) {
								return escapeDecode(part);
							});
					}));

			}

			if (userInfo != nullptr) {
				userInfo = escapeDecode(userInfo);
			}

			if (host != nullptr) {
				host = escapeDecode(host);
			}

			if (fragment != nullptr) {
				fragment = escapeDecode(fragment);
			}

			return Strong<URL>(
				scheme,
				userInfo,
				host,
				port,
				path,
				query,
				fragment);

		}

		URL() = delete;
		URL(const URL& other) : Type(), _scheme(other._scheme), _userInfo(*other._userInfo), _host(*other._host), _port(other._port), _path(other._path), _query(other._query), _fragment(*other._fragment) { }
		URL(
			const String& scheme,
			Strong<String> userInfo,
			Strong<String> host,
			uint16_t port,
			const String& path,
			Array<Array<String>> query,
			Strong<String> fragment
		) : _scheme(scheme),
		    _userInfo(userInfo != nullptr ? userInfo : nullptr),
		    _host(host != nullptr ? host : nullptr),
		    _port(port),
		    _path(path
		    	.split("/")
				->filter([](const String& component, size_t idx) {
					return idx == 0 || component.length() > 0;
				})
		    	->map<String>([](const String& component) {
		    		return escapeDecode(component);
		    	})),
		    _query(query),
		    _fragment(fragment != nullptr ? fragment : nullptr) { }

		virtual Kind kind() const override {
			return Kind::url;
		}

		const String& scheme() const {
			return this->_scheme;
		}

		const String* userInfo() const {
			return this->_userInfo;
		}

		const String* host() const {
			return this->_host;
		}

		uint16_t port() const {
			return this->_port ?: this->_schemaDefaultPort();
		}

		String path() const {
			return String::join(this->_path, "/");
		}

		const Array<String> queryKeys() const {
			return this->_query
				.map<String>([](const Array<String>& parts) {
					return parts[0];
				});
		}

		const Strong<String> valueForQueryKey(const String& key) const {

			Strong<Array<Array<String>>> queryParts = this->_query
				.filter([&](const Array<String>& parts) {
					return parts[0] == key;
				});

			if (queryParts->count() == 0) return nullptr;

			Strong<Array<String>> keyParts = (*queryParts)[0];

			if (keyParts->count() < 2) return nullptr;

			return (*keyParts)[1];

		}

		const Strong<String> fragment() const {
			return this->_fragment;
		}

		Strong<URL> appendingPath(const String& relativePath) const {
			return Strong<URL>(
				this->_scheme,
				this->_userInfo,
				this->_host,
				this->_port,
				String::join(relativePath
					.split("/")
					->filter([](const String& component, size_t idx) {
						return idx == 0 || component.length() > 0;
					})
					->reduce<Array<String>>(this->_path, [](Array<String> result, const String& component) {

						if (component.length() == 0) {
							result = Array<String>({ "" });
						}
						else if (component == String(Data<uint8_t>(component.length(), '.'))) {
							for (size_t idx = 1 ; result.count() > 1 && idx < component.length() ; idx++) {
								result = result.removingItemAtIndex(result.count() - 1);
							}
						}
						else {
							return *result.appending(component);
						}

						return result;

					}), "/"),
				this->_query,
				this->_fragment);
		}

		Strong<URL> relativeTo(const URL& baseUrl) const {

			if (this->_scheme != baseUrl._scheme) {
				return *this;
			}

			if (*this->_host.def(Strong<String>("")) != baseUrl._host.def(Strong<String>(""))) {
				return *this;
			}

			if (this->port() != baseUrl.port()) {
				return *this;
			}

			return Strong<URL>(
				this->_scheme,
				this->_userInfo,
				this->_host,
				this->_port,
				baseUrl.appendingPath(this->path())->path(),
				this->_query,
				this->_fragment);

		}

		virtual bool operator==(const Type& other) const override {
			if (other.kind() != Kind::url) return false;
			const URL& otherUrl = (const URL&)other;
			return this->_scheme == otherUrl._scheme &&
				(this->_userInfo == otherUrl._userInfo || (this->_userInfo != nullptr && otherUrl._userInfo != nullptr && *this->_userInfo == *otherUrl._userInfo)) &&
				(this->_host == otherUrl._host || (this->_host != nullptr && otherUrl._host != nullptr && *this->_host == *otherUrl._host)) &&
				this->_port == otherUrl._port &&
				this->_path == otherUrl._path &&
				this->_query == otherUrl._query &&
				(this->_fragment == otherUrl._fragment || (this->_fragment != nullptr && otherUrl._fragment != nullptr && *this->_fragment == *otherUrl._fragment));
		}

	private:

		uint16_t _schemaDefaultPort() const {
			if (this->_scheme == "http") return 80;
			if (this->_scheme == "https") return 443;
			if (this->_scheme == "ftp") return 21;
			if (this->_scheme == "telnet") return 23;
			if (this->_scheme == "ssh") return 22;
			return 0;
		}

		String _scheme;
		Strong<String> _userInfo;
		Strong<String> _host;
		uint16_t _port;
		Array<String> _path;
		Array<Array<String>> _query;
		Strong<String> _fragment;

	};

}

#endif /* url_hpp */
