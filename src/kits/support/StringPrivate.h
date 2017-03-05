/*
 * Copyright 2011, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_SUPPORT_BSTRING_PRIVATE_H
#define BHAPI_SUPPORT_BSTRING_PRIVATE_H

#include "../support/String.h"

class BString::Private {
public:
	static const uint32 kPrivateDataOffset = 2 * sizeof(int32);
public:
    Private(const BString& string);
    char* Data();
    bool IsShareable() const;
    static int32& DataRefCount(char* data);
    int32& DataRefCount();
    static int32& DataLength(char* data);
    int32& DataLength();
    static void IncrementDataRefCount(char* data);
    static void DecrementDataRefCount(char* data);
    static BString StringFromData(char* data);
private:
	const BString&	fString;
};

#endif	// BHAPI_SUPPORT_BSTRING_PRIVATE_H

