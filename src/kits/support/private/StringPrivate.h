/*
 * Copyright 2011, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_SUPPORT_BSTRING_PRIVATE_H
#define BHAPI_SUPPORT_BSTRING_PRIVATE_H
#include <kits/support/String.h>
//-------------------------------------------------------------------------------------------------
#ifdef __cplusplus
class BHAPI_IMPEXP BString::Private {
public:
    static const uint32_t_t kPrivateDataOffset = 2 * sizeof(int32_t_t);
public:
    Private(const BString& string);
    char* Data();
    bool IsShareable() const;
    static int32_t_t& DataRefCount(char* data);
    int32_t_t& DataRefCount();
    static int32_t_t& DataLength(char* data);
    int32_t_t& DataLength();
    static void IncrementDataRefCount(char* data);
    static void DecrementDataRefCount(char* data);
    static BString StringFromData(char* data);
private:
    const BString&	fString;
};
#define BSTRING_PRIVATE_DEF
#endif // C++
//-------------------------------------------------------------------------------------------------
#endif// BHAPI_SUPPORT_BSTRING_PRIVATE_H
//-------------------------------------------------------------------------------------------------
