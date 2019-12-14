//
//  message.hpp
//  fart
//
//  Created by Kristian Trenskow on 11/12/2019.
//  Copyright © 2019 Kristian Trenskow. All rights reserved.
//

#ifndef message_hpp
#define message_hpp

#include "../../memory/object.hpp"
#include "../../memory/strong.hpp"
#include "../../types/string.hpp"
#include "../../types/data.hpp"
#include "../../types/dictionary.hpp"
#include "../../exceptions/exception.hpp"

using namespace fart::types;
using namespace fart::memory;
using namespace fart::exceptions::network::web;

namespace fart::network::web {
    
    template<class Head>
    class Message : public Head {
        
        static_assert(std::is_base_of<Object, Head>::value, "T must be a subclass of class Object.");
        
    public:
        
        enum LineBreakMode {
            crLf = 0,
            cr,
            lf
        };
        
        Message() : _lineBreakMode(crLf) {};
        
        Message(Data<uint8_t>& data) noexcept(false) : Head(data) {
            
            Data<uint8_t> lineBreak = dataForLineBreakMode(determineLineBreakMode(data));
            
            Data<uint8_t> deliminator(lineBreak);
            deliminator.append(lineBreak);
            
            Strong<Array<Data<uint8_t>>> parts = data.split(deliminator, 2);
            
            if (parts->getCount() < 1) {
                throw DataIncompleteException();
            }
            
            Strong<Array<Data<uint8_t>>> header = parts->getItemAtIndex(0)->split(lineBreak);
            
            if (header->getCount() < 2) throw DataIncompleteException();
                        
            header->forEach([this](Data<uint8_t>& current) {
                Data<uint8_t> keyValueSplitter((uint8_t *)": ", 2);
                Strong<Array<Data<uint8_t>>> parts = current.split(keyValueSplitter);
                if (parts->getCount() != 2) throw DataIncompleteException();
                Strong<Array<String>> strings = parts->map<String>([](Data<uint8_t>& current) {
                    return Strong<String>(current);
                });
                _headers.set(strings->getItemAtIndex(0), strings->getItemAtIndex(1));
            });
            
        }
        
        Message(const Message<Head>& other) : Head(other), _lineBreakMode(other._lineBreakMode), _headers(other._headers) {}
        
        virtual ~Message() {}
        
        Strong<Data<uint8_t>> getData() const {
            return nullptr;
        }
        
        Strong<String> getHeaderValue(const char* key) const {
            return _headers[String(key)];
        }
        
        Strong<String> getHeaderValue(const String& key) const {
            return _headers[key];
        }
        
        void setHeaderValue(const String& key, String& value) {
            _headers.set(key, value);
        }
        
        Strong<Data<uint8_t>> getBody() const {
            return nullptr;
        }
        
        static LineBreakMode determineLineBreakMode(Data<uint8_t>& data) {
            
            if (data.indexOf(dataForLineBreakMode(crLf))) {
                return crLf;
            }
            else if (data.indexOf(dataForLineBreakMode(cr))) {
                return cr;
            }
            else if (data.indexOf(dataForLineBreakMode(lf))) {
                return lf;
            }
            else {
                throw DataIncompleteException();
            }
            
        }
        
        static Strong<Data<uint8_t>> dataForLineBreakMode(LineBreakMode lineBreakMode) {
            switch (lineBreakMode) {
                case crLf:
                    return Strong<Data<uint8_t>>((uint8_t*)"\r\n", 2);
                case cr:
                    return Strong<Data<uint8_t>>((uint8_t*)"\r", 1);
                case lf:
                    return Strong<Data<uint8_t>>((uint8_t*)"\n", 1);
            }
        }

    protected:
        
        const LineBreakMode getLineBreakMode() const {
            return _lineBreakMode;
        }
        
    private:
                
        LineBreakMode _lineBreakMode;
        Dictionary<String, String> _headers;
            
    };
    
}

#endif /* message_hpp */
