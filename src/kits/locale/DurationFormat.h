/*
 * Copyright 2010-2011, Haiku, Inc.
 * Distributed under the terms of the MIT License.
 */
#ifndef _B_DURATION_FORMAT_H_
#define _B_DURATION_FORMAT_H_


#include <Format.h>
#include <LocaleClass.h>
#include <kits/support/String.h>
#include <TimeUnitFormat.h>


class BTimeZone;

#ifndef U_ICU_NAMESPACE
  #define U_ICU_NAMESPACE icu
#endif
namespace U_ICU_NAMESPACE {
	class GregorianCalendar;
}


class BDurationFormat : public BFormat {
	typedef	BFormat				Inherited;

public:
								BDurationFormat(const BLanguage& language,
									const BFormattingConventions& conventions,
									const BString& separator = ", ");
								BDurationFormat(
									const BString& separator = ", ");
								BDurationFormat(const BDurationFormat& other);
	virtual						~BDurationFormat();

			void				SetSeparator(const BString& separator);
			status_t			SetTimeZone(const BTimeZone* timeZone);

			status_t			Format(BString& buffer,
									const bigtime_t startValue,
									const bigtime_t stopValue,
									time_unit_style style = B_TIME_UNIT_FULL
									) const;

private:
			BString				fSeparator;
			BTimeUnitFormat		fTimeUnitFormat;
			U_ICU_NAMESPACE::GregorianCalendar*	fCalendar;
};


#endif
