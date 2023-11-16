/*
 * Copyright 2010, Michael Lotz <mmlr@mlotz.ch>. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef TRUNCATE_STRING_H
#define TRUNCATE_STRING_H

#include <Haiku.h>

void truncate_string(BString& string, uint32_t mode, float width,
	const float* escapementArray, float fontSize, float ellipsisWidth,
	int32_t numChars);

#endif // TRUNCATE_STRING_H
