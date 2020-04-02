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
    try { _status = (Status)(*_parts)[1]->parseNumber(); }
    catch (DecoderException) { throw DataMalformedException(); }
}

ResponseHead::ResponseHead(const ResponseHead& other) : _version(other._version), _status(other._status) {}

ResponseHead::~ResponseHead() {}

const Version ResponseHead::getVersion() const {
    return _version;
}

const ResponseHead::Status ResponseHead::getStatus() const {
    return _status;
}

Strong<Data<uint8_t>> ResponseHead::getHeadData(const Data<uint8_t>& lineBreak) const {
    
    Strong<Data<uint8_t>> result = MessageHead::getHeadData(lineBreak);
    
    result->append(Head::versionData(_version));
    result->append(' ');
    result->append(String::format("%d", _status)->getUTF8Data());
    result->append(' ');
    
    switch (_status) {
        case OK:
            result->append(String("OK").getUTF8Data());
            break;
    }
    
    result->append(lineBreak);
    
    return result;
    
}
