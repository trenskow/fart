//
//  request.cpp
//  fart
//
//  Created by Kristian Trenskow on 12/12/2019.
//  Copyright © 2019 Kristian Trenskow. All rights reserved.
//

#include "./request.hpp"
#include "../message.hpp"
#include "../url.hpp"
#include "../../../exceptions/exception.hpp"
#include "../../../types/string.hpp"
#include "../../../memory/strong.hpp"

using namespace fart::network::web::http;
using namespace fart::network::web;
using namespace fart::exceptions::network::web;

RequestHead::RequestHead(Data<uint8_t>& data) {
    
    Data<uint8_t> lineBreak = Message<RequestHead>::dataForLineBreakMode(Message<RequestHead>::determineLineBreakMode(data));
    Data<uint8_t> head = data.remove(0, data.indexOf(lineBreak) + lineBreak.getCount());
    
    head.remove(head.getCount() - lineBreak.getCount(), lineBreak.getCount());
    
    Array<Data<uint8_t>> parts = head.split((uint8_t*)" ", 1);
    
    if (parts.getCount() != 3) throw DataMalformedException();
    
    Array<String> stringParts = parts.map<String>([](Data<uint8_t>& current) {
        return String(current);
    });
    
    if (*stringParts[0] == "HEAD") _method = HEAD;
    else if (*stringParts[0] == "GET") _method = GET;
    else if (*stringParts[0] == "POST") _method = POST;
    else if (*stringParts[0] == "PUT") _method = PUT;
    else if (*stringParts[0] == "DELETE") _method = DELETE;
    else throw MethodNotSupportedException();
    
    if (*stringParts[2] == "HTTP/1.0") _version = HTTP1_0;
    else if (*stringParts[2] == "HTTP/1.1") _version = HTTP1_1;
    else throw VersionNotSupportedException();
    
    _path = *Url::decode(stringParts[1]);
    
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
