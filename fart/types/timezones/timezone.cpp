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
    static const TimeZone utc = TimeZone();
    return utc;
}

TimeZone::TimeZone() {}

Duration TimeZone::offset(const Date& at) const {
    return 0;
};
