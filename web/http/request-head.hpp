//
// request-head.hpp
// fart
//
// Created by Kristian Trenskow on 2019/12/12.
// See license in LICENSE.
//

#ifndef request_head_hpp
#define request_head_hpp

#include "../message.hpp"
#include "./head.hpp"
#include "../../types/url.hpp"
#include "./version.hpp"

using namespace fart::types;
using namespace fart::web;

namespace fart::web::http {

	class RequestHead : public Head {

	public:

		enum class Method {
			head = 0,
			get,
			post,
			put,
			del
		};

		RequestHead() : _version(Version::HTTP1_1), _method(Method::get), _path("/") {}

		RequestHead(Data<uint8_t>& data) : Head(data) {

			if (*_parts[0] == "HEAD") _method = Method::head;
			else if (*_parts[0] == "GET") _method = Method::get;
			else if (*_parts[0] == "POST") _method = Method::post;
			else if (*_parts[0] == "PUT") _method = Method::put;
			else if (*_parts[0] == "DELETE") _method = Method::del;
			else throw MethodNotSupportedException();

			_version = parseVersion((_parts)[2]);
			_path = URL::escapeDecode(_parts[1]);

		}

		RequestHead(const RequestHead& other) : _version(other._version), _method(other._method), _path(other._path) {}

		virtual ~RequestHead() {}

		Version version() const {
			return _version;
		}

		Method method() const {
			return _method;
		}

		const String path() const {
			return String(_path);
		}

	protected:

		virtual Data<uint8_t> headData(const Data<uint8_t> &lineBreak) const {

			Data<uint8_t> result = MessageHead::headData(lineBreak);

			switch (method()) {
				case Method::head:
					result.append(String("HEAD").UTF8Data());
					break;
				case Method::get:
					result.append(String("GET").UTF8Data());
					break;
				case Method::post:
					result.append(String("POST").UTF8Data());
					break;
				case Method::put:
					result.append(String("PUT").UTF8Data());
					break;
				case Method::del:
					result.append(String("DELETE").UTF8Data());
					break;
			}

			result.append(' ');
			result.append(_path->UTF8Data());
			result.append(' ');
			result.append(Head::versionData(_version));
			result.append(lineBreak);

			return result;

		}


	private:

		Version _version;
		Method _method;
		Strong<String> _path;

		void ensureSpaceAt(Data<uint8_t>& data, size_t index) const noexcept(false) {
			if (data.length() < index) throw DataIncompleteException();
			if (data[index]) throw DataMalformedException();
		}

	};

	typedef Message<RequestHead> HTTPRequest;

}

#endif /* request_head_hpp */
