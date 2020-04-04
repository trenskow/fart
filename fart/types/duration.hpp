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
        long double _seconds;
        mutable Mutex _mutex;
        
    public:
        
        Duration(): _seconds(0) {}
        Duration(long double duration) : _seconds(duration) {}
        Duration(const Duration& other) {
            _seconds = other._seconds;
        }
        
        virtual ~Duration() {};
        
        template<class T = Strong<Duration>>
        static T fromNanoseconds(long double nanoseconds) {
            return Duration::fromMilliseconds<T>(nanoseconds / 1000.0);
        }
        
        template<class T = Strong<Duration>>
        static T fromMilliseconds(long double milliseconds) {
            return Duration::fromSeconds<T>(milliseconds / 1000.0);
        }
        
        template<class T = Strong<Duration>>
        static T fromSeconds(long double seconds) {
            return T(seconds);
        }
        
        template<class T = Strong<Duration>>
        static T fromMinutes(long double minutes) {
            return Duration::fromSeconds<T>(minutes * 60.0);
        }
        
        template<class T = Strong<Duration>>
        static T fromHours(long double hours) {
            return Duration::fromMinutes<T>(hours * 60.0);
        }
        
        template<class T = Strong<Duration>>
        static T fromDays(long double days) {
            return Duration::fromHours<T>(days * 24.0);
        }
        
        inline long double getNanoseconds() const {
            return this->getMilliseconds() * 1000.0;
        }
        
        inline long double getMilliseconds() const {
            return this->getSeconds() * 1000.0;
        }
        
        inline long double getSeconds() const {
            return this->_mutex.lockedValue([this](){
                return this->_seconds;
            });
        }
        
        inline long double getMinutes() const {
            return this->getSeconds() / 60.0;
        }
        
        inline long double getHours() const {
            return this->getMinutes() / 60.0;
        }
        
        inline long double getDays() const {
            return this->getHours() / 24.0;
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
        
        const Kind getKind() const override {
            return Kind::duration;
        }
        
    };
    
}

#endif /* duration_hpp */
