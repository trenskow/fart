//
//  message.hpp
//  fart
//
//  Created by Kristian Trenskow on 11/12/2019.
//  Copyright © 2019 Kristian Trenskow. All rights reserved.
//

#ifndef message_hpp
#define message_hpp

#include "../memory/object.hpp"
#include "../memory/strong.hpp"
#include "../types/string.hpp"
#include "../types/data.hpp"
#include "../types/dictionary.hpp"
#include "../exceptions/exception.hpp"

using namespace fart::types;
using namespace fart::memory;
using namespace fart::exceptions::web;

namespace fart::web {

	class MessageHead : public Object {

	public:
		MessageHead() {}
		MessageHead(Data<uint8_t>& data) {}
		virtual ~MessageHead() {}

	protected:

		virtual Data<uint8_t> headData(const Data<uint8_t>& lineBreak) const {
			return Data<uint8_t>();
		}

	};

	template<class Head = MessageHead>
	class Message : public Head {

		static_assert(std::is_base_of<Object, MessageHead>::value);

	public:

		enum class LineBreakMode {
			crLf = 0,
			cr,
			lf
		};

		Message() : _lineBreakMode(LineBreakMode::crLf) {};

		Message(Data<uint8_t>& data) noexcept(false) : Head(data) {

			_lineBreakMode = determineLineBreakMode(data);

			Data<uint8_t> lineBreak = dataForLineBreakMode(_lineBreakMode);

			Data<uint8_t> deliminator(lineBreak);
			deliminator.append(lineBreak);

			Array<Data<uint8_t>> parts = data.split(deliminator, IncludeSeparator::none, 2);

			if (parts.count() < 1) {
				throw DataIncompleteException();
			}

			Array<Data<uint8_t>> header = parts[0].split(lineBreak);

			header.forEach([this](const Data<uint8_t>& current) {
				Data<uint8_t> keyValueSplitter((uint8_t *)": ", 2);
				Array<Data<uint8_t>> parts = current.split(keyValueSplitter);
				if (parts.count() != 2) throw DataIncompleteException();
				Array<String> strings = parts.map<String>([](const Data<uint8_t>& current) {
					return String(current);
				});
				_headers.set(strings[0], strings[1]);
			});

			if (_headers.hasKey("content-length")) {

				size_t length;

				try { length = _headers["content-length"].doubleValue(); }
				catch (const DecoderException&) { throw DataMalformedException(); }

				if (parts.count() < 2) throw DataIncompleteException();

				if (parts[1].count() < length) throw DataIncompleteException();
				if (parts[1].count() > length) throw DataMalformedException();

				_body = parts[1];

			}

			parts.forEach([&data](const Data<uint8_t>& current) {
				data.remove(0, current.count());
			});

			data.remove(0, deliminator.count());

		}

		Message(const Message<Head>& other) : Head(other), _lineBreakMode(other._lineBreakMode), _headers(other._headers) {}

		virtual ~Message() {}

		bool hasHeader(const String& key) const {
			return _headers.hasKey(key);
		}

		bool hasHeader(const char* key) const {
			return _headers.hasKey(String(key));
		}

		String headerValue(const char* key) const {
			return _headers[String(key)];
		}

		String headerValue(const String& key) const {
			return _headers[key];
		}

		void setHeaderValue(const String& key, String& value) {
			_headers.set(key, value);
		}

		Data<uint8_t> body() const {
			return _body;
		}

		void setBody(Data<uint8_t>& data) {
			setHeaderValue("Content-Length", String::format("%d", data.count()));
			_body = data;
		}

		Data<uint8_t> data() const {

			Data<uint8_t> result;
			Data<uint8_t> lineBreak = dataForLineBreakMode(lineBreakMode());

			result.append(Head::headData(lineBreak));

			_headers.forEach([&result,&lineBreak](const String& key, const String& value) {
				key.withCString([&result,&value](const char* key){
					value.withCString([&result,key](const char* value){
						result.append(String::format("%s: %s", key, value).UTF8Data());
					});
				});
				result.append(lineBreak);
			});

			result.append(lineBreak);

			if (_body != nullptr) {
				result.append(_body);
			}

			return result;

		}

		static LineBreakMode determineLineBreakMode(Data<uint8_t>& data) {

			if (data.indexOf(dataForLineBreakMode(LineBreakMode::crLf))) {
				return LineBreakMode::crLf;
			}
			else if (data.indexOf(dataForLineBreakMode(LineBreakMode::cr))) {
				return LineBreakMode::cr;
			}
			else if (data.indexOf(dataForLineBreakMode(LineBreakMode::lf))) {
				return LineBreakMode::lf;
			}
			else {
				throw DataIncompleteException();
			}

		}

		static Data<uint8_t> dataForLineBreakMode(LineBreakMode lineBreakMode) {
			switch (lineBreakMode) {
				case LineBreakMode::crLf:
					return Data<uint8_t>((uint8_t*)"\r\n", 2);
				case LineBreakMode::cr:
					return Data<uint8_t>((uint8_t*)"\r", 1);
				case LineBreakMode::lf:
					return Data<uint8_t>((uint8_t*)"\n", 1);
			}
		}

		const LineBreakMode lineBreakMode() const {
			return _lineBreakMode;
		}

	private:

		LineBreakMode _lineBreakMode;
		Dictionary<String, String> _headers;
		Data<uint8_t> _body;

	};

}

#endif /* message_hpp */
