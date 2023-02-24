//
//  duration.hpp
//  fart
//
//  Created by Kristian Trenskow on 2020/04/04.
//  Copyright © 2018-2023 Kristian Trenskow. All rights reserved.
//

#ifndef duration_hpp
#define duration_hpp

#include <math.h>

#include "../memory/strong.hpp"
#include "./type.hpp"
#include "./comparable.hpp"

namespace fart::types {

	class Duration : public Comparable<Duration> {

	private:

		double _seconds;

	public:

		using Comparable<Duration>::operator==;

		static const Duration& zero() {
			static const Duration zero = 0;
			return zero;
		}

		static const Duration& microsecond() {
			static const Duration microsecond = millisecond().seconds() / 1000.0;
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
			static const Duration minute = 60 * second().seconds();
			return minute;
		}

		static const Duration& hour() {
			static const Duration hour = 60 * minute().seconds();
			return hour;
		}

		static const Duration& day() {
			static const Duration day = 24 * hour().seconds();
			return day;
		}

		static const Duration& year(bool isLeapYear = false) {
			static const Duration year = 365 * day().seconds();
			static const Duration leapYear = 366 * day().seconds();
			return isLeapYear ? leapYear : year;
		}

		static Duration parse(const String& string) {

			if (string == "Z") return Duration::zero();

			String parse = string;

			if (parse.length() == 0) throw DurationParserException();

			double multiplier = 1;

			auto prefix = parse.substring(0, 1);
			if (*prefix == "+" || *prefix == "-") {
				multiplier = (*prefix == "-" ? -1 : 1);
				parse = parse.substring(1);
			}

			uint64_t hours = 0;
			uint64_t minutes = 0;

			if (parse.length() == 5 && *parse.substring(2, 1) == ":") {
				hours = parse.substring(0, 2)->doubleValue();
				minutes = parse.substring(3, 2)->doubleValue();
			}
			else if (parse.length() == 4) {
				hours = parse.substring(0, 2)->doubleValue();
				minutes = parse.substring(2, 4)->doubleValue();
			}
			else if (parse.length() == 2) {
				hours = parse.doubleValue();
			}
			else throw DurationParserException();

			return Duration::fromHours(hours) + Duration::fromMinutes(minutes) * multiplier;

		}

		Duration(): _seconds(0) {}
		Duration(double duration) : _seconds(duration) {}

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

		inline  double microseconds() const {
			return this->milliseconds() * 1000;
		}

		inline double milliseconds() const {
			return this->seconds() * 1000;
		}

		inline double seconds() const {
			return this->_seconds;
		}

		inline double minutes() const {
			return this->seconds() / minute().seconds();
		}

		inline double hours() const {
			return this->seconds() / hour().seconds();
		}

		inline double days() const {
			return this->seconds() / day().seconds();
		}

		enum class ToStringOptions: uint8_t {
			prefixPositive = 1 << 0
		};

		String toString(ToStringOptions options) const {
			double offset = _seconds;
			double absOffset = fabs(offset);
			uint64_t hours = Duration(absOffset).hours();
			uint64_t minutes = Duration(absOffset - Duration::fromHours(hours)).minutes();
			String prefix = offset < 0 ? "-" : (static_cast<uint8_t>(options) & static_cast<uint8_t>(ToStringOptions::prefixPositive) ? "+" : "");
			return prefix.mapCString<String>([&hours,&minutes](const char* prefix){
				return String(String::format("%s%02llu:%02llu", prefix, hours, minutes));
			});
		}

		operator double() const {
			return this->seconds();
		}

		virtual bool operator>(const Duration& other) const override {
			return _seconds > other._seconds;
		}

		Duration operator+(const Duration& other) const {
			return this->seconds() + other.seconds();
		}

		Duration operator-(const Duration& other) const {
			return this->seconds() - other.seconds();
		}

		Duration operator*(const double other) const {
			return this->seconds() * other;
		}

		Duration operator/(const double other) const {
			return this->seconds() / other;
		}

		void operator+=(const Duration& other) {
			_seconds += other;
		}

		void operator-=(const Duration& other) {
			_seconds -= other;
		}

		void operator*=(const double other) {
			_seconds *= other;
		}

		void operator/=(const double other) {
			_seconds /= other;
		}

	};

}

#endif /* duration_hpp */
