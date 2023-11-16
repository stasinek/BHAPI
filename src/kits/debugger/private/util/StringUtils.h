/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <kits/support/String.h>


class StringUtils {
public:
	static	uint32_t				HashValue(const char* string);
	static	uint32_t				HashValue(const BString& string);
};


/*static*/ inline uint32_t StringUtils::HashValue(const BString& string)
{
	return HashValue(string.String());
}


#endif	// STRING_UTILS_H
