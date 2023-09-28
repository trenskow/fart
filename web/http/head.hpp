//
// head.hpp
// fart
//
// Created by Kristian Trenskow on 2019/12/14.
// See license in LICENSE.
//

#ifndef head_hpp
#define head_hpp

#include "../../memory/object.hpp"
#include "../../memory/strong.hpp"
#include "../../types/data.hpp"
#include "../../types/string.hpp"
#include "../../types/array.hpp"
#include "./version.hpp"
#include "../message.hpp"

using namespace fart::memory;
using namespace fart::types;

namespace fart::web::http {

	class Head : public MessageHead {

	public:

		Head() {}

		Head(Data<uint8_t>& data) {

			Data<uint8_t> lineBreak = Message<Head>::dataForLineBreakMode(Message<Head>::determineLineBreakMode(data));
			Data<uint8_t> head = data.remove(0, data.indexOf(lineBreak) + lineBreak.length());

			head.remove(head.length() - lineBreak.length(), lineBreak.length());

			Array<Data<uint8_t>> parts = head.split((uint8_t*)" ", 1);

			if (parts.count() != 3) throw DataMalformedException();

			_parts = parts.map<String>([](const Data<uint8_t>& current) {
				return String(current);
			});

		}

		Head(const Head& other) : _parts(other._parts) {}

		virtual ~Head() {}

	protected:

		static Version parseVersion(String& version) {
			if (version == "HTTP/1.0") return Version::HTTP1_0;
			else if (version == "HTTP/1.1") return Version::HTTP1_1;
			throw VersionNotSupportedException();
		}

		static Data<uint8_t> versionData(Version version) {
			switch (version) {
				case Version::HTTP1_0:
					return String("HTTP/1.0").UTF8Data();
				case Version::HTTP1_1:
					return String("HTTP/1.1").UTF8Data();
			}
		}

		Array<String> _parts;

	};

}

#endif /* head_hpp */
