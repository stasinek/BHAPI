﻿/*
 * Copyright 2003-2014, Haiku, Inc.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_B_NUMBER_FORMAT_H
#define BHAPI_B_NUMBER_FORMAT_H

#include <Format.h>
#include <Errors.h>

enum BNumberElement {
    B_NUMBER_ELEMENT_INVALID = B_BAD_DATA,
    B_NUMBER_ELEMENT_INTEGER = 0,
    B_NUMBER_ELEMENT_FRACTIONAL,
    B_NUMBER_ELEMENT_CURRENCY
};


class BNumberFormat : public BFormat {
public:
                                BNumberFormat();
                                BNumberFormat(const BNumberFormat &other);
                                ~BNumberFormat();

                                // formatting

            ssize_t				Format(char* string, size_t maxSize,
                                    const double value) const;
            status_t			Format(BString& string, const double value)
                                    const;
            ssize_t				Format(char* string, size_t maxSize,
                                    const int32_t value) const;
            status_t			Format(BString& string, const int32_t value)
                                    const;

                                // monetary

            ssize_t				FormatMonetary(char* string, size_t maxSize,
                                    const double value) const;
            status_t			FormatMonetary(BString& string,
                                    const double value) const;
};


#endif	// _B_NUMBER_FORMAT_H_
