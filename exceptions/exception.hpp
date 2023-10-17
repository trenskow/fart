//
// exception.hpp
// fart
//
// Created by Kristian Trenskow on 2018/08/17.
// See license in LICENSE.
//

#ifndef exception_hpp
#define exception_hpp

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <sys/types.h>

namespace fart::exceptions {

	class Exception {
	public:
		virtual ~Exception() = default;
		virtual const char* description() const = 0;
		virtual Exception* clone() const = 0;
	};

	class NotImplementedException: public Exception {

	public:
		NotImplementedException() {
#ifdef DEBUG
			raise(SIGTRAP);
#endif
		}

		virtual ~NotImplementedException() = default;

		virtual const char* description() const override {
			return "Not implemented.";
		}

		virtual NotImplementedException* clone() const override {
			return new NotImplementedException();
		}

	};

	namespace io {

		namespace sockets {

			class AddressAlreadyInUseException : public Exception {

				uint16_t _port;

			public:

				AddressAlreadyInUseException(uint16_t port) : _port(port) {};
				AddressAlreadyInUseException(const AddressAlreadyInUseException& other) : _port(other._port) {}

				virtual ~AddressAlreadyInUseException() = default;

				virtual const char* description() const override {
					return "Port already in use.";
				}

				virtual AddressAlreadyInUseException* clone() const override {
					return new AddressAlreadyInUseException(this->_port);
				}

				uint16_t port() const {
					return _port;
				}

			};

		}

		namespace fs {

			class CannotOpenFileException: public Exception {

			public:

				virtual ~CannotOpenFileException() = default;

				virtual const char* description() const override {
					return "Cannot open file.";
				}

				virtual CannotOpenFileException* clone() const override {
					return new CannotOpenFileException();
				}

			};

			class PositionIsOutsideFileRange: public Exception {

			private:

				ssize_t _position;

			public:

				PositionIsOutsideFileRange(ssize_t position) {
					_position = position;
				}

				virtual ~PositionIsOutsideFileRange() = default;

				ssize_t position() const {
					return _position;
				}

				virtual const char* description() const override {
					return "Position is outside file range.";
				}

				virtual PositionIsOutsideFileRange* clone() const override {
					return new PositionIsOutsideFileRange(this->_position);
				}

			};

			class FileModeException: public Exception {

			public:

				virtual ~FileModeException() = default;

				virtual const char* description() const override {
					return "Operation not allowed in file's mode.";
				}

				virtual FileModeException* clone() const override {
					return new FileModeException();
				}

			};

			class FileNotFoundException: public Exception {

			public:

				virtual ~FileNotFoundException() = default;

				virtual const char* description() const override {
					return "File does not exists.";
				}

				virtual FileNotFoundException* clone() const override {
					return new FileNotFoundException();
				}

			};

		}

	}

	namespace serialization {

		class JSONMalformedException : public Exception {

		private:
			size_t _line;
			size_t _character;

		public:

			JSONMalformedException(size_t line, size_t character) {
				this->_line = line;
				this->_character = character;
			}

			virtual ~JSONMalformedException() = default;

			virtual const char* description() const override {
				return "JSON is malformed.";
			}

			virtual JSONMalformedException* clone() const override {
				return new JSONMalformedException(this->_line, this->_character);
			}

			size_t line() const {
				return this->_line;
			}

			size_t character() const {
				return this->_character;
			}

		};

	}

	namespace memory {

		class AllocationException : public Exception {

		private:
			size_t _size;

		public:
			AllocationException(const size_t size) : _size(size) {}
			AllocationException(const AllocationException& other) : _size(other._size) {}

			virtual ~AllocationException() = default;

			virtual const char* description() const override {
				return "Cannot allocate memory.";
			}

			virtual AllocationException* clone() const override {
				return new AllocationException(this->_size);
			}

			size_t size() const {
				return _size;
			}

		};

	}

	namespace types {

		class TypeConversionException : public Exception {

		public:

			virtual ~TypeConversionException() = default;

			virtual const char* description() const override {
				return "Could not convert to type.";
			}

			virtual TypeConversionException* clone() const override {
				return new TypeConversionException();
			}

		};

		class ISO8601Exception : public Exception {

		public:

			virtual ~ISO8601Exception() = default;

			virtual const char* description() const override {
				return "Not a valid ISO 8601 string";
			}

			virtual ISO8601Exception* clone() const override {
				return new ISO8601Exception();
			}

		};

		class DecoderException : public Exception {

		private:
			size_t _characterIndex;

		public:

			DecoderException(const size_t characterIndex) : _characterIndex(characterIndex) {}
			DecoderException(const DecoderException& other) : _characterIndex(other._characterIndex) {}

			virtual ~DecoderException() = default;

			virtual const char* description() const override {
				return "Cannot decode character";
			}

			virtual DecoderException* clone() const override {
				return new DecoderException(this->_characterIndex);
			}

			size_t characterIndex() const {
				return _characterIndex;
			}

		};

		class EncoderException: public Exception {

		private:
			size_t _characterIndex;

		public:
			EncoderException(const size_t characterIndex) : _characterIndex(characterIndex) {}

			virtual ~EncoderException() = default;

			virtual const char* description() const override {
				return "Cannot encode character";
			}

			virtual EncoderException* clone() const override {
				return new EncoderException(this->_characterIndex);
			}

			size_t characterIndex() const {
				return _characterIndex;
			}

		};

		class EncoderTypeException: public Exception {

		public:
			EncoderTypeException() {}

			virtual ~EncoderTypeException() = default;

			virtual const char* description() const override {
				return "Cannot encode type.";
			}

			virtual EncoderTypeException* clone() const override {
				return new EncoderTypeException();
			}

		};

		class OutOfBoundException : public Exception {

		private:
			size_t _index;

		public:
			OutOfBoundException(const size_t index) : _index(index) {}
			OutOfBoundException(const OutOfBoundException& other) : _index(other._index) {}

			virtual ~OutOfBoundException() = default;

			virtual const char* description() const override {
				return "Index is out of bounds.";
			}

			virtual OutOfBoundException* clone() const override {
				return new OutOfBoundException(this->_index);
			}

			size_t index() const {
				return _index;
			}

		};

		class KeyNotFoundException : public Exception {

		public:

			KeyNotFoundException() {}

			virtual ~KeyNotFoundException() = default;

			virtual const char* description() const override {
				return "Key was not found";
			}

			virtual KeyNotFoundException* clone() const override {
				return new KeyNotFoundException();
			}

		};

		class NotFoundException : public Exception {

		public:

			virtual ~NotFoundException() = default;

			virtual const char* description() const override {
				return "Item not found.";
			}

			virtual NotFoundException* clone() const override {
				return new NotFoundException();
			}

		};

		class DurationParserException: public Exception {

		public:

			virtual ~DurationParserException() = default;

			virtual const char* description() const override {
				return "Could not parse duration";
			}

			virtual DurationParserException* clone() const override {
				return new DurationParserException();
			}

		};

		class UUIDMalformedException : public Exception {

		public:

			virtual ~UUIDMalformedException() = default;

			virtual const char* description() const override {
				return "UUID is malformed.";
			}

			virtual UUIDMalformedException* clone() const override {
				return new UUIDMalformedException();
			}

		};

		class URLMalformedException : public Exception {

		public:

			virtual ~URLMalformedException() = default;

			virtual const char* description() const override {
				return "URL is malformed.";
			}

			virtual URLMalformedException* clone() const override {
				return new URLMalformedException();
			}

		};

	}

	namespace web {

		class UrlDecodingException : public Exception {

			uint8_t _character;

		public:

			UrlDecodingException(uint8_t chr) : _character(chr) {};
			UrlDecodingException(const UrlDecodingException& other) : _character(other._character) {}

			virtual ~UrlDecodingException() = default;

			virtual const char* description() const override {
				return "Cannot decode character.";
			}

			virtual UrlDecodingException* clone() const override {
				return new UrlDecodingException(this->_character);
			}

			uint8_t character() const {
				return _character;
			}

		};

		class DataIncompleteException : public Exception {

			public:

				virtual ~DataIncompleteException() = default;

				virtual const char* description() const override {
					return "Data is incomplete.";
				}

				virtual DataIncompleteException* clone() const override {
					return new DataIncompleteException();
				}

		};

		class DataMalformedException : public Exception {

			public:

				virtual ~DataMalformedException() = default;

				virtual const char* description() const override {
					return "Data is malformed.";
				}

				virtual DataMalformedException* clone() const override {
					return new DataMalformedException();
				}

		};

		class MethodNotSupportedException : public Exception {

			public:

				virtual ~MethodNotSupportedException() = default;

				virtual const char* description() const override {
					return "Method is not supported.";
				}

				virtual MethodNotSupportedException* clone() const override {
					return new MethodNotSupportedException();
				}

		};

		class VersionNotSupportedException : public Exception {

			public:

				virtual ~VersionNotSupportedException() = default;

				virtual const char* description() const override {
					return "Version is not supported.";
				}

				virtual VersionNotSupportedException* clone() const override {
					return new VersionNotSupportedException();
				}

		};

	}

}

#endif /* exception_hpp */
