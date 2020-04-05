//
//  duration.hpp
//  fart
//
//  Created by Kristian Trenskow on 04/04/2020.
//  Copyright © 2020 Kristian Trenskow. All rights reserved.
//

#ifndef duration_hpp
#define duration_hpp

#include <cmath>

#include "../threading/mutex.hpp"
#include "../memory/strong.hpp"
#include "./type.hpp"

namespace fart::types {
    
    class Duration {
        
    private:
        
        double _seconds;
        
    public:
        
        static const Duration& zero() {
            static const Duration zero = 0;
            return zero;
        }
        
        static const Duration& microsecond() {
            static const Duration microsecond = millisecond().getSeconds() / 1000.0;
            return microsecond;
        }
        
        static const Duration& millisecond() {
            static const Duration millisecond = 1.0 / 1000.0;
            return millisecond;
        }
        
        static const Duration& second() {
            static const Duration second = 1;
            return second;
        }
        
        static const Duration& minute() {
            static const Duration minute = 60 * second().getSeconds();
            return minute;
        }
        
        static const Duration& hour() {
            static const Duration hour = 60 * minute().getSeconds();
            return hour;
        }
        
        static const Duration& day() {
            static const Duration day = 24 * hour().getSeconds();
            return day;
        }
        
        static const Duration& year(bool isLeapYear = false) {
            static const Duration year = 365 * day().getSeconds();
            static const Duration leapYear = 366 * day().getSeconds();
            return isLeapYear ? leapYear : year;
        }
        
        Duration(): _seconds(0) {}
        Duration(double duration) : _seconds(duration) {}
        Duration(const Duration& other) : _seconds(other._seconds) {}
        
        ~Duration() {};
        
        static Duration fromMicroseconds(double microseconds) {
            return Duration::fromMilliseconds(microseconds / 1000);
        }
        
        static Duration fromMilliseconds(double milliseconds) {
            return Duration::fromSeconds(milliseconds / 1000);
        }
        
        static Duration fromSeconds(double seconds) {
            return Duration(seconds);
        }
        
        static Duration fromMinutes(double minutes) {
            return Duration::fromSeconds(minutes * 60);
        }
        
        static Duration fromHours(double hours) {
            return Duration::fromMinutes(hours * 60);
        }
        
        static Duration fromDays(double days) {
            return Duration::fromHours(days * 24);
        }
        
        inline  double getMicroseconds() const {
            return this->getMilliseconds() * 1000;
        }
        
        inline double getMilliseconds() const {
            return this->getSeconds() * 1000;
        }
        
        inline double getSeconds() const {
            return this->_seconds;
        }
        
        inline double getMinutes() const {
            return this->getSeconds() / minute().getSeconds();
        }
        
        inline double getHours() const {
            return this->getSeconds() / hour().getSeconds();
        }
        
        inline double getDays() const {
            return this->getSeconds() / day().getSeconds();
        }
        
        operator double() const {
            return this->getSeconds();
        }
                
        Duration operator+(const Duration& other) const noexcept {
            return this->getSeconds() + other.getSeconds();
        }
        
        Duration operator-(const Duration& other) const noexcept {
            return this->getSeconds() - other.getSeconds();
        }
        
        Duration operator*(const Duration& other) const noexcept {
            return this->getSeconds() * other.getSeconds();
        }
        
        Duration operator/(const Duration& other) const noexcept {
            return this->getSeconds() / other.getSeconds();
        }
        
        Duration operator%(const Duration& other) const noexcept {
            return fmodl(this->getSeconds(), other.getSeconds());
        }
        
        void operator+=(const Duration& other) {
            _seconds += other;
        }
        
        void operator-=(const Duration& other) {
            _seconds -= other;
        }
        
        void operator*=(const Duration& other) {
            _seconds *= other;
        }
        
        void operator/=(const Duration& other) {
            _seconds /= other;
        }
        
        void operator%=(const Duration& other) {
            _seconds = fmodl(_seconds, other);
        }
                
    };
    
}

#endif /* duration_hpp */
