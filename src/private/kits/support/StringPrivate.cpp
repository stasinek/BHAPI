#include "StringPrivate.h"
using namespace bhapi;
//-------------------------------------------------------------------------------------------------

BString::Private::Private(const BString& string)
    :
    fString(string)
{
}
//-------------------------------------------------------------------------------------------------

char* BString::Private::Data()
{
    return fString.fPrivateData;
}
//-------------------------------------------------------------------------------------------------

bool BString::Private::IsShareable() const
{
    return fString.__IsShareable();
}
//-------------------------------------------------------------------------------------------------

int32& BString::Private::DataRefCount(char* data)
{
    return *(((int32 *)data) - 2);
}
//-------------------------------------------------------------------------------------------------

int32& BString::Private::DataRefCount()
{
    return DataRefCount(Data());
}
//-------------------------------------------------------------------------------------------------

int32& BString::Private::DataLength(char* data)
{
    return *(((int32*)data) - 1);
}
//-------------------------------------------------------------------------------------------------

int32& BString::Private::DataLength()
{
    return DataLength(Data());
}
//-------------------------------------------------------------------------------------------------

void BString::Private::IncrementDataRefCount(char* data)
{
    if (data != NULL)
        atomic_add(&DataRefCount(data), 1);
}
//-------------------------------------------------------------------------------------------------

void BString::Private::DecrementDataRefCount(char* data)
{
    if (data != NULL) {
        if (atomic_add(&DataRefCount(data), -1) == 1)
            free(data - kPrivateDataOffset);
    }
}
//-------------------------------------------------------------------------------------------------

BString BString::Private::StringFromData(char* data)
{
    return BString(data, BString::PRIVATE_DATA);
}
