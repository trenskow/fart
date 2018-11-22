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

namespace fart {
    
    namespace exceptions {
        
        class Exception {
        public:
            virtual const char* getDescription() const = 0;
        };
        
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
            
        }
        
    }
    
}

#endif /* exception_hpp */
