//
//  file.hpp
//  fart
//
//  Created by Kristian Trenskow on 12/04/2020.
//  Copyright © 2020 Kristian Trenskow. All rights reserved.
//

#ifndef file_h
#define file_h

#include <cstdio>
#include <wordexp.h>
#include <cstring>

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
        
    public:
        
        enum Mode {
            asRead,
            asWrite,
            asAppend
        };
        
    private:
            
        Mode _mode;
        FILE* _stream;
        size_t _size;
        size_t _position;
        
        Mutex _mutex;

        void _determineSize() {
            this->_position = ftell(this->_stream);
            fseek(this->_stream, 0, SEEK_END);
            this->_size = ftell(this->_stream);
            fseek(this->_stream, this->_position, SEEK_SET);
        }
        
    public:
        
        enum Position {
            beginning,
            current,
            end
        };
        
        static Strong<String> expand(const String& filename) {
            return filename.mapCString<Strong<String>>([](const char* filename) {
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
        
        static Strong<String> resolve(const String& filename) {
            return filename.mapCString<Strong<String>>([](const char* filename) {
                return String::fromCString([filename](char* buffer, size_t length) {
                    ssize_t result = readlink(filename, buffer, length);
                    if (result < 0) strncpy(buffer, filename, length);
                    return strlen(buffer);
                });
            });
        }
                
        File(const String& filename, Mode mode) {
            File::resolve(File::expand(filename))->withCString([this,&mode](const char* filename) {
                
                switch (mode) {
                    case asRead:
                        this->_stream = fopen(filename, "r");
                        break;
                    case asWrite:
                        this->_stream = fopen(filename, "w");
                        break;
                    case asAppend:
                        this->_stream = fopen(filename, "a");
                        break;
                }
                                
                if (!this->_stream) throw CannotOpenFileException();
                this->_mode = mode;
                this->_determineSize();
                
            });
        }
        
        virtual ~File() {
            this->_mutex.locked([this](){
                fclose(this->_stream);
            });
        }
        
        template<typename T = uint8_t>
        Strong<Data<T>> read(size_t count) {
            return _mutex.lockedValue([this,count](){
                if (this->_mode == asWrite) throw FileModeException();
                size_t toRead = math::max(sizeof(T) * count, this->_size - this->_position);
                this->_position += toRead;
                return Data<T>::fromCBuffer([this,toRead](void* buffer, size_t length){
                    fread(buffer, sizeof(T), length / sizeof(T), this->_stream);
                }, toRead);
            });
        }
        
        template<typename T>
        const T read() {
            return _mutex.lockedValue([this](){
                this->_position += sizeof(T);
                if (this->_position > this->_size) throw PositionIsOutsideFileRange(this->_position);
                T result;
                fread(&result, sizeof(T), 1, this->_stream);
                return result;
            });
        }
        
        template<typename T = uint8_t>
        Strong<Data<T>> readToEnd() {
            return _mutex.lockedValue([this](){
                return read(this->_size / sizeof(T) - this->_position / sizeof(T));
            });
        }
        
        template<typename T = uint8_t>
        void write(const Data<T>& data) {
            _mutex.locked([this,&data](){
                if (this->_mode == asRead) throw FileModeException();
                this->_position += data.count() * sizeof(T);
                this->_size = math::max(this->_position, this->_size);
                fwrite(data.items(), sizeof(T), data.count(), this->_stream);
            });
        }
            
    };
    
}

#endif /* file_h */
