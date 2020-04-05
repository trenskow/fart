//
//  timezone.hpp
//  fart
//
//  Created by Kristian Trenskow on 04/04/2020.
//  Copyright © 2020 Kristian Trenskow. All rights reserved.
//

#ifndef timezone_hpp
#define timezone_hpp

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
//define something for Windows (32-bit and 64-bit, this part is common)
#ifdef _WIN64
//define something for Windows (64-bit only)
#else
//define something for Windows (32-bit only)
#endif
#elif __APPLE__
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
#define TZDIR "/usr/share/zoneinfo"
#elif TARGET_OS_IPHONE
#define TZDIR "/var/db/timezone/zoneinfo"
#elif TARGET_OS_MAC
#define TZDIR "/usr/share/zoneinfo"
#else
# error "Unknown Apple platform"
#endif
#elif __linux__
#define TZDIR "/usr/share/zoneinfo"
#elif __unix__ // all unices not caught above
#define TZDIR "/usr/share/zoneinfo"
#elif defined(_POSIX_VERSION)
#define TZDIR "/usr/share/zoneinfo"
#else
#error "Unknown platform"
#endif

#include "../../memory/object.hpp"
#include "../hashable.hpp"
#include "../../threading/mutex.hpp"
#include "../../memory/strong.hpp"
#include "../string.hpp"

using namespace fart::types;
using namespace fart::memory;

namespace fart::types {
    
    class Date;
    class Duration;
    
    namespace timezones {
        
        class TimeZone: public Object, public Hashable {
            
        private:
            
            TimeZone();
            
        public:
            
            virtual ~TimeZone() {}
            
            static const TimeZone& utc();
            
            virtual Duration offset(const Date& at) const;
            
        };
        
    }
    
}

#endif /* timezone_hpp */
