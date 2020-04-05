//
//  date.hpp
//  fart
//
//  Created by Kristian Trenskow on 03/04/2020.
//  Copyright © 2020 Kristian Trenskow. All rights reserved.
//

#ifndef date_hpp
#define date_hpp

#include <cmath>
#include <ctime>

#include "../exceptions/exception.hpp"
#include "../threading/mutex.hpp"
#include "type.hpp"
#include "duration.hpp"
#include "string.hpp"
#include "timezones/timezone.hpp"

#ifndef MIN
#define MIN(a,b) (a < b ? a : b)
#endif

#ifndef MAX
#define MAX(a,b) (a > b ? a : b)
#endif

using namespace fart::exceptions::types;
using namespace fart::types::timezones;

namespace fart::types {
    
    class Date : public Type {
        
    public:
                
        enum Month: uint8_t {
            january = 1,
            february,
            march,
            april,
            may,
            june,
            july,
            august,
            september,
            october,
            november,
            december
        };
        
        enum Weekday: uint8_t {
            sunday = 0,
            monday,
            tuesday,
            wednesday,
            thursday,
            friday,
            saturday
        };
        
    private:
        
        static const Weekday _epochWeekday = thursday;
        
        Duration _time;
        TimeZone _timeZone;
        
        mutable Mutex _mutex;
        
        static const uint16_t daysInMonth(Month month, bool isLeapYear = false) {
            switch (month) {
                case january: return 31;
                case february: return isLeapYear ? 29 : 28;
                case march: return 31;
                case april: return 30;
                case may: return 31;
                case june: return 30;
                case july: return 31;
                case august: return 31;
                case september: return 30;
                case october: return 31;
                case november: return 30;
                case december: return 31;
            }
        }
        
        static const int64_t _daysFromEpoch(int64_t y, unsigned m, unsigned d) noexcept {
            y -= m <= 2;
            const int64_t era = (y >= 0 ? y : y-399) / 400;
            const unsigned yoe = static_cast<unsigned>(y - era * 400);      // [0, 399]
            const unsigned doy = (153*(m + (m > 2 ? -3 : 9)) + 2)/5 + d-1;  // [0, 365]
            const unsigned doe = yoe * 365 + yoe/4 - yoe/100 + doy;         // [0, 146096]
            return era * 146097 + static_cast<int64_t>(doe) - 719468;
        }
        
        static void _components(int64_t time, int64_t *year, uint8_t *month, uint8_t *day) noexcept {
            time += 719468;
            const int64_t era = (time >= 0 ? time : time - 146096) / 146097;
            const unsigned doe = static_cast<unsigned>(time - era * 146097);          // [0, 146096]
            const unsigned yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;  // [0, 399]
            const int64_t y = static_cast<int64_t>(yoe) + era * 400;
            const unsigned doy = doe - (365*yoe + yoe/4 - yoe/100);                // [0, 365]
            const unsigned mp = (5*doy + 2)/153;                                   // [0, 11]
            const unsigned d = doy - (153*mp+2)/5 + 1;                             // [1, 31]
            const unsigned m = mp + (mp < 10 ? 3 : -9);                            // [1, 12]
            if (year != nullptr) *year = y + (m <= 2);
            if (month != nullptr) *month = m;
            if (day != nullptr) *day = d;
        }
        
        Duration _getTimeZoned() const {
            return this->_mutex.lockedValue([this](){
                return this->_time + this->_timeZone.offset(*this);
            });
        }
        
        static int64_t _decodePart(const String& string) {
            int64_t ret = 0;
            for (size_t idx = 0 ; idx < string.getLength() ; idx++) {
                ret *= 10;
                if (string[idx] < '0' || string[idx] > '9') throw ISO8601Exception();
                ret += string[idx] - '0';
            }
            return ret;
        }
        
        void _set(const int64_t year, const uint8_t month, const uint8_t day, const uint8_t hours, const uint8_t minutes, const uint8_t seconds, uint64_t microseconds) {
            _time += Duration::fromDays<Duration>(_daysFromEpoch(year, month, day));
            _time += Duration::fromHours<Duration>(hours);
            _time += Duration::fromMinutes<Duration>(minutes);
            _time += Duration::fromSeconds<Duration>(seconds);
            _time += Duration::fromMicroseconds<Duration>(microseconds);
        }
        
    public:
        
        static const uint8_t daysInWeek = 7;
        
        static const Date& getEpoch() {
            static const Date epoch = Duration::zero();
            return epoch;
        }
        
        static const bool isLeapYear(int64_t year) {
            return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
        }
        
        Date(): _timeZone(TimeZone::utc()) {
            _time = Duration::fromSeconds<Duration>(time(nullptr));
        }
        
        Date(const Duration& time) : Date() {
            _time = time;
        }
        
        Date(const int64_t year, const uint8_t month = 1, const uint8_t day = 1, const uint8_t hours = 0, const uint8_t minutes = 0, const uint8_t seconds = 0, uint64_t microseconds = 0) : Date() {
            this->_set(year, month, day, hours, minutes, seconds, microseconds);
        }
        
        Date(const String& iso8601) : Date() {
            
            int64_t year = 0;
            int8_t month = 0;
            int8_t day = 0;
            int8_t hours = 0;
            int8_t minutes = 0;
            int8_t seconds = 0;
            uint64_t microseconds = 0;
            Duration timeZoneOffset;
            
            auto parts = iso8601.split("T");
            
            if (parts->getCount() > 0) {
                
                auto datePart = parts->getItemAtIndex(0);
                
                year = _decodePart(datePart->substring(0, 4));
                
                if (datePart->getLength() == 8) {
                    month = _decodePart(datePart->substring(4, 2));
                    day = _decodePart(datePart->substring(6, 2));
                }
                else if (datePart->getLength() == 10) {
                    month = _decodePart(datePart->substring(5, 2));
                    day = _decodePart(datePart->substring(8, 2));
                }
                else throw ISO8601Exception();
                
                if (parts->getCount() > 1) {
                    
                    auto seperators = Array<String>();
                    seperators.append(Strong<String>("+"));
                    seperators.append(Strong<String>("-"));
                    seperators.append(Strong<String>("Z"));

                    auto timeParts = parts->getItemAtIndex(1)->split(seperators);
                    
                    if (timeParts->getCount() == 1) throw ISO8601Exception();
                    
                    auto timeComponentsPart = timeParts->getItemAtIndex(0);
                    auto timeZonePart = timeParts->getItemAtIndex(1);
                    
                    auto timeComponentsSubParts = timeComponentsPart->split(".");
                    
                    if (timeComponentsSubParts->getCount() > 0) {
                        
                        auto hmsComponentParts = timeComponentsSubParts->getItemAtIndex(0);
                        
                        hours = _decodePart(hmsComponentParts->substring(0, 2));
                        
                        if (hmsComponentParts->getLength() == 6) {
                            minutes = _decodePart(hmsComponentParts->substring(2, 2));
                            seconds = _decodePart(hmsComponentParts->substring(4, 2));
                        }
                        else if (timeComponentsPart->getLength() == 8) {
                            minutes = _decodePart(hmsComponentParts->substring(3, 2));
                            seconds = _decodePart(hmsComponentParts->substring(6, 2));
                        }
                        
                        if (timeComponentsSubParts->getCount() > 1) {
                            microseconds = _decodePart(timeComponentsSubParts->getItemAtIndex(1));
                        }
                        
                    } else throw ISO8601Exception();
                    
                    if (timeZonePart->getLength() > 0) {
                        bool isNegative = parts->getItemAtIndex(1)->indexOf("-") > -1;
                        if (timeZonePart->getLength() == 2) {
                            timeZoneOffset = Duration::fromHours<Duration>(_decodePart(timeZonePart));
                        }
                        else if (timeZonePart->getLength() >= 4) {
                            size_t minutesOffset = timeZonePart->getLength() == 4 ? 2 : 3;
                            auto hours = _decodePart(timeZonePart->substring(0, 2)) * 60;
                            auto minutes = _decodePart(timeZonePart->substring(minutesOffset, 2));
                            auto total = (hours + minutes) * (isNegative ? -1 : 1);
                            timeZoneOffset = Duration::fromMinutes<Duration>(total);
                        }
                    }
                    
                }
                
            }
            else ISO8601Exception();
            
            this->_set(year, month, day, hours, minutes, seconds, microseconds);
            
            _time += timeZoneOffset;
            
        }
        
        virtual ~Date() {}
        
        const int64_t getYear() const {
            int64_t year;
            _components(this->_getTimeZoned().getDays(), &year, nullptr, nullptr);
            return year;
        }
        
        const Month getMonth() const {
            uint8_t month;
            _components(this->_getTimeZoned().getDays(), nullptr, &month, nullptr);
            return Month(month);
        }
        
        const int16_t getDay() const {
            uint8_t day;
            _components(this->_getTimeZoned().getDays(), nullptr, nullptr, &day);
            return day;
        }
        
        const Weekday getWeekday() const {
            return Weekday(((int64_t)this->_getTimeZoned().getDays() + _epochWeekday) % daysInWeek);
        }
        
        Duration sinceMidnight() const {
            return this->_mutex.lockedValue([this](){
                auto seconds = this->_time.getSeconds();
                double daysSeconds = floor(this->_getTimeZoned().getDays()) * Duration::day();
                return Duration(seconds - daysSeconds);
            });
        }
        
        const bool isLeapYear() {
            return Date::isLeapYear(this->getYear());
        }
        
        const uint8_t getHours() const {
            return this->sinceMidnight().getSeconds() / Duration::hour();
        }
        
        const uint8_t getMinutes() const {
            return (this->sinceMidnight().getSeconds() - (this->getHours() * Duration::hour())) / Duration::minute();
        }
        
        const uint8_t getSeconds() const {
            return (this->sinceMidnight().getSeconds()) - (this->getHours() * Duration::hour()) - (this->getMinutes() * Duration::minute());
        }
        
        const uint32_t getMicroseconds() const {
            return this->_mutex.lockedValue([this](){
                return (this->_time.getSeconds() - floor(this->_time.getSeconds())) * 1000000;
            });
        }
                
        Duration getTime() const {
            return this->_mutex.lockedValue([this](){
                return this->_time;
            });
        }
        
        const TimeZone getTimeZone() const {
            return this->_mutex.lockedValue([this](){
                return this->_timeZone;
            });
        }
        
        void setTimeZone(const TimeZone timeZone) {
            return this->_mutex.locked([this,&timeZone](){
                this->_timeZone = timeZone;
            });
        }
        
        template<class T = Strong<Duration>>
        T since(const Date& other) const {
            return this->getTime() - other.getTime();
        }
        
        Strong<String> toISO8601() const {
            return this->_mutex.lockedValue([this](){
                Strong<String> ret;
                ret->append(String::format("%02lld-%02d-%02dT%02d:%02d:%02d",
                            this->getYear(),
                            this->getMonth(),
                            this->getDay(),
                            this->getHours(),
                            this->getMinutes(),
                            this->getSeconds()));
                auto microseconds = this->getMicroseconds();
                if (microseconds != 0) {
                    ret->append(String::format(".%llu", microseconds));
                }
                ret->append(this->_timeZone.iso8601Suffix());
                return ret;
            });
        }
        
        const Kind getKind() const override {
            return Kind::date;
        }
        
        virtual const uint64_t getHash() const override {
            return this->_mutex.lockedValue([this](){
                return this->_time.getHash();
            });
        }
        
    };
    
}

#endif /* date_hpp */
