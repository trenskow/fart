//
//  file.hpp
//  fart
//
//  Created by Kristian Trenskow on 12/04/2020.
//  Copyright © 2020 Kristian Trenskow. All rights reserved.
//

#ifndef file_hpp
#define file_hpp

#include <stdio.h>
#include <wordexp.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#include "../../memory/object.hpp"
#include "../../exceptions/exception.hpp"
#include "../../threading/mutex.hpp"
#include "../../tools/math.hpp"

using namespace fart::memory;
using namespace fart::types;
using namespace fart::exceptions::io::fs;
using namespace fart::tools;

namespace fart::io::fs {

	class File : public Object {

		friend class Strong<File>;

	public:

		enum class Mode {
			asRead,
			asWrite,
			asAppend
		};

		enum class Position {
			beginning,
			current,
			end
		};

		static Strong<File> open(const String& filename, Mode mode) {
			return Strong<File>(filename, mode);
		}

		static bool exists(const String& filename) {
			return filename.mapCString<bool>([](const char* filename){
				return access(filename, F_OK) == 0;
			});
		}

		static String expand(const String& filename) {
			return filename.mapCString<String>([](const char* filename) {
				wordexp_t exp_result;
				wordexp(filename, &exp_result, 0);
				size_t filenameLength = strlen(exp_result.we_wordv[0]);
				auto ret = String::fromCString([&exp_result,&filenameLength](char* buffer, size_t length) {
					memcpy(buffer, exp_result.we_wordv[0], filenameLength);
					return filenameLength;
				}, filenameLength);
				wordfree(&exp_result);
				return ret;
			});
		}

		static String resolve(const String& filename) {
			return filename.mapCString<String>([](const char* filename) {
				return String::fromCString([filename](char* buffer, size_t length) {
					ssize_t result = readlink(filename, buffer, length);
					if (result < 0) strncpy(buffer, filename, length);
					return strlen(buffer);
				}, 4096);
			});
		}

		static Array<String> directoryContent(const String& filename) {

			if (!File::exists(filename)) throw FileNotFoundException();

			DIR* dp;
			dirent* ep;

			dp = filename.mapCString<dirent*>([](const char* filename) {
				return opendir(filename);
			});

			if (dp == nullptr) throw FileNotFoundException();

			Array<String> result;

			while ((ep = readdir(dp))) {
				result.append(Strong<String>(ep->d_name));
			}

			closedir(dp);

			return result;

		}

		virtual ~File() {
			this->_mutex.locked([this](){
				fclose(this->_stream);
			});
		}

		template<typename T = uint8_t>
		Data<T> read(size_t count) {
			return this->_mutex.lockedValue([this,count](){
				if (this->_mode == Mode::asWrite) throw FileModeException();
				size_t toRead = math::min(sizeof(T) * count, this->_size - this->_position);
				auto read = Data<T>::fromCBuffer([this,toRead](void* buffer, size_t length){
					fread(buffer, sizeof(T), length / sizeof(T), this->_stream);
					return length;
				}, toRead);
				this->_position += toRead;
				return read;
			});
		}

		template<typename T>
		const T read() {
			return this->_mutex.lockedValue([this](){
				this->_position += sizeof(T);
				if (this->_position > this->_size) throw PositionIsOutsideFileRange(this->_position);
				T result;
				fread(&result, sizeof(T), 1, this->_stream);
				return result;
			});
		}

		template<typename T = uint8_t>
		Data<T> readToEnd() {
			Data<T> buffer;
			this->_mutex.locked([this,&buffer](){
				while (this->_position < this->_size) {
					buffer.append(this->read(16384));
				}
			});
			return buffer;
		}

		template<typename T = uint8_t>
		void write(const Data<T>& data) {
			this->_mutex.locked([this,&data](){
				if (this->_mode == Mode::asRead) throw FileModeException();
				this->_position += data.count() * sizeof(T);
				this->_size = math::max(this->_position, this->_size);
				fwrite(data.items(), sizeof(T), data.count(), this->_stream);
			});
		}

	private:

		File(const String& filename, Mode mode) {
			File::resolve(File::expand(filename)).withCString([this,&mode](const char* filename) {

				switch (mode) {
					case Mode::asRead:
						this->_stream = fopen(filename, "r");
						break;
					case Mode::asWrite:
						this->_stream = fopen(filename, "w");
						break;
					case Mode::asAppend:
						this->_stream = fopen(filename, "a");
						break;
				}

				if (!this->_stream) throw CannotOpenFileException();
				this->_mode = mode;
				this->_determineSize();

			});
		}

		void _determineSize() {
			this->_position = ftell(this->_stream);
			fseek(this->_stream, 0, SEEK_END);
			this->_size = ftell(this->_stream);
			fseek(this->_stream, this->_position, SEEK_SET);
		}

		Mode _mode;
		FILE* _stream;
		size_t _size;
		size_t _position;

		Mutex _mutex;

	public:

	};

}

#endif /* file_hpp */
