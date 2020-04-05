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
    
    class Duration : public Type {
        
    private:
        
        double _seconds;
        mutable Mutex _mutex;
        
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
        Duration(const Duration& other) {
            _seconds = other._seconds;
        }
        
        virtual ~Duration() {};
        
        template<class T = Strong<Duration>>
        static T fromMicroseconds(double microseconds) {
            return Duration::fromMilliseconds<T>(microseconds / 1000);
        }
        
        template<class T = Strong<Duration>>
        static T fromMilliseconds(double milliseconds) {
            return Duration::fromSeconds<T>(milliseconds / 1000);
        }
        
        template<class T = Strong<Duration>>
        static T fromSeconds(double seconds) {
            return T(seconds);
        }
        
        template<class T = Strong<Duration>>
        static T fromMinutes(double minutes) {
            return Duration::fromSeconds<T>(minutes * 60);
        }
        
        template<class T = Strong<Duration>>
        static T fromHours(double hours) {
            return Duration::fromMinutes<T>(hours * 60);
        }
        
        template<class T = Strong<Duration>>
        static T fromDays(double days) {
            return Duration::fromHours<T>(days * 24);
        }
        
        inline  double getMicroseconds() const {
            return this->getMilliseconds() * 1000;
        }
        
        inline double getMilliseconds() const {
            return this->getSeconds() * 1000;
        }
        
        inline double getSeconds() const {
            return this->_mutex.lockedValue([this](){
                return this->_seconds;
            });
        }
        
        inline double getMinutes() const {
            return this->getSeconds() / 60;
        }
        
        inline double getHours() const {
            return this->getMinutes() / 60;
        }
        
        inline double getDays() const {
            return this->getHours() / 24;
        }
        
        operator double() const {
            return this->_mutex.lockedValue([this](){
                return this->_seconds;
            });
        }
                
        Strong<Duration> operator+(const Duration& other) const noexcept {
            return Strong<Duration>(this->getSeconds() + other.getSeconds());
        }
        
        Strong<Duration> operator-(const Duration& other) const noexcept {
            return Strong<Duration>(this->getSeconds() - other.getSeconds());
        }
        
        Strong<Duration> operator*(const Duration& other) const noexcept {
            return Strong<Duration>(this->getSeconds() * other.getSeconds());
        }
        
        Duration operator/(const Duration& other) const noexcept {
            return Strong<Duration>(this->getSeconds() / other.getSeconds());
        }
        
        Duration operator%(const Duration& other) const noexcept {
            return Strong<Duration>(fmodl(this->getSeconds(), other.getSeconds()));
        }
        
        void operator+=(const Duration& other) {
            this->_mutex.locked([this,&other](){
                _seconds += other.getSeconds();
            });
        }
        
        void operator-=(const Duration& other) {
            this->_mutex.locked([this,&other](){
                _seconds -= other.getSeconds();
            });
        }
        
        void operator*=(const Duration& other) {
            this->_mutex.locked([this,&other](){
                _seconds *= other.getSeconds();
            });
        }
        
        void operator/=(const Duration& other) {
            this->_mutex.locked([this,&other](){
                _seconds /= other.getSeconds();
            });
        }
        
        void operator%=(const Duration& other) {
            this->_mutex.locked([this,&other](){
                _seconds = fmodl(_seconds, other.getSeconds());
            });
        }
        
        virtual const uint64_t getHash() const override {
            return this->_mutex.lockedValue([this](){
                return *((uint64_t*)&_seconds);
            });
        }
        
        const Kind getKind() const override {
            return Kind::duration;
        }
        
    };
    
}

#endif /* duration_hpp */
