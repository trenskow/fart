//
//  response.cpp
//  fart
//
//  Created by Kristian Trenskow on 12/12/2019.
//  Copyright © 2019 Kristian Trenskow. All rights reserved.
//

#include "./response-head.hpp"

using namespace fart::network::web::http;
using namespace fart::exceptions::network::web;

ResponseHead::ResponseHead() : _version(HTTP1_1), _status(OK) {}

ResponseHead::ResponseHead(Data<uint8_t>& data) : Head(data) {
    _version = parseVersion((*_parts)[0]);
    try { _status = (Status)(*_parts)[1]->toInteger(); }
    catch (DecoderException) { throw DataMalformedException(); }
}

ResponseHead::ResponseHead(const ResponseHead& other) : _version(other._version), _status(other._status) {}

ResponseHead::~ResponseHead() {}

const Version ResponseHead::version() const {
    return _version;
}

const ResponseHead::Status ResponseHead::status() const {
    return _status;
}

Strong<Data<uint8_t>> ResponseHead::headData(const Data<uint8_t>& lineBreak) const {
    
    Strong<Data<uint8_t>> result = MessageHead::headData(lineBreak);
    
    result->append(Head::versionData(_version));
    result->append(' ');
    result->append(String::format("%d", _status)->UTF8Data());
    result->append(' ');
    
    switch (_status) {
        case OK:
            result->append(String("OK").UTF8Data());
            break;
    }
    
    result->append(lineBreak);
    
    return result;
    
}
