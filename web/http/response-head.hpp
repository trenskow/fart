//
//  response.hpp
//  fart
//
//  Created by Kristian Trenskow on 12/12/2019.
//  Copyright © 2019 Kristian Trenskow. All rights reserved.
//

#ifndef response_hpp
#define response_hpp

#include "./head.hpp"
#include "../message.hpp"
#include "./version.hpp"

namespace fart::web::http {

	class ResponseHead : public Head {

	public:

		enum class Status {
			ok = 200
		};

		ResponseHead() : _version(Version::HTTP1_1), _status(Status::ok) {}

		ResponseHead(Data<uint8_t>& data) : Head(data) {
			_version = parseVersion((*_parts)[0]);
			try { _status = (Status)(*_parts)[1]->doubleValue(); }
			catch (DecoderException) { throw DataMalformedException(); }
		}

		ResponseHead(const ResponseHead& other) : _version(other._version), _status(other._status) {}

		virtual ~ResponseHead() {}

		Version version() const {
			return _version;
		}

		Status status() const {
			return _status;
		}

	protected:

		virtual Strong<Data<uint8_t>> headData(const Data<uint8_t>& lineBreak) const {

			Strong<Data<uint8_t>> result = MessageHead::headData(lineBreak);

			result->append(Head::versionData(_version));
			result->append(' ');
			result->append(String::format("%d", _status)->UTF8Data());
			result->append(' ');

			switch (_status) {
				case Status::ok:
					result->append(String("OK").UTF8Data());
					break;
			}

			result->append(lineBreak);

			return result;

		}


	private:

		Version _version;
		Status _status;

	};

	typedef Message<ResponseHead> HTTPResponse;

}

#endif /* response_hpp */
