//
//  exception.hpp
//  fart
//
//  Created by Kristian Trenskow on 17/08/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef exception_hpp
#define exception_hpp

#include <cstdlib>
#include <cstring>

namespace fart {
    
    namespace exceptions {
        
        class Exception {
        public:
            virtual const char* getDescription() const = 0;
        };
        
        class NotImplementedException: public Exception {
            
        public:
            NotImplementedException() {}
            
            virtual const char* getDescription() const {
                return "Not implemented.";
            }
            
        };
        
        namespace io {
            
            namespace fs {
                
                class CannotResolveFilenameException: public Exception {
                    
                public:
                    
                    const char * getDescription() const override {
                        return "Cannot resolve filename.";
                    }
                    
                };
                
                class CannotOpenFileException: public Exception {
                    
                public:
                    
                    const char * getDescription() const override {
                        return "Cannot open file.";
                    }
                    
                };
                
                class PositionIsOutsideFileRange: public Exception {
                    
                private:
                    
                    ssize_t _position;
                    
                public:
                    
                    PositionIsOutsideFileRange(ssize_t position) {
                        _position = position;
                    }
                    
                    const ssize_t getPosition() const {
                        return _position;
                    }
                    
                    const char * getDescription() const override {
                        return "Position is outside file range.";
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
                
                const size_t getLine() const {
                    return this->_line;
                }
                
                const size_t getCharacter() const {
                    return this->_character;
                }
                
                virtual const char* getDescription() const {
                    return "JSON is malformed.";
                }
                
            };
            
            class JSONUnexpectedEndOfDataException: public Exception {
                
            public:
                virtual const char* getDescription() const {
                    return "Unexpected end of JSON data.";
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
                
                const size_t getSize() const {
                    return _size;
                }
                
                virtual const char* getDescription() const {
                    return "Cannot allocate memory.";
                }
                
            };

        }

        namespace types {
            
            class ISO8601Exception : public Exception {
                
            public:
                virtual const char* getDescription() const {
                    return "Not a valid ISO 8601 string";
                }
                
            };

            class DecoderException : public Exception {
                
            private:
                size_t _characterIndex;
                
            public:
                DecoderException(const size_t characterIndex) : _characterIndex(characterIndex) {}
                DecoderException(const DecoderException& other) : _characterIndex(other._characterIndex) {}
                
                const size_t getCharacterIndex() const {
                    return _characterIndex;
                }
                
                virtual const char* getDescription() const {
                    return "Cannot decode character";
                }
                
            };
            
            class EncoderException: public Exception {
                
            private:
                size_t _characterIndex;
                
            public:
                EncoderException(const size_t characterIndex) : _characterIndex(characterIndex) {}
                
                const size_t getCharacterIndex() const {
                    return _characterIndex;
                }
                
                virtual const char* getDescription() const {
                    return "Cannot encode character";
                }
                
            };
            
            class EncoderTypeException: public Exception {
                
            public:
                EncoderTypeException() {}
                
                virtual const char* getDescription() const {
                    return "Cannot encode type.";
                }
                
            };

            class OutOfBoundException : public Exception {
                
            private:
                size_t _index;
                
            public:
                OutOfBoundException(const size_t index) : _index(index) {}
                OutOfBoundException(const OutOfBoundException& other) : _index(other._index) {}
                
                const size_t getIndex() const {
                    return _index;
                }
                
                virtual const char* getDescription() const {
                    return "Index is out of bounds.";
                }
                
            };

            template<class T>
            class KeyNotFoundException : public Exception {
                
                T _key;
                
            public:
                
                KeyNotFoundException(T key) : _key(key) {};
                KeyNotFoundException(const KeyNotFoundException<T>& other) : _key(other._key) {}
                
                virtual const char* getDescription() const {
                    return "Key was not found";
                }
                
                const T& getKey() const {
                    return _key;
                }
                
            };

            template<class T>
            class NotFoundException : public Exception {
                
                const T& _item;
                
            public:
                
                NotFoundException(const T& item) : _item(item) {}
                NotFoundException(const NotFoundException<T>& other) : _item(other._item) {}
                
                virtual const char* getDescription() const {
                    return "Item not found.";
                }
                
                const T& getItem() const {
                    return _item;
                }
                
            };
            
            class TimeZoneDecoderException: public Exception {
                
            public:
                
                const char * getDescription() const override {
                    return "TimeZone decoder error";
                }
                
            };

        }

        namespace network {

            class AddressAlreadyInUseException : public Exception {
                
                uint16_t _port;
                
            public:
                
                AddressAlreadyInUseException(uint16_t port) : _port(port) {};
                AddressAlreadyInUseException(const AddressAlreadyInUseException& other) : _port(other._port) {}
                
                virtual const char* getDescription() const {
                    return "Port already in use.";
                }
                
                const uint16_t getPort() const {
                    return _port;
                }
                
            };

            class UrlDecodingException : public Exception {
                
                uint8_t _character;
                
            public:
                
                UrlDecodingException(uint8_t chr) : _character(chr) {};
                UrlDecodingException(const UrlDecodingException& other) : _character(other._character) {}
                
                virtual const char* getDescription() const {
                    return "Cannot decode character.";
                }
                
                const uint8_t getCharacter() const {
                    return _character;
                }
                
            };

            namespace web {

                class DataIncompleteException : public Exception {
                    
                    virtual const char* getDescription() const {
                        return "Data is incomplete.";
                    }
                    
                };

                class DataMalformedException : public Exception {
                    
                    virtual const char* getDescription() const {
                        return "Data is malformed.";
                    }
                    
                };

                class MethodNotSupportedException : public Exception {
                    
                    virtual const char* getDescription() const {
                        return "Method is not supported.";
                    }
                    
                };

                class VersionNotSupportedException : public Exception {
                    
                    virtual const char* getDescription() const {
                        return "Version is not supported.";
                    }
                    
                };

            }
            
        }
        
    }
    
}

#endif /* exception_hpp */

