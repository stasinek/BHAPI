/*
 * Copyright 2010-2011, Oliver Tappe, zooey@hirschkaefer.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ICU_MONETARY_DATA_H
#define _ICU_MONETARY_DATA_H


#include <ICULocaleconvData.h>
#include <LocaleBackend.h>

#include <locale.h>


namespace BPrivate {
namespace Libroot {


class ICUMonetaryData : public ICULocaleconvData {
	typedef	ICULocaleconvData	inherited;

public:
	static	const int32_t			kParenthesesAroundCurrencyAndValue = 0;
	static	const int32_t			kSignPrecedesCurrencyAndValue      = 1;
	static	const int32_t			kSignSucceedsCurrencyAndValue      = 2;
	static	const int32_t			kSignImmediatelyPrecedesCurrency   = 3;
	static	const int32_t			kSignImmediatelySucceedsCurrency   = 4;

								ICUMonetaryData(pthread_key_t tlsKey,
									struct lconv& localeConv);

			void				Initialize(
									LocaleMonetaryDataBridge* dataBridge);

	virtual	status_t			SetTo(const Locale& locale,
									const char* posixLocaleName);
	virtual	status_t			SetToPosix();

			const char*			GetLanginfo(int index);

private:
	static	const int32_t			kCsPrecedesFlag = 1 << 0;
	static	const int32_t			kSepBySpaceFlag = 1 << 1;

			int32_t				_DetermineCurrencyPosAndSeparator(
									const UnicodeString& prefix,
									const UnicodeString& suffix,
									const UnicodeString& signSymbol,
									const UnicodeString& currencySymbol,
									UChar& currencySeparatorChar);
			int32_t				_DetermineSignPos(const UnicodeString& prefix,
									const UnicodeString& suffix,
									const UnicodeString& signSymbol,
									const UnicodeString& currencySymbol);

			char				fDecimalPoint[skLCBufSize];
			char				fThousandsSep[skLCBufSize];
			char				fGrouping[skLCBufSize];
			char				fIntCurrSymbol[skLCBufSize];
			char				fCurrencySymbol[skLCBufSize];
			char				fPositiveSign[skLCBufSize];
			char				fNegativeSign[skLCBufSize];

			struct lconv&		fLocaleConv;
			const struct lconv*	fPosixLocaleConv;
};


}	// namespace Libroot
}	// namespace BPrivate


#endif	// _ICU_MONETARY_DATA_H
