/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku source-code, All Rights Reserved
 *
 * BHAPI++ library is a freeware; it may be used and distributed according to
 * the terms of The MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File: DateTime.h
 *
 * --------------------------------------------------------------------------*/
/*
 * Copyright 2007-2010 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_DATE_TIME_H
#define BHAPI_DATE_TIME_H

#include <kits/support/String.h>

class BMessage;
namespace BPrivate {

enum time_type {
	B_GMT_TIME,
	B_LOCAL_TIME
};


enum diff_type {
	B_HOURS_DIFF,
	B_MINUTES_DIFF,
	B_SECONDS_DIFF,
	B_MILLISECONDS_DIFF,
	B_MICROSECONDS_DIFF
};

class BTime {
public:
								BTime();
								BTime(const BTime& other);
								BTime(int32_t hour, int32_t minute, int32_t second,
									int32_t microsecond = 0);
								BTime(const BMessage* archive);
								~BTime();

			status_t			Archive(BMessage* into) const;

			bool				IsValid() const;
	static	bool				IsValid(const BTime& time);
	static	bool				IsValid(int32_t hour, int32_t minute, int32_t second,
									int32_t microsecond = 0);

	static	BTime				CurrentTime(time_type type);

			BTime				Time() const;
			bool				SetTime(const BTime& time);
			bool				SetTime(int32_t hour, int32_t minute, int32_t second,
									int32_t microsecond = 0);

			BTime&				AddHours(int32_t hours);
			BTime&				AddMinutes(int32_t minutes);
			BTime&				AddSeconds(int32_t seconds);
			BTime&				AddMilliseconds(int32_t milliseconds);
			BTime&				AddMicroseconds(int32_t microseconds);

			int32_t				Hour() const;
			int32_t				Minute() const;
			int32_t				Second() const;
			int32_t				Millisecond() const;
			int32_t				Microsecond() const;
			bigtime_t			Difference(const BTime& time,
									diff_type type) const;

			bool				operator!=(const BTime& time) const;
			bool				operator==(const BTime& time) const;

			bool				operator<(const BTime& time) const;
			bool				operator<=(const BTime& time) const;

			bool				operator>(const BTime& time) const;
			bool				operator>=(const BTime& time) const;

private:
			bigtime_t			_Microseconds() const;
			BTime&				_AddMicroseconds(bigtime_t microseconds);
			bool				_SetTime(bigtime_t hour, bigtime_t minute,
									bigtime_t second, bigtime_t microsecond);

private:
			bigtime_t			fMicroseconds;
};

class BDate {
public:
								BDate();
								BDate(const BDate& other);
								BDate(int32_t year, int32_t month, int32_t day);
								BDate(time_t time,
									time_type type = B_LOCAL_TIME);
								BDate(const BMessage* archive);
								~BDate();

			status_t			Archive(BMessage* into) const;

			bool				IsValid() const;
	static	bool				IsValid(const BDate& date);
	static	bool				IsValid(int32_t year, int32_t month,
									int32_t day);

	static	BDate				CurrentDate(time_type type);

			BDate				Date() const;
			bool				SetDate(const BDate& date);

			bool				SetDate(int32_t year, int32_t month, int32_t day);
			void				GetDate(int32_t* year, int32_t* month,
									int32_t* day) const;

			void				AddDays(int32_t days);
			void				AddYears(int32_t years);
			void				AddMonths(int32_t months);

			int32_t				Day() const;
			int32_t				Year() const;
			int32_t				Month() const;
			int32_t				Difference(const BDate& date) const;

			void				SetDay(int32_t day);
			void				SetMonth(int32_t month);
			void				SetYear(int32_t year);

			int32_t				DayOfWeek() const;
			int32_t				DayOfYear() const;

			int32_t				WeekNumber() const;
			bool				IsLeapYear() const;
	static	bool				IsLeapYear(int32_t year);

			int32_t				DaysInYear() const;
			int32_t				DaysInMonth() const;

			BString				ShortDayName() const;
	static	BString				ShortDayName(int32_t day);

			BString				ShortMonthName() const;
	static	BString				ShortMonthName(int32_t month);

			BString				LongDayName() const;
	static	BString				LongDayName(int32_t day);

			BString				LongMonthName() const;
	static	BString				LongMonthName(int32_t month);

			int32_t				DateToJulianDay() const;
	static	BDate				JulianDayToDate(int32_t julianDay);

			bool				operator!=(const BDate& date) const;
			bool				operator==(const BDate& date) const;

			bool				operator<(const BDate& date) const;
			bool				operator<=(const BDate& date) const;

			bool				operator>(const BDate& date) const;
			bool				operator>=(const BDate& date) const;

private:
	static	int32_t				_DaysInMonth(int32_t year, int32_t month);
			bool				_SetDate(int32_t year, int32_t month, int32_t day);
	static	int32_t				_DateToJulianDay(int32_t year, int32_t month,
									int32_t day);

private:
			int32_t				fDay;
			int32_t				fYear;
			int32_t				fMonth;
};

class BDateTime {
public:
								BDateTime();
								BDateTime(const BDate &date, const BTime &time);
								BDateTime(const BMessage* archive);
								~BDateTime();

			status_t			Archive(BMessage* into) const;

			bool				IsValid() const;

	static	BDateTime			CurrentDateTime(time_type type);
			void				SetDateTime(const BDate &date, const BTime &time);

			BDate&				Date();
			const BDate&		Date() const;
			void				SetDate(const BDate &date);

			BTime&				Time();
			const BTime&		Time() const;
			void				SetTime(const BTime &time);

			time_t				Time_t() const;
			void				SetTime_t(time_t seconds);

			bool				operator!=(const BDateTime& dateTime) const;
			bool				operator==(const BDateTime& dateTime) const;

			bool				operator<(const BDateTime& dateTime) const;
			bool				operator<=(const BDateTime& dateTime) const;

			bool				operator>(const BDateTime& dateTime) const;
			bool				operator>=(const BDateTime& dateTime) const;

private:
			BDate				fDate;
			BTime				fTime;
};

}	// namespace BPrivate

using BPrivate::time_type;
using BPrivate::B_GMT_TIME;
using BPrivate::B_LOCAL_TIME;
using BPrivate::diff_type;
using BPrivate::B_HOURS_DIFF;
using BPrivate::B_MINUTES_DIFF;
using BPrivate::B_SECONDS_DIFF;
using BPrivate::B_MILLISECONDS_DIFF;
using BPrivate::B_MICROSECONDS_DIFF;
using BPrivate::BTime;
using BPrivate::BDate;
using BPrivate::BDateTime;

#endif	// BHAPI_DATE_TIME_H
