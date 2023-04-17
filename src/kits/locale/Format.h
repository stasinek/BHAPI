/*
 * Copyright 2003-2010, Haiku, Inc.
 * Distributed under the terms of the MIT Licence.
*/
#ifndef BHAPI_FORMAT_H
#define BHAPI_FORMAT_H

#include <FormattingConventions.h>
#include <FormatParameters.h>
#include <Locker.h>
#include <Language.h>
#include <Haiku.h>

// types of fields contained in formatted strings
enum {
    // number format fields
    B_CURRENCY_FIELD,
    B_DECIMAL_SEPARATOR_FIELD,
    B_EXPONENT_FIELD,
    B_EXPONENT_SIGN_FIELD,
    B_EXPONENT_SYMBOL_FIELD,
    B_FRACTION_FIELD,
    B_GROUPING_SEPARATOR_FIELD,
    B_INTEGER_FIELD,
    B_PERCENT_FIELD,
    B_PERMILLE_FIELD,
    B_SIGN_FIELD,

    // date format fields
    // TODO: ...
};

// structure filled in while formatting
struct format_field_position {
    uint32_t	field_type;
    int32_t	start;
    int32_t	length;
};

#ifndef BLOCALE_I
#define BLOCALE_I
class BLocale;
#endif

class BFormat {
public:
            status_t			InitCheck() const;
protected:
                                BFormat(const BLocale* locale = NULL);
                                BFormat(const BLanguage& language,
                                    const BFormattingConventions& conventions);

                                BFormat(const BFormat& other);
    virtual 					~BFormat();

private:
            BFormat&			operator=(const BFormat& other);

            status_t			_Initialize(const BLocale& locale);
            status_t			_Initialize(const BLanguage& language,
                                    const BFormattingConventions& conventions);

protected:
            BFormattingConventions	fConventions;
            BLanguage			fLanguage;
            status_t			fInitStatus;
};

#define BFORMAT_I
#endif	// BHAPI_FORMAT_H
