//
//  url.cpp
//  fart
//
//  Created by Kristian Trenskow on 12/12/2019.
//  Copyright © 2019 Kristian Trenskow. All rights reserved.
//

#include "./url.hpp"
#include "../../exceptions/exception.hpp"

using namespace fart::network::web;
using namespace fart::exceptions::network;

const uint8_t Url::hexValue(uint32_t value) {
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'A' && value <= 'F') return 10 + (value - 'A');
    throw UrlDecodingException(value);
}

Strong<String> Url::decode(const String& url) {
    Strong<String> result;
    for (size_t idx = 0 ; idx < url.getLength() ; idx++) {
        uint32_t chr = url[idx];
        if (chr == '+') chr = ' ';
        else if (chr == '%') {
            if (idx >= url.getLength() - 2) throw UrlDecodingException(chr);
            chr = (hexValue(url[idx + 1]) << 8) + hexValue(url[idx + 2]);
            idx += 2;
        }
        result->append(chr);
    }
    return result;
}
