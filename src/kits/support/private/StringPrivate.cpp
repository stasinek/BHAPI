#include <kits/support/private/StringPrivate.h>
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

int32_t& BString::Private::DataRefCount(char* data)
{
    return *(((int32_t *)data) - 2);
}
//-------------------------------------------------------------------------------------------------

int32_t& BString::Private::DataRefCount()
{
    return DataRefCount(Data());
}
//-------------------------------------------------------------------------------------------------

int32_t& BString::Private::DataLength(char* data)
{
    return *(((int32_t*)data) - 1);
}
//-------------------------------------------------------------------------------------------------

int32_t& BString::Private::DataLength()
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
