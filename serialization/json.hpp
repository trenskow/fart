//
//  json.hpp
//  fart
//
//  Created by Kristian Trenskow on 01/04/2020.
//  Copyright © 2020 Kristian Trenskow. All rights reserved.
//

#ifndef json_hpp
#define json_hpp

#include <sstream>

#include "../memory/object.hpp"
#include "../types/string.hpp"
#include "../exceptions/exception.hpp"
#include "../types/number.hpp"
#include "../types/null.hpp"
#include "../types/string.hpp"
#include "../types/dictionary.hpp"
#include "../types/duration.hpp"
#include "../types/date.hpp"
#include "../system/endian.hpp"

using namespace fart::memory;
using namespace fart::types;
using namespace fart::exceptions::serialization;

namespace fart::serialization {

	class JSON: Object {

	private:

		inline static bool _hasLength(const String& string, size_t* idx, size_t length) {
			return *idx + length <= string.length();
		}

		inline static void _ensureLength(const String& string, size_t* idx, size_t length, size_t line, size_t character) {
			if (!_hasLength(string, idx, length)) {
				throw JSONMalformedException(line, character);
			}
		}

		inline static void _ensureData(const String& string, size_t* idx, size_t line, size_t character) {
			return _ensureLength(string, idx, 1, line, character);
		}

		static void _offsetWhiteSpaces(const String& string, size_t* idx, size_t* line, size_t* character, bool ensure = true) {

			// First we ignore any whitespaces.
			while (string.length() > *idx && (string[*idx] == 0x20 || string[*idx] == 0x09 || string[*idx] == 0x0A || string[*idx] == 0x0D)) {
				if (string[*idx] == 0x0A) {
					(*line)++;
					(*character) = 0;
				} else {
					(*character)++;
				}
				(*idx)++;
			}

			if (ensure) _ensureData(string, idx, *line, *character);

		}

		static Strong<Type> _parseDictionary(const String& string, size_t* idx, size_t* line, size_t* character, size_t level) {

			if (string[*idx] != '{') throw JSONMalformedException(*line, *character);

			(*idx)++;
			(*character)++;

			_ensureData(string, idx, *line, *character);

			Strong<Dictionary<String, Type>> result;

			while (true) {
				_offsetWhiteSpaces(string, idx, line, character);
				if (string[*idx] == '}') break;
				if (string[*idx] == ',') {
					if (result->count() == 0) throw JSONMalformedException(*line, *character);
					(*idx)++;
					(*character)++;
					_offsetWhiteSpaces(string, idx, line, character);
				}
				Strong<String> key = _parseString(string, idx, line, character).as<String>();
				_offsetWhiteSpaces(string, idx, line, character);
				if (string[*idx] != ':') throw JSONMalformedException(*line, *character);
				(*idx)++;
				(*character)++;
				_offsetWhiteSpaces(string, idx, line, character);
				Strong<Type> value = _parse(string, idx, line, character, level + 1);
				result->set(key, value);
				_offsetWhiteSpaces(string, idx, line, character);
				if (string[*idx] != ',' && string[*idx] != '}') throw JSONMalformedException(*line, *character);
			}

			(*idx)++;
			(*character)++;

			return result.as<Type>();

		}

		static Strong<Type> _parseArray(const String& string, size_t* idx, size_t* line, size_t* character, size_t level) {

			if (string[*idx] != '[') throw JSONMalformedException(*line, *character);

			(*idx)++;
			(*character)++;

			_ensureData(string, idx, *line, *character);

			Strong<Array<Type>> result;

			while (true) {
				_offsetWhiteSpaces(string, idx, line, character);
				if (string[*idx] == ']') break;
				if (string[*idx] == ',') {
					if (result->count() == 0) throw JSONMalformedException(*line, *character);
					(*idx)++;
					(*character)++;
					_offsetWhiteSpaces(string, idx, line, character);
				} else if (result->count() > 0) throw JSONMalformedException(*line, *character);
				result->append(_parse(string, idx, line, character, level + 1));
				_offsetWhiteSpaces(string, idx, line, character);
				if (string[*idx] != ',' && string[*idx] != ']') throw JSONMalformedException(*line, *character);
			}

			(*idx)++;
			(*character)++;

			return result.as<Type>();

		}

		static Strong<Type> _parseNumber(const String& string, size_t* idx, size_t* line, size_t* character) {

			size_t consumed = 0;

			double value;
			try {
				value = string.doubleValue(*idx, &consumed, false);
				(*idx) += consumed;
				(*character) += consumed;
			} catch (const DecoderException& exception) {
				throw JSONMalformedException(*line, *character + exception.characterIndex());
			}

			if (round(value) == value) return Strong<Integer>(value).as<Type>();

			return Strong<Float>(value).as<Type>();

		}

		static Strong<Type> _parseString(const String& string, size_t* idx, size_t* line, size_t* character) {

			if (string[*idx] != '"') throw JSONMalformedException(*line, *character);

			(*idx)++;
			(*character)++;

			Data<uint16_t> stringBytes;

			do {

				if (string.length() == *idx) throw JSONMalformedException(*line, *character);
				if (string[*idx] == '"') break;

				switch (string[*idx]) {
					case '\b':
					case '\f':
					case '\n':
					case '\r':
					case '\t':
						throw JSONMalformedException(*line, *character);
					case '\\':
						(*idx)++;
						(*character)++;
						_ensureData(string, idx, *line, *character);
						switch (string[*idx]) {
							case 'b':
								stringBytes.append('\b');
								break;
							case 'f':
								stringBytes.append('\f');
								break;
							case 'n':
								stringBytes.append('\n');
								break;
							case 'r':
								stringBytes.append('\r');
								break;
							case 't':
								stringBytes.append('\t');
								break;
							case '"':
								stringBytes.append('"');
								break;
							case '\\':
								stringBytes.append('\\');
								break;
							case '/':
								stringBytes.append('/');
								break;
							case 'u': {
								_ensureLength(string, idx, 5, *line, *character);
								String code = string.substring(*idx + 1, 4);
								try {
									stringBytes.append(Endian::toSystemVariant(code.hexData().as<uint16_t>()[0], Endian::Variant::big));
								} catch (DecoderException&) {
									throw JSONMalformedException(*line, *character);
								} catch (OutOfBoundException&) {
									throw JSONMalformedException(*line, *character);
								}
								(*idx) += 4;
								(*character) += 4;
								break;
							}
							default:
								throw JSONMalformedException(*line, *character);
						}
						break;
					default:
						if (string[*idx] <= 0x0f) throw JSONMalformedException(*line, *character);
						stringBytes.append(string[*idx]);
				}

				(*idx)++;
				(*character)++;

			} while (true);

			(*idx)++;
			(*character)++;

			return Strong<String>(stringBytes, Endian::systemVariant()).as<Type>();

		}

		static Strong<Type> _parseLiteral(const String& string, size_t* idx, size_t* line, size_t* character) {

			static const String trueLiteral = "true";
			static const String falseLiteral = "false";
			static const String nullLiteral = "null";

			switch (string[*idx]) {
				case 't':
					_ensureLength(string, idx, 4, *line, *character);
					if (string.substring(*idx, 4) != trueLiteral) {
						throw JSONMalformedException(*line, *character);
					}
					(*idx) += 4;
					(*character) += 4;
					return Strong<types::Boolean>(true).as<Type>();
				case 'f':
					_ensureLength(string, idx, 5, *line, *character);
					if (string.substring(*idx, 5) != falseLiteral) {
						throw JSONMalformedException(*line, *character);
					}
					(*idx) += 5;
					(*character) += 5;
					return Strong<types::Boolean>(false).as<Type>();
				case 'n':
					_ensureLength(string, idx, 4, *line, *character);
					if (string.substring(*idx, 4) != nullLiteral) {
						throw JSONMalformedException(*line, *character);
					}
					(*idx) += 4;
					(*character) += 4;
					return Strong<Null>().as<Type>();
				default:
					throw JSONMalformedException(*line, *character);
			}
		}

		static Strong<Type> _parse(const String& string, size_t* idx, size_t* line, size_t* character, size_t level) {

			// Maliciously deeply nested JSON could trigger stack overflow.
			if (level > 32) throw JSONMalformedException(*line, *character);

			_offsetWhiteSpaces(string, idx, line, character);

			uint32_t chr = string[*idx];

			switch (chr) {
				case '{':
					return _parseDictionary(string, idx, line, character, level);
				case '[':
					return _parseArray(string, idx, line, character, level);
				case '"':
					return _parseString(string, idx, line, character);
				case 'n':
				case 't':
				case 'f':
					return _parseLiteral(string, idx, line, character);
					break;
				default:
					if (chr == '-' || (chr >= '0' && chr <= '9')) {
						return _parseNumber(string, idx, line, character);
					}
					throw JSONMalformedException(*line, *character);
			}

		}

	public:

		static Strong<Type> parse(const String& string) {

			size_t idx = 0;
			size_t line = 0;
			size_t character = 0;

			Strong<Type> result = nullptr;

			try {
				result = _parse(string, &idx, &line, &character, 0);
			} catch (const DecoderException& exception) {
				throw JSONMalformedException(line, character + exception.characterIndex());
			}

			_offsetWhiteSpaces(string, &idx, &line, &character, false);

			if (string.length() > idx) throw DecoderException(idx);

			return result;

		}

		static bool isStringifiable(const Type& data) {
			switch (data.kind()) {
				case Type::Kind::dictionary: {
					const Dictionary<Type, Type>& dictionary = data.as<Dictionary<Type, Type>>();
					return dictionary.keys().are(Type::Kind::string) && isStringifiable(dictionary.values());
				}
				case Type::Kind::array: {
					return data.as<Array<Type>>().every([](const Type& data) {
						return isStringifiable(data);
					});
				}
				case Type::Kind::string:
				case Type::Kind::number:
				case Type::Kind::null:
				case Type::Kind::date:
					return true;
				default:
					return false;
			}
		}

		static String stringify(const Type& data) {

			String result;

			switch (data.kind()) {
				case Type::Kind::dictionary: {
					result.append("{");
					auto dictionary = data.as<Dictionary<Type, Type>>();
					result.append(String::join(dictionary.keys().map<String>([dictionary](const Type& key) {
						if (key.kind() != Type::Kind::string) throw EncoderTypeException();
						String result;
						result.append(stringify(key));
						result.append(":");
						result.append(stringify(dictionary.get(key)));
						return result;
					}), ","));
					result.append("}");
					break;
				}
				case Type::Kind::array: {
					Array<Type>& array = data.as<Array<Type>>();
					result.append("[");
					result.append(String::join(array.map<String>([](const Type& item) {
						return stringify(item);
					}), ","));
					result.append("]");
					break;
				}
				case Type::Kind::string: {
					auto bytes = data.as<String>().UTF16Data(Endian::systemVariant());
					result.append("\"");
					for (size_t idx = 0 ; idx < bytes.count() ; idx++) {
						auto byte = bytes[idx];
						switch (byte) {
							case '\b':
								result.append("\\b");
								break;
							case '\f':
								result.append("\\f");
								break;
							case '\n':
								result.append("\\n");
								break;
							case '\r':
								result.append("\\r");
								break;
							case '\t':
								result.append("\\t");
								break;
							case '\"':
								result.append("\\\"");
								break;
							case '\\':
								result.append("\\\\");
								break;
							default:
								if (!((byte >= 0x20 && byte <= 0x21) || (byte >= 0x23 && byte <= 0x5B) || byte >= 0x5D)) {
									uint16_t beByte = Endian::fromSystemVariant(byte, Endian::Variant::big);
									auto beByteData = Data<uint16_t>(&beByte, 1).as<uint8_t>();
									result.append("\\U");
									result.append(String::fromHex(beByteData));
								} else {
									result.append(byte);
								}
								break;
						}
					}
					result.append("\"");
					break;
				}
				case Type::Kind::number: {
					switch (data.as<Number<uint64_t>>().subType()) {
						case Subtype::boolean:
							result.append(data.as<types::Boolean>().value() ? "true" : "false");
							break;
						case Subtype::integer:
							result.append(String::format("%lld", data.as<Integer>().value()));
							break;
						case Subtype::floatingPoint: {
							double value = data.as<Float>().value();
							std::ostringstream stream;
							stream << value;
							result.append(stream.str().c_str());
							break;
						}
					}
					break;
				}
				case Type::Kind::null:
					result.append("null");
					break;
				case Type::Kind::date:
					return stringify(data.as<Date>().to(Date::TimeZone::utc).toISO8601());
					break;
				default:
					throw EncoderTypeException();
					break;
			}

			return result;

		}

	};

}

#endif /* json_hpp */
