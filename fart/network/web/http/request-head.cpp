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
    if (data.getCount() < index) throw DataIncompleteException();
    if (data.getItemAtIndex(index)) throw DataMalformedException();
}

const Version RequestHead::getVersion() const {
    return _version;
}

const RequestHead::Method RequestHead::getMethod() const {
    return _method;
}

const Strong<String> RequestHead::getPath() const {
    return Strong<String>(_path);
}

Strong<Data<uint8_t>> RequestHead::getHeadData(const Data<uint8_t>& lineBreak) const {
    
    Strong<Data<uint8_t>> result = MessageHead::getHeadData(lineBreak);
    
    switch (getMethod()) {
       case HEAD:
           result->append(String("HEAD").getData());
           break;
       case GET:
           result->append(String("GET").getData());
           break;
       case POST:
           result->append(String("POST").getData());
           break;
       case PUT:
           result->append(String("PUT").getData());
           break;
       case DELETE:
           result->append(String("DELETE").getData());
           break;
    }
    
    result->append(' ');
    result->append(_path->getData());
    result->append(' ');
    result->append(Head::versionData(_version));
    result->append(lineBreak);
    
    return result;
    
}
