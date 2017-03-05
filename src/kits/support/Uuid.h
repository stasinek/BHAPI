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
 * File: Uuid.h
 *
 * --------------------------------------------------------------------------*/

/*
 * Copyright 2013, Haiku, Inc.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_UUID_H
#define BHAPI_UUID_H

#include "SupportDefs.h"

namespace BPrivate {
class BString;
class BUuid {
public:
								BUuid();
								BUuid(const BUuid& other);
								~BUuid();

			bool				IsNil() const;

			BUuid&				SetToRandom();

			BString				ToString() const;

			int					Compare(const BUuid& other) const;

	inline	bool				operator==(const BUuid& other) const;
	inline	bool				operator!=(const BUuid& other) const;

	inline	bool				operator<(const BUuid& other) const;
	inline	bool				operator>(const BUuid& other) const;
	inline	bool				operator<=(const BUuid& other) const;
	inline	bool				operator>=(const BUuid& other) const;

			BUuid&				operator=(const BUuid& other);

private:
			bool				_SetToDevRandom();
			void				_SetToRandomFallback();

private:
			uint8				fValue[16];
};

inline bool BUuid::operator==(const BUuid& other) const
{
	return Compare(other) == 0;
}

inline bool BUuid::operator!=(const BUuid& other) const
{
	return Compare(other) != 0;
}

inline bool BUuid::operator<(const BUuid& other) const
{
	return Compare(other) < 0;
}

inline bool BUuid::operator>(const BUuid& other) const
{
	return Compare(other) > 0;
}

inline bool BUuid::operator<=(const BUuid& other) const
{
	return Compare(other) <= 0;
}

inline bool BUuid::operator>=(const BUuid& other) const
{
	return Compare(other) >= 0;
}


}	// namespace BPrivate
using BPrivate::BUuid;
#endif	// BHAPI_UUID_H_
