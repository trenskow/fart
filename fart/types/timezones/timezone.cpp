//
//  timezone.cpp
//  fart
//
//  Created by Kristian Trenskow on 04/04/2020.
//  Copyright © 2020 Kristian Trenskow. All rights reserved.
//

#include "timezone.hpp"
#include "../string.hpp"
#include "../../memory/strong.hpp"
#include "../duration.hpp"

/*
 Currently only UTC is supported.
 This file is for future implementation of timezones.
 */

using namespace fart::types::timezones;

const TimeZone& TimeZone::utc() {
    static const TimeZone utc = TimeZone("Z");
    return utc;
}

TimeZone::TimeZone(const String& iso8601Suffix) : _iso8601Suffix(iso8601Suffix) {}

Strong<String> TimeZone::iso8601Suffix() const {
    return _iso8601Suffix;
}

Duration TimeZone::offset(const Date& at) const {
    return Duration(0);
};
