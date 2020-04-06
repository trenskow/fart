//
//  request.cpp
//  fart
//
//  Created by Kristian Trenskow on 12/12/2019.
//  Copyright © 2019 Kristian Trenskow. All rights reserved.
//

#include "./request-head.hpp"
#include "../message.hpp"
#include "../url.hpp"
#include "../../../exceptions/exception.hpp"
#include "../../../types/string.hpp"
#include "../../../memory/strong.hpp"

using namespace fart::network::web::http;
using namespace fart::network::web;
using namespace fart::exceptions::network::web;

RequestHead::RequestHead() : _version(HTTP1_1), _method(GET), _path("/") {}

RequestHead::RequestHead(Data<uint8_t>& data) : Head(data) {
        
    if (*(*_parts)[0] == "HEAD") _method = HEAD;
    else if (*(*_parts)[0] == "GET") _method = GET;
    else if (*(*_parts)[0] == "POST") _method = POST;
    else if (*(*_parts)[0] == "PUT") _method = PUT;
    else if (*(*_parts)[0] == "DELETE") _method = DELETE;
    else throw MethodNotSupportedException();
    
    _version = parseVersion((*_parts)[2]);
    _path = *Url::decode(*(*_parts)[1]);
    
}

RequestHead::RequestHead(const RequestHead& other) : _version(other._version), _method(other._method), _path(other._path) {}

RequestHead::~RequestHead() {}

void RequestHead::ensureSpaceAt(Data<uint8_t> &data, size_t index) const {
    if (data.count() < index) throw DataIncompleteException();
    if (data.itemAtIndex(index)) throw DataMalformedException();
}

const Version RequestHead::version() const {
    return _version;
}

const RequestHead::Method RequestHead::method() const {
    return _method;
}

const Strong<String> RequestHead::path() const {
    return Strong<String>(_path);
}

Strong<Data<uint8_t>> RequestHead::headData(const Data<uint8_t>& lineBreak) const {
    
    Strong<Data<uint8_t>> result = MessageHead::headData(lineBreak);
    
    switch (method()) {
       case HEAD:
           result->append(String("HEAD").UTF8Data());
           break;
       case GET:
           result->append(String("GET").UTF8Data());
           break;
       case POST:
           result->append(String("POST").UTF8Data());
           break;
       case PUT:
           result->append(String("PUT").UTF8Data());
           break;
       case DELETE:
           result->append(String("DELETE").UTF8Data());
           break;
    }
    
    result->append(' ');
    result->append(_path->UTF8Data());
    result->append(' ');
    result->append(Head::versionData(_version));
    result->append(lineBreak);
    
    return result;
    
}
