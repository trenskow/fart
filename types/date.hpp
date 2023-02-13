//
//  date.hpp
//  fart
//
//  Created by Kristian Trenskow on 2020/04/03.
//  Copyright © 2018-2023 Kristian Trenskow. All rights reserved.
//

#ifndef date_hpp
#define date_hpp

#include <math.h>
#include <time.h>

#include "../exceptions/exception.hpp"
#include "./type.hpp"
#include "./duration.hpp"
#include "./string.hpp"
#include "./comparable.hpp"

using namespace fart::exceptions::types;

namespace fart::types {

	class Date : public Type, public Comparable<Date> {

	public:

		using Comparable<Date>::operator==;

		enum class TimeZone {
			utc,
			local
		};

		enum class Month: uint8_t {
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

		enum class Weekday: uint8_t {
			sunday = 0,
			monday,
			tuesday,
			wednesday,
			thursday,
			friday,
			saturday
		};

	private:

		static const Weekday _epochWeekday = Weekday::thursday;

		Duration _time;
		TimeZone _timeZone;

		static uint16_t daysInMonth(Month month, bool isLeapYear = false) {
			switch (month) {
				case Month::january: return 31;
				case Month::february: return isLeapYear ? 29 : 28;
				case Month::march: return 31;
				case Month::april: return 30;
				case Month::may: return 31;
				case Month::june: return 30;
				case Month::july: return 31;
				case Month::august: return 31;
				case Month::september: return 30;
				case Month::october: return 31;
				case Month::november: return 30;
				case Month::december: return 31;
			}
		}

		static int64_t _daysFromEpoch(int64_t y, unsigned m, unsigned d) {
			y -= m <= 2;
			const int64_t era = (y >= 0 ? y : y-399) / 400;
			const unsigned yoe = static_cast<unsigned>(y - era * 400);      // [0, 399]
			const unsigned doy = (153*(m + (m > 2 ? -3 : 9)) + 2)/5 + d-1;  // [0, 365]
			const unsigned doe = yoe * 365 + yoe/4 - yoe/100 + doy;         // [0, 146096]
			return era * 146097 + static_cast<int64_t>(doe) - 719468;
		}

		static void _components(int64_t time, int64_t *year, uint8_t *month, uint8_t *day) {
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

		void _set(const int64_t year, const uint8_t month, const uint8_t day, const uint8_t hours, const uint8_t minutes, const uint8_t seconds, uint64_t microseconds) {
			_time = Duration::fromDays(_daysFromEpoch(year, month, day));
			_time += Duration::fromHours(hours);
			_time += Duration::fromMinutes(minutes);
			_time += Duration::fromSeconds(seconds);
			_time += Duration::fromMicroseconds(microseconds);
		}

		static const Duration _localOffset() {
			time_t rawTime;
			::time(&rawTime);
			return localtime(&rawTime)->tm_gmtoff;
		}

	public:

		static const uint8_t daysInWeek = 7;

		static const Date& epoch() {
			static const Date epoch = Duration::zero();
			return epoch;
		}

		static bool isLeapYear(int64_t year) {
			return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
		}

		Date(const Date& other) : Type(other), _time(other._time), _timeZone(other._timeZone) { }

		Date() : _timeZone(TimeZone::utc) {
			_time = Duration::fromSeconds(::time(nullptr));
		}

		Date(const Duration& time, TimeZone timeZone = TimeZone::utc) : _time(time), _timeZone(timeZone) { }

		Date(const time_t& time, TimeZone timeZone = TimeZone::utc) : _time(Duration((double)time)), _timeZone(timeZone) { }

		Date(const int64_t year, const uint8_t month, const uint8_t day, const uint8_t hours = 0, const uint8_t minutes = 0, const uint8_t seconds = 0, uint64_t microseconds = 0) : Date() {
			this->_set(year, month, day, hours, minutes, seconds, microseconds);
		}

		Date(const String& iso8601) : Date() {

			int64_t year = 0;
			uint8_t month = 0;
			uint8_t day = 0;
			uint8_t hours = 0;
			uint8_t minutes = 0;
			uint8_t seconds = 0;
			uint64_t microseconds = 0;
			Duration timeZoneOffset;

			auto parts = iso8601.split("T");

			if (parts->count() > 0) {

				auto datePart = parts->itemAtIndex(0);

				year = datePart.substring(0, 4)->doubleValue();

				if (datePart.length() == 8) {
					month = datePart.substring(4, 2)->doubleValue();
					day = datePart.substring(6, 2)->doubleValue();
				}
				else if (datePart.length() == 10) {
					month = datePart.substring(5, 2)->doubleValue();
					day = datePart.substring(8, 2)->doubleValue();
				}
				else throw ISO8601Exception();

				if (parts->count() > 1) {

					auto separators = String();
					separators.append("+");
					separators.append("-");
					separators.append("Z");

					auto timeParts = parts->itemAtIndex(1).split(separators, IncludeSeparator::prefix);

					if (timeParts->count() == 1) throw ISO8601Exception();

					auto timeComponentsPart = timeParts->itemAtIndex(0);
					auto timeZonePart = timeParts->itemAtIndex(1);

					auto timeComponentsSubParts = timeComponentsPart.split(".");

					if (timeComponentsSubParts->count() > 0) {

						auto hmsComponentParts = timeComponentsSubParts->itemAtIndex(0);

						hours = hmsComponentParts.substring(0, 2)->doubleValue();

						if (hmsComponentParts.length() == 6) {
							minutes = hmsComponentParts.substring(2, 2)->doubleValue();
							seconds = hmsComponentParts.substring(4, 2)->doubleValue();
						}
						else if (timeComponentsPart.length() == 8) {
							minutes = hmsComponentParts.substring(3, 2)->doubleValue();
							seconds = hmsComponentParts.substring(6, 2)->doubleValue();
						}

						if (timeComponentsSubParts->count() > 1) {
							microseconds = timeComponentsSubParts->itemAtIndex(1).doubleValue();
						}

					} else throw ISO8601Exception();

					if (timeZonePart.length() > 0) {
						timeZoneOffset = Duration::parse(timeZonePart);
					}

				}

			}
			else ISO8601Exception();

			this->_set(year, month, day, hours, minutes, seconds, microseconds);

			_time -= timeZoneOffset;

		}

		virtual ~Date() {}

		int64_t year() const {
			int64_t year;
			_components(this->_time.days(), &year, nullptr, nullptr);
			return year;
		}

		Month month() const {
			uint8_t month;
			_components(this->_time.days(), nullptr, &month, nullptr);
			return Month(month);
		}

		int16_t day() const {
			uint8_t day;
			_components(this->_time.days(), nullptr, nullptr, &day);
			return day;
		}

		Weekday weekday() const {
			return static_cast<Weekday>(((int64_t)this->_time.days() + static_cast<int64_t>(_epochWeekday)) % daysInWeek);
		}

		const Duration durationSinceMidnight() const {
			auto seconds = this->_time.seconds();
			double daysSeconds = floor(this->_time.days()) * Duration::day();
			return Duration(seconds - daysSeconds);
		}

		bool isLeapYear() {
			return Date::isLeapYear(this->year());
		}

		uint8_t hours() const {
			return this->durationSinceMidnight() / Duration::hour();
		}

		uint8_t minutes() const {
			return (this->durationSinceMidnight().seconds() - (this->hours() * Duration::hour())) / Duration::minute();
		}

		uint8_t seconds() const {
			return (this->durationSinceMidnight().seconds()) - (this->hours() * Duration::hour()) - (this->minutes() * Duration::minute());
		}

		uint32_t microseconds() const {
			return (this->_time.seconds() - floor(this->_time.seconds())) * 1000000;
		}

		const Duration durationSinceEpoch() const {
			return this->_time;
		}

		Duration since(const Date& other) const {
			return this->durationSinceEpoch() - other.durationSinceEpoch();
		}

		Date to(TimeZone timeZone) const {
			if (this->_timeZone == timeZone) return Date(*this);
			if (timeZone == TimeZone::local) return Date(this->_time + _localOffset(), TimeZone::local);
			return Date(this->_time - _localOffset(), TimeZone::utc);
		}

		String toISO8601() const {
			String ret;
			ret.append(String::format("%02lld-%02d-%02dT%02d:%02d:%02d",
									  this->year(),
									  this->month(),
									  this->day(),
									  this->hours(),
									  this->minutes(),
									  this->seconds()));
			auto microseconds = this->microseconds();
			if (microseconds != 0) {
				ret.append(String::format(".%llu", microseconds));
			}
			switch (this->_timeZone) {
				case TimeZone::utc:
					ret.append("Z");
					break;
				case TimeZone::local: {
					ret.append(_localOffset().toString(Duration::ToStringOptions::prefixPositive));
					break;
				}
			}
			return ret;
		}

		virtual Kind kind() const override {
			return Kind::date;
		}

		virtual uint64_t hash() const override {
			double seconds = this->_time.seconds();
			uint64_t hash;
			memcpy(&hash, &seconds, sizeof(uint64_t));
			return hash;
		}

		Date operator+(const Duration& duration) const {
			return Date(this->seconds() + duration.seconds());
		}

		Date operator-(const Duration& duration) const {
			return Date(this->seconds() - duration.seconds());
		}

		const Duration operator-(const Date& other) const {
			return this->seconds() - other.seconds();
		}

		void operator+=(const Duration& duration) {
			_time += duration;
		}

		void operator-=(const Duration& duration) {
			_time -= duration;
		}

		virtual bool operator>(const Date& other) const override {
			return this->to(TimeZone::utc).seconds() > other.to(TimeZone::utc).seconds();
		}

		Date& operator=(const Date& other) {
			Type::operator=(other);
			this->_time = other._time;
			this->_timeZone = other._timeZone;
			return *this;
		}

		operator time_t() const {
			return (time_t)this->_time.seconds();
		}

	};

}

#endif /* date_hpp */
