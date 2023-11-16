/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
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
 * File: Message.cpp
 *
 * --------------------------------------------------------------------------*/

#include <kits/netMessage.h>
#include <kits/app/Messenger.h>
#include <kits/app/Handler.h>

#include <kits/interface/Point.h>
#include <kits/interface/Rect.h>
#include <os/debug.h>
#include <os/kernel.h>
#include <kits/support/StreamIO.h>
#include <kits/support/String.h>
#include <kits/debug/Errors.h>
#include <kits/private/support/Token.h>

status_t BMessage::BGetInfo(type_code type,  int32_t index,
           char **nameFound, type_code *typeFound,  int32_t *countFound) const
{
    if(index < 0) return B_BAD_INDEX;
     int32_t aIndex = index;

    for(int32_t i = 0; i < CountNames(B_ANY_TYPE, true); i++)
    {
         int32_t typesCount = CountTypesByName(i);
        for(int32_t k = 0; k < typesCount; k++)
        {
            type_code aType;
             int32_t count = CountItems(i, k, &aType);
            if(!(type == B_ANY_TYPE || aType == type) || (aIndex--) > 0) continue;
            if(nameFound) *nameFound = (char*)NameAt(i);
            if(typeFound) *typeFound = aType;
            if(countFound) *countFound = count;
            return B_OK;
        }
    }

    return(aIndex == index ? B_BAD_TYPE : B_BAD_INDEX);
}


status_t BMessage::BFindData(const char *name, type_code type,  int32_t index,
            const void **data,  ssize_t *numBytes) const
{
    if(index < 0) return B_BAD_INDEX;

     int32_t nameIndex = FindName(name);
    if(nameIndex < 0) return B_NAME_NOT_FOUND;

     int32_t typesCount = CountTypesByName(nameIndex);
     int32_t aIndex = index;

    for(int32_t k = 0; k < typesCount; k++)
    {
        type_code aType;
         int32_t count = CountItems(nameIndex, k, &aType);
        if(!(type == B_ANY_TYPE || aType == type)) continue;

        if(aIndex < count)
            return(FindData(nameIndex, k, aIndex, data, numBytes) ? B_OK : B_ERROR);

        aIndex -= count;
    }

    return(aIndex == index ? B_BAD_TYPE : B_BAD_INDEX);
}


status_t BMessage::BFindData(const char *name, type_code type,
            const void **data,  ssize_t *numBytes) const
{
    return BFindData(name, type, 0, data, numBytes);
}

BMessage::BMessage()
    : what(0),
      fTargetToken(B_MAXUINT64), fTargetTokenTimestamp(B_INT64_CONSTANT(0)),
      fReplyToken(B_MAXUINT64), fReplyTokenTimestamp(B_INT64_CONSTANT(0)),
      fNoticeSource(false), fSource(NULL), fIsReply(false)
{
    fTeam = bhapi::get_current_team_id();
}


BMessage::BMessage(uint32_t a_what)
    : fTargetToken(B_MAXUINT64), fTargetTokenTimestamp(B_INT64_CONSTANT(0)),
      fReplyToken(B_MAXUINT64), fReplyTokenTimestamp(B_INT64_CONSTANT(0)),
      fNoticeSource(false), fSource(NULL), fIsReply(false)
{
    BMessage::what = a_what;
    fTeam = bhapi::get_current_team_id();
}


BMessage::BMessage(const BMessage &msg)
    : what(0), fTeam(B_INT64_CONSTANT(0)),
      fTargetToken(B_MAXUINT64), fTargetTokenTimestamp(B_INT64_CONSTANT(0)),
      fReplyToken(B_MAXUINT64), fReplyTokenTimestamp(B_INT64_CONSTANT(0)),
      fNoticeSource(false), fSource(NULL), fIsReply(false)
{
    operator=(msg);
}


BMessage& BMessage::operator=(const BMessage &msg)
{
    what = msg.what;

    MakeEmpty();

    for(int32_t k = 0; k < msg.fObjectsList.CountItems(); k++)
    {
        list_data *ldata = (list_data*)msg.fObjectsList.ItemAt(k);
        if(!ldata) continue;

        for(int32_t j = 0; j < ldata->list.CountItems(); j++)
        {
            type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(j);
            if(!tldata) continue;

            for(int32_t i = 0; i < tldata->list.CountItems(); i++)
            {
                _object_t *Object = (_object_t*)tldata->list.ItemAt(i);
                if(!Object) continue;

                AddData(ldata->name, tldata->type, Object->data, Object->bytes, Object->fixed_size);
            }
        }
    }

    if(fSource != NULL)
    {
        if(fNoticeSource) bhapi::close_port(fSource);
        bhapi::delete_port(fSource);
        fSource = NULL;
    }

    fTargetToken = B_MAXUINT64;
    fTargetTokenTimestamp = B_MAXINT64;
    fReplyToken = B_MAXUINT64;
    fReplyTokenTimestamp = B_MAXINT64;

    if(msg.fTeam == bhapi::get_current_team_id())
    {
        fTargetToken = msg.fTargetToken;
        fTargetTokenTimestamp = msg.fTargetTokenTimestamp;
    }

    if(msg.fSource == NULL)
    {
        fReplyToken = msg.fReplyToken;
        fReplyTokenTimestamp = msg.fReplyTokenTimestamp;
    }
    else if(msg.fTeam == bhapi::get_current_team_id())
    {
        fSource = (port_id)bhapi::open_port_by_source((void*)(msg.fSource));
    }

    fTeam = msg.fTeam;
    fIsReply = msg.fIsReply;
    fNoticeSource = false;

    return *this;
}


size_t BMessage::FlattenedSize() const
{
    size_t size = sizeof(size_t) + sizeof(uint32_t) + sizeof(uint64); // FlattenSize + msg->what + recordCount

    size += sizeof(int64); // fTeam
    size += sizeof(uint64) + sizeof(bigtime_t); // fTargetToken + fTargetTokenTimestamp
    size += sizeof(uint64) + sizeof(bigtime_t); // fReplyToken + fReplyTokenTimestamp
    size += sizeof(void*); // fSource
    size += sizeof(bool); // fIsReply

    for(int32_t k = 0; k < fObjectsList.CountItems(); k++)
    {
        list_data *ldata = (list_data*)fObjectsList.ItemAt(k);
        if(!ldata) continue;

        size_t nameLen = (size_t)(ldata->name ? strlen(ldata->name) : 0);

        for(int32_t j = 0; j < ldata->list.CountItems(); j++)
        {
            type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(j);
            if(!tldata) continue;

            for(int32_t i = 0; i < tldata->list.CountItems(); i++)
            {
                _object_t *Object = (_object_t*)tldata->list.ItemAt(i);
                if(!Object) continue;

                // msg->_object_t
                size_t objectSize = sizeof(size_t) + nameLen + sizeof(type_code) + sizeof(bool) + sizeof(size_t);
                size_t dataLen = Object->fixed_size ? Object->bytes : sizeof(void*);
                objectSize += dataLen;

                size += objectSize;
            }
        }
    }

    return size;
}


bool BMessage::Flatten(char *buffer, size_t bufferSize) const
{
    if(buffer == NULL ||
       bufferSize < sizeof(size_t) + sizeof(uint32_t) + sizeof(uint64) +
            sizeof(int64) + 2 * (sizeof(uint64) + sizeof(bigtime_t)) + sizeof(void*) + sizeof(bool)) return false;

    size_t size = 0;

    // msg->what
    size += sizeof(size_t) + sizeof(uint32_t) + sizeof(uint64) +
        sizeof(int64) + 2 * (sizeof(uint64) + sizeof(bigtime_t)) + sizeof(void*) + sizeof(bool);
    char *dst = buffer;
    dst += sizeof(size_t);
    memcpy(dst, &what, sizeof(uint32_t)); dst += sizeof(uint32_t);

    // fTeam
    memcpy(dst, &fTeam, sizeof(int64)); dst += sizeof(int64);

    // fTargetToken + fTargetTokenTimestamp
    memcpy(dst, &fTargetToken, sizeof(uint64)); dst += sizeof(uint64);
    memcpy(dst, &fTargetTokenTimestamp, sizeof(bigtime_t)); dst += sizeof(bigtime_t);

    // fReplyToken + fReplyTokenTimestamp
    memcpy(dst, &fReplyToken, sizeof(uint64)); dst += sizeof(uint64);
    memcpy(dst, &fReplyTokenTimestamp, sizeof(bigtime_t)); dst += sizeof(bigtime_t);

    // fSource
     void* sourcb_address = reinterpret_cast<void*>(fSource);
    memcpy(dst, &sourcb_address, sizeof(void*)); dst += sizeof(void*);

    // fIsReply
    memcpy(dst, &fIsReply, sizeof(bool)); dst += sizeof(bool);

    // recordCount
    char *countDst = dst; dst += sizeof(uint64);
     uint64 count = (uint64)B_INT64_CONSTANT(0);

    for(int32_t k = 0; k < fObjectsList.CountItems(); k++)
    {
        list_data *ldata = (list_data*)fObjectsList.ItemAt(k);
        if(!ldata) continue;

        size_t nameLen = (size_t)(ldata->name ? strlen(ldata->name) : 0);

        for(int32_t j = 0; j < ldata->list.CountItems(); j++)
        {
            type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(j);
            if(!tldata) continue;

            for(int32_t i = 0; i < tldata->list.CountItems(); i++)
            {
                _object_t *Object = (_object_t*)tldata->list.ItemAt(i);
                if(!Object) continue;

                // msg->_object_t
                size_t dataLen = Object->fixed_size ? Object->bytes : sizeof(void*);
                size_t objectSize = sizeof(size_t) + nameLen + sizeof(type_code) + sizeof(bool) + sizeof(size_t) + dataLen;
                size += objectSize;

                if(bufferSize < size) return false;

                memcpy(dst, &nameLen, sizeof(size_t)); dst += sizeof(size_t);
                if(nameLen > 0) {memcpy(dst, ldata->name, nameLen); dst += nameLen;}
                memcpy(dst, &(tldata->type), sizeof(type_code)); dst += sizeof(type_code);
                memcpy(dst, &(Object->fixed_size), sizeof(bool)); dst += sizeof(bool);
                memcpy(dst, &dataLen, sizeof(size_t)); dst += sizeof(size_t);
                if(dataLen > 0)
                {
                    if(Object->fixed_size)
                    {
                        memcpy(dst, Object->data, dataLen);
                    }
                    else
                    {
                         void* address = reinterpret_cast<void*>(Object->data);
                        memcpy(dst, &address, dataLen);
                    }

                    dst += dataLen;
                }

                count++;
            }
        }
    }

    // recordCount
    memcpy(countDst, &count, sizeof(uint64));

    memcpy(buffer, &size, sizeof(size_t));

    return true;
}


bool BMessage::Unflatten(const char *buffer, size_t bufferSize)
{
    if(buffer == NULL ||
       bufferSize < sizeof(size_t) + sizeof(uint32_t) + sizeof(uint64) +
            sizeof(int64) + 2 * (sizeof(uint64) + sizeof(bigtime_t)) + sizeof(void*) + sizeof(bool)) return false;

    const char *src = buffer;
    BMessage msg;
     uint64 recordCount = 0;
    size_t _bufferSize = 0;

    memcpy(&_bufferSize, src, sizeof(size_t));
    if(bufferSize < _bufferSize) return false;
    src += sizeof(size_t); bufferSize -= sizeof(size_t);

    // msg->what
    memcpy(&msg.what, src, sizeof(uint32_t)); src += sizeof(uint32_t); bufferSize -= sizeof(uint32_t);

    // fTeam
    memcpy(&msg.fTeam, src, sizeof(int64)); src += sizeof(int64); bufferSize -= sizeof(int64);

    // fTargetToken + fTargetTokenTimestamp
    memcpy(&msg.fTargetToken, src, sizeof(uint64)); src += sizeof(uint64); bufferSize -= sizeof(uint64);
    memcpy(&msg.fTargetTokenTimestamp, src, sizeof(bigtime_t)); src += sizeof(bigtime_t); bufferSize -= sizeof(bigtime_t);

    // fReplyToken + fReplyTokenTimestamp
    memcpy(&msg.fReplyToken, src, sizeof(uint64)); src += sizeof(uint64); bufferSize -= sizeof(uint64);
    memcpy(&msg.fReplyTokenTimestamp, src, sizeof(bigtime_t)); src += sizeof(bigtime_t); bufferSize -= sizeof(bigtime_t);

    // fSource
     void* sourcb_address;
    memcpy(&sourcb_address, src, sizeof(void*)); src += sizeof(void*); bufferSize -= sizeof(void*);

    // fIsReply
    memcpy(&msg.fIsReply, src, sizeof(bool)); src += sizeof(bool); bufferSize -= sizeof(bool);

    // recordCount
    memcpy(&recordCount, src, sizeof(uint64)); src += sizeof(uint64); bufferSize -= sizeof(uint64);

    for(uint64 i = (uint64)B_INT64_CONSTANT(0); i < recordCount; i++)
    {
        // Object->name
        if(bufferSize < sizeof(size_t)) return false;
        size_t nameLen;
        memcpy(&nameLen, src, sizeof(size_t)); src += sizeof(size_t); bufferSize -= sizeof(size_t);

        char *name = NULL;
        if(nameLen > 0)
        {
            name = (char*)malloc(nameLen + 1);
            if(!name) return false;
            bzero(name, nameLen + 1);

            if(bufferSize < nameLen) {free(name); return false;}
            memcpy(name, src, nameLen); src += nameLen; bufferSize -= nameLen;
        }

        // Object->type
        if(bufferSize < sizeof(type_code)) {if(name) free(name); return false;}
        type_code type;
        memcpy(&type, src, sizeof(type_code)); src += sizeof(type_code); bufferSize -= sizeof(type_code);

        // Object->fixed_size
        if(bufferSize < sizeof(bool)) {if(name) free(name); return false;}
        bool fixed_size;
        memcpy(&fixed_size, src, sizeof(bool)); src += sizeof(bool); bufferSize -= sizeof(bool);

        // Object->bytes
        if(bufferSize < sizeof(size_t)) {if(name) free(name); return false;}
        size_t bytes;
        memcpy(&bytes, src, sizeof(size_t)); src += sizeof(size_t); bufferSize -= sizeof(size_t);

        // Object->data
        void *data = NULL;

        if(bytes > 0)
        {
            if(!fixed_size)
            {
                size_t dataLen = sizeof(void*);
                if(bytes != dataLen || bufferSize < dataLen) {if(name) free(name); return false;}

                 void* address = 0;
                memcpy(&address, src, dataLen);
                data = reinterpret_cast<void*>(address);
            }
            else
            {
                data = malloc(bytes);
                if(!data) {if(name) free(name); return false;}
                bzero(data, bytes);

                if(bufferSize < bytes) {if(name) free(name); free(data); return false;}
                memcpy(data, src, bytes);
            }

            src += bytes; bufferSize -= bytes;
        }

        // add to message
        if(msg.AddData(name ? name : "", type, data, bytes, fixed_size) == false)
            {if(name) free(name); if(data && fixed_size) free(data); return false;}

        if(name) free(name); if(data && fixed_size) free(data);
    }

    what = msg.what;
    MakeEmpty();
    fObjectsList = msg.fObjectsList;
    msg.fObjectsList.MakeEmpty();

    if(fSource != NULL)
    {
        if(fNoticeSource) bhapi::close_port(fSource);
        bhapi::delete_port(fSource);
        fSource = NULL;
    }

    fTargetToken = B_MAXUINT64;
    fTargetTokenTimestamp = B_MAXINT64;
    fReplyToken = B_MAXUINT64;
    fReplyTokenTimestamp = B_MAXINT64;

    if(msg.fTeam == bhapi::get_current_team_id())
    {
        fTargetToken = msg.fTargetToken;
        fTargetTokenTimestamp = msg.fTargetTokenTimestamp;
    }

    void *source = (msg.fTeam != bhapi::get_current_team_id() ? (void*)NULL : reinterpret_cast<void*>(sourcb_address));
    if(source == NULL)
    {
        if(msg.fTeam == bhapi::get_current_team_id())
        {
            fReplyToken = msg.fReplyToken;
            fReplyTokenTimestamp = msg.fReplyTokenTimestamp;
        }
    }
    else
    {
        // TODO: not safe
#if 0
        fSource = bhapi::open_port_by_source(source);
#endif
    }

    fTeam = msg.fTeam;
    fIsReply = msg.fIsReply;
    fNoticeSource = false;

    return true;
}


bool
BMessage::IsSystem() const
{
#ifdef BHAPI_BIG_ENDIAN
    return((what & 0xff) == '_');
#else
    return(((what >> 24) & 0xff) == '_');
#endif
}


void
BMessage::PrintToStream(BStreamIO &stream) const
{
    // TODO
}


void
BMessage::PrintToStream() const
{
    BHAPI_OUTPUT("what = '%c%c%c%c'\t\tteam = %I64i\n",
#ifdef BHAPI_BIG_ENDIAN
           what & 0xff, (what >> 8) & 0xff, (what >> 16) & 0xff, (what >> 24) & 0xff,
#else
           (what >> 24) & 0xff, (what >> 16) & 0xff, (what >> 8) & 0xff, what & 0xff,
#endif
           fTeam);
    if(fTargetToken != B_MAXUINT64)
        BHAPI_OUTPUT("Target token: %I64u\t\t", fTargetToken);
    else
        BHAPI_OUTPUT("No Target token\t\t");
    if(fReplyToken != B_MAXUINT64)
        BHAPI_OUTPUT("Reply token: %I64u\n", fReplyToken);
    else
        BHAPI_OUTPUT("No Reply token\n");
    BHAPI_OUTPUT("%s\t\t%s\n", (fIsReply ? "Reply message" : "Not reply message"), (fSource ? "Has source" : "No source"));

    for(int32_t k = 0; k < fObjectsList.CountItems(); k++)
    {
        list_data *ldata = (list_data*)fObjectsList.ItemAt(k);
        if(!ldata)
        {
            BHAPI_WARNING("Message::(%I32i) is NULL!", k);
            continue;
        }

        const char *name = ldata->name ? (strlen(ldata->name) > 0 ? (const char*)ldata->name : "NULL") : "ERROR-NULL";
         uint64 count = (uint64)B_INT64_CONSTANT(0);

        for(int32_t j = 0; j < ldata->list.CountItems(); j++)
        {
            type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(j);
            if(!tldata)
            {
                BHAPI_WARNING("Message::(%I32i - %I32i) is NULL!", k, j);
                continue;
            }

            for(int32_t i = 0; i < tldata->list.CountItems(); i++)
            {
                _object_t *Object = (_object_t*)tldata->list.ItemAt(i);

                if(!Object)
                {
                    BHAPI_WARNING("Message::(%I32i - %I32i - %I32i) is NULL!",
                           k, j, i);
                    continue;
                }
                else
                {
                    count++;
                    BHAPI_OUTPUT("%s[%I64u]:", name, count);
                    if(Object->data == NULL)
                    {
                        BHAPI_OUTPUT("\tWARNING: *** NO DATA ***\n");
                    }
                    else
                    {
                        switch(tldata->type)
                        {
                            case B_STRING_TYPE:
                                BHAPI_OUTPUT("\tSTRING\t\"%s\"\n", (char*)Object->data);
                                break;

                            case B_INT8_TYPE:
                                BHAPI_OUTPUT("\tINT8\t%I8i\n", *((int8*)Object->data));
                                break;

                            case B_INT16_TYPE:
                                BHAPI_OUTPUT("\tINT16\t%I16i\n", *((int16*)Object->data));
                                break;

                            case B_INT32_TYPE:
                                BHAPI_OUTPUT("\tINT32\t%I32i\n", *((int32_t*)Object->data));
                                break;

                            case B_INT64_TYPE:
                                BHAPI_OUTPUT("\tINT64\t%I64i\n", *((int64*)Object->data));
                                break;

                            case B_BOOL_TYPE:
                                BHAPI_OUTPUT("\tBOOL\t%s\n", (*((bool*)Object->data) ? "true" : "false"));
                                break;

                            case B_FLOAT_TYPE:
                                BHAPI_OUTPUT("\tFLOAT\t%g\n", *((float*)Object->data));
                                break;

                            case B_DOUBLE_TYPE:
                                BHAPI_OUTPUT("\tDOUBLE\t%g\n", *((double*)Object->data));
                                break;

                            case B_POINT_TYPE:
                                {
                                    struct point_t {
                                        float x;
                                        float y;
                                    } *pt;

                                    pt = (struct point_t *)Object->data;

                                    BHAPI_OUTPUT("\tPOINT\t(%g,%g)\n", pt->x, pt->y);
                                }
                                break;

                            case B_RECT_TYPE:
                                {
                                    struct rect_t {
                                        float l;
                                        float t;
                                        float r;
                                        float b;
                                    } *r;

                                    r = (struct rect_t *)Object->data;

                                    BHAPI_OUTPUT("\tRECT\t(%g,%g,%g,%g)\n", r->l, r->t, r->r, r->b);
                                }
                                break;

                            default:
                                BHAPI_OUTPUT("\t'%c%c%c%c'\tbytes[%lu]  fixed_size[%s]  address[%p]\n",
#ifdef BHAPI_BIG_ENDIAN
                                       tldata->type & 0xff, (tldata->type >> 8) & 0xff,
                                       (tldata->type >> 16) & 0xff, (tldata->type >> 24) & 0xff,
#else
                                       (tldata->type >> 24) & 0xff, (tldata->type >> 16) & 0xff,
                                       (tldata->type >> 8) & 0xff, tldata->type & 0xff,
#endif
                                       Object->bytes,
                                       (Object->fixed_size ? "true" : "false"),
                                       Object->data);
                        }
                    }
                }
            }
        }
    }
}


BMessage::~BMessage()
{
    MakeEmpty();

    if(fSource != NULL)
    {
        if(fNoticeSource) bhapi::close_port(fSource);
        bhapi::delete_port(fSource);
    }
}


BMessage::list_data*
BMessage::_find_list(const BList *list, const char *name,  int32_t *index)
{
    if(!list || !name) return NULL;

    for(int32_t i = 0; i < list->CountItems(); i++)
    {
        list_data *data = (list_data*)list->ItemAt(i);
        if(!data || !data->name) continue;
        if(strcmp(data->name, name) == 0 && strlen(data->name) == strlen(name))
        {
            if(index) *index = i;
            return data;
        }
    }

    return NULL;
}


BMessage::type_list_data*
BMessage::_find_type_list(const BList *list, type_code type)
{
    if(!list) return NULL;

    for(int32_t i = 0; i < list->CountItems(); i++)
    {
        type_list_data *data = (type_list_data*)list->ItemAt(i);
        if(!data) continue;
        if(data->type == type) return data;
    }

    return NULL;
}


int32_t
BMessage::CountItems(const char *name, type_code type) const
{
    if(!name) return -1;

    list_data *ldata = _find_list(&fObjectsList, name);
    if(!ldata) return -1;

    type_list_data *tldata = _find_type_list(&(ldata->list), type);
    if(!tldata) return -1;

    return tldata->list.CountItems();
}


int32_t
BMessage::CountItems(int32_t nameIndex,  int32_t typeIndex, type_code *type) const
{
    list_data *ldata = (list_data*)fObjectsList.ItemAt(nameIndex);
    if(!ldata) return -1;

    type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(typeIndex);
    if(!tldata) return -1;

    if(type) *type = tldata->type;
    return tldata->list.CountItems();
}


bool
BMessage::TypeAt(const char *name,  int32_t typeIndex, type_code *type) const
{
    if(!name || !type) return false;

    list_data *ldata = _find_list(&fObjectsList, name);
    if(!ldata) return false;

    type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(typeIndex);
    if(!tldata) return false;

    *type = tldata->type;

    return true;
}


bool
BMessage::TypeAt(int32_t nameIndex,  int32_t typeIndex, type_code *type) const
{
    if(!type) return false;

    list_data *ldata = (list_data*)fObjectsList.ItemAt(nameIndex);
    if(!ldata) return false;

    type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(typeIndex);
    if(!tldata) return false;

    *type = tldata->type;

    return true;
}


int32_t
BMessage::CountTypesByName(const char *name) const
{
    if(!name) return -1;

    list_data *ldata = _find_list(&fObjectsList, name);
    if(!ldata) return -1;

    return ldata->list.CountItems();
}


int32_t
BMessage::CountTypesByName(int32_t nameIndex) const
{
    list_data *ldata = (list_data*)fObjectsList.ItemAt(nameIndex);
    if(!ldata) return -1;

    return ldata->list.CountItems();
}


int32_t
BMessage::CountNames(type_code type, bool count_all_names_when_any_type) const
{
    if(type == B_ANY_TYPE && count_all_names_when_any_type) return fObjectsList.CountItems();

     int32_t retVal = 0;

    for(int32_t i = 0; i < fObjectsList.CountItems(); i++)
    {
        list_data *ldata = (list_data*)fObjectsList.ItemAt(i);
        if(ldata == NULL) continue;

        for(int32_t k = 0; k < ldata->list.CountItems(); k++)
        {
            type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(i);
            if(tldata == NULL) continue;
            if(tldata->type == type) {retVal++; break;}
        }
    }

    return retVal;
}


int32_t
BMessage::FindName(const char *name) const
{
     int32_t index = -1;
    _find_list(&fObjectsList, name, &index);
    return index;
}


const char*
BMessage::NameAt(int32_t nameIndex) const
{
    list_data *data = (list_data*)fObjectsList.ItemAt(nameIndex);
    return(data ? data->name : NULL);
}


void
BMessage::MakeEmpty()
{
    for(int32_t k = 0; k < fObjectsList.CountItems(); k++)
    {
        list_data *ldata = (list_data*)fObjectsList.ItemAt(k);
        if(!ldata) continue;

        for(int32_t j = 0; j < ldata->list.CountItems(); j++)
        {
            type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(j);
            if(!tldata) continue;

            for(int32_t i = 0; i < tldata->list.CountItems(); i++)
            {
                _object_t *Object = (_object_t*)tldata->list.ItemAt(i);
                if(!Object) continue;

                if(Object->fixed_size && Object->data) free(Object->data);
                delete Object;
            }

            delete tldata;
        }

        delete[] ldata->name;
        delete ldata;
    }


    fObjectsList.MakeEmpty();
}


bool
BMessage::IsEmpty() const
{
    return fObjectsList.IsEmpty();
}


bool
BMessage::Rename(const char *old_entry, const char *new_entry)
{
    if(!old_entry || !new_entry) return false;
    if(strcmp(old_entry, new_entry) == 0 && strlen(old_entry) == strlen(new_entry)) return true;

    list_data *ldata = _find_list(&fObjectsList, new_entry);
    if(ldata) return false;

    ldata = _find_list(&fObjectsList, old_entry);
    if(!ldata) return false;

    char *newName = bhapi::strdup(new_entry);
    if(!newName) return false;

    delete[] ldata->name;
    ldata->name = newName;

    return true;
}


bool
BMessage::AddData(const char *name, type_code type, const void *data, size_t numBytes, bool is_fixed_size)
{
    if(!name) return false;
    if(!data && (!is_fixed_size || numBytes != 0)) return false;

    list_data *ldata = _find_list(&fObjectsList, name);

    _object_t *Object = new _object_t;
    if(!Object) return false;

    Object->bytes = is_fixed_size ? numBytes : sizeof(void*);
    Object->fixed_size = is_fixed_size;
    Object->data = NULL;
    if(is_fixed_size && numBytes > 0)
    {
        if((Object->data = malloc(numBytes)) == NULL)
        {
            delete Object;
            return false;
        }

        bzero(Object->data, numBytes);

        if(memcpy(Object->data, data, numBytes) == NULL)
        {
            free(Object->data);
            delete Object;
            return false;
        }
    }
    else
    {
        Object->data = (void*)data;
    }

    if(!ldata) // data not exist
    {
        type_list_data *tldata = NULL;

        if((ldata = new list_data) != NULL)
            if((ldata->name = bhapi::strdup(name)) != NULL)
                if((tldata = new type_list_data) != NULL)
                {
                    tldata->type = type;
                    if(tldata->list.AddItem((void*)Object))
                        if(ldata->list.AddItem((void*)tldata))
                            if(fObjectsList.AddItem((void*)ldata)) return true;
                }

        if(Object->fixed_size && Object->data) free(Object->data);
        delete Object;

        if(tldata) delete tldata;

        if(ldata)
        {
            if(ldata->name) delete[] ldata->name;
            delete ldata;
        }
    }
    else
    {
        type_list_data *tldata = _find_type_list(&(ldata->list), type);

        if(tldata) if(tldata->list.AddItem((void*)Object)) return true;

        if(Object->fixed_size && Object->data) free(Object->data);
        delete Object;
    }

    return false;
}


bool
BMessage::AddString(const char *name, const char *aString)
{
    if(!name || !aString) return false;

    return AddData(name, B_STRING_TYPE, (const void*)aString, (size_t)(strlen(aString) + 1), true);
}


bool
BMessage::AddString(const char *name, const BString &aString)
{
    return AddString(name, aString.String());
}


bool
BMessage::AddInt8(const char *name,  int8 val)
{
    if(!name) return false;
    return AddData(name, B_INT8_TYPE, (const void*)&val, sizeof(int8), true);
}


bool
BMessage::AddInt16(const char *name,  int16 val)
{
    if(!name) return false;
    return AddData(name, B_INT16_TYPE, (const void*)&val, sizeof(int16), true);
}


bool
BMessage::AddInt32(const char *name,  int32_t val)
{
    if(!name) return false;
    return AddData(name, B_INT32_TYPE, (const void*)&val, sizeof(int32_t), true);
}


bool
BMessage::AddInt64(const char *name,  int64 val)
{
    if(!name) return false;
    return AddData(name, B_INT64_TYPE, (const void*)&val, sizeof(int64), true);
}


bool
BMessage::AddBool(const char *name, bool aBoolean)
{
    if(!name) return false;
    return AddData(name, B_BOOL_TYPE, (const void*)&aBoolean, sizeof(bool), true);
}


bool
BMessage::AddFloat(const char *name, float aFloat)
{
    if(!name) return false;
    return AddData(name, B_FLOAT_TYPE, (const void*)&aFloat, sizeof(float), true);
}


bool
BMessage::AddDouble(const char *name, double aDouble)
{
    if(!name) return false;
    return AddData(name, B_DOUBLE_TYPE, (const void*)&aDouble, sizeof(double), true);
}


bool
BMessage::AddPoint(const char *name, BPoint pt)
{
    if(!name) return false;

    struct point_t {
        float x;
        float y;
    } apt;

    apt.x = pt.x;
    apt.y = pt.y;

    return AddData(name, B_POINT_TYPE, (const void*)&apt, sizeof(struct point_t), true);
}


bool
BMessage::AddRect(const char *name, BRect r)
{
    if(!name) return false;

    struct rect_t {
        float left;
        float top;
        float right;
        float bottom;
    } ar;

    ar.left = r.left;
    ar.top = r.top;
    ar.right = r.right;
    ar.bottom = r.bottom;

    return AddData(name, B_RECT_TYPE, (const void*)&ar, sizeof(struct rect_t), true);
}


bool
BMessage::AddPointer(const char *name, const void *ptr)
{
    if(!name || !ptr) return false;
    return AddData(name, B_POINTER_TYPE, ptr, 0, false);
}


bool
BMessage::AddMessage(const char *name, const BMessage *msg)
{
    if(!name || !msg) return false;

    size_t flattenedSize = msg->FlattenedSize();
    if(flattenedSize <= 0) return false;

    char *buffer = (char*)malloc(flattenedSize);
    if(!buffer) return false;

    if(msg->Flatten(buffer, flattenedSize) == false)
    {
        free(buffer);
        return false;
    }

    bool retval = AddData(name, B_MESSAGE_TYPE, buffer, flattenedSize, true);

    free(buffer);

    return retval;
}


bool
BMessage::AddMessenger(const char *name, const BMessenger *msgr)
{
    if(!name || !msgr) return false;

    size_t flattenedSize = msgr->FlattenedSize();
    if(flattenedSize <= 0) return false;

    char *buffer = (char*)malloc(flattenedSize + sizeof(size_t));
    if(!buffer) return false;

    memcpy(buffer, &flattenedSize, sizeof(size_t));
    if(msgr->Flatten(buffer + sizeof(size_t), flattenedSize) == false)
    {
        free(buffer);
        return false;
    }

    bool retval = AddData(name, B_MESSENGER_TYPE, buffer, flattenedSize + sizeof(size_t), true);

    free(buffer);

    return retval;
}


bool
BMessage::AddMessenger(const char *name, const BMessenger &msgr)
{
    return AddMessenger(name, &msgr);
}


bool
BMessage::FindData(const char *name, type_code type, const void **data,  ssize_t *numBytes) const
{
    return FindData(name, type, 0, data, numBytes);
}


bool
BMessage::FindData(const char *name, type_code type,  int32_t index, const void **data,  ssize_t *numBytes) const
{
    if(!name) return false;

    list_data *ldata = _find_list(&fObjectsList, name);
    if(!ldata) return false;

    type_list_data *tldata = _find_type_list(&(ldata->list), type);
    if(!tldata) return false;

    _object_t *Object = (_object_t*)tldata->list.ItemAt(index);
    if(!Object) return false;

    if(data) *data = Object->data;
    if(numBytes)
    {
        if(Object->fixed_size)
            *numBytes = (ssize_t)Object->bytes;
        else
            *numBytes = -1;
    }

    return true;
}


bool
BMessage::FindData(int32_t nameIndex,  int32_t typeIndex,  int32_t index, const void **data,  ssize_t *numBytes) const
{
    list_data *ldata = (list_data*)fObjectsList.ItemAt(nameIndex);
    if(!ldata) return false;

    type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(typeIndex);
    if(!tldata) return false;

    _object_t *Object = (_object_t*)tldata->list.ItemAt(index);
    if(!Object) return false;

    if(data) *data = Object->data;
    if(numBytes)
    {
        if(Object->fixed_size)
            *numBytes = (ssize_t)Object->bytes;
        else
            *numBytes = -1;
    }

    return true;
}


bool
BMessage::FindString(const char *name, const char **str) const
{
    return FindString(name, 0, str);
}


bool
BMessage::FindString(const char *name,  int32_t index, const char **str) const
{
    return FindData(name, B_STRING_TYPE, index, (const void**)str, NULL);
}


bool
BMessage::FindString(const char *name, BString *str) const
{
    return FindString(name, 0, str);
}


bool
BMessage::FindString(const char *name,  int32_t index, BString *str) const
{
    const char *string = NULL;
    if(!FindData(name, B_STRING_TYPE, index, (const void**)&string, NULL)) return false;
    if(str) str->SetTo(string);
    return true;
}


bool
BMessage::FindInt8(const char *name,  int8 *val) const
{
    return FindInt8(name, 0, val);
}


bool
BMessage::FindInt8(const char *name,  int32_t index,  int8 *val) const
{
    const  int8 *value = NULL;
    if(!FindData(name, B_INT8_TYPE, index, (const void**)&value, NULL)) return false;
    if(val) *val = *value;
    return true;
}


bool
BMessage::FindInt16(const char *name,  int16 *val) const
{
    return FindInt16(name, 0, val);
}


bool
BMessage::FindInt16(const char *name,  int32_t index,  int16 *val) const
{
    const  int16 *value = NULL;
    if(!FindData(name, B_INT16_TYPE, index, (const void**)&value, NULL)) return false;
    if(val) *val = *value;
    return true;
}


bool
BMessage::FindInt32(const char *name,  int32_t *val) const
{
    return FindInt32(name, 0, val);
}


bool
BMessage::FindInt32(const char *name,  int32_t index,  int32_t *val) const
{
    const  int32_t *value = NULL;
    if(!FindData(name, B_INT32_TYPE, index, (const void**)&value, NULL)) return false;
    if(val) *val = *value;
    return true;
}


bool
BMessage::FindInt64(const char *name,  int64 *val) const
{
    return FindInt64(name, 0, val);
}


bool
BMessage::FindInt64(const char *name,  int32_t index,  int64 *val) const
{
    const  int64 *value = NULL;
    if(!FindData(name, B_INT64_TYPE, index, (const void**)&value, NULL)) return false;
    if(val) *val = *value;
    return true;
}


bool
BMessage::FindBool(const char *name, bool *aBoolean) const
{
    return FindBool(name, 0, aBoolean);
}


bool
BMessage::FindBool(const char *name,  int32_t index, bool *aBoolean) const
{
    const bool *value = NULL;
    if(!FindData(name, B_BOOL_TYPE, index, (const void**)&value, NULL)) return false;
    if(aBoolean) *aBoolean = *value;
    return true;
}


bool
BMessage::FindFloat(const char *name, float *f) const
{
    return FindFloat(name, 0, f);
}


bool
BMessage::FindFloat(const char *name,  int32_t index, float *f) const
{
    const float *value = NULL;
    if(!FindData(name, B_FLOAT_TYPE, index, (const void**)&value, NULL)) return false;
    if(f) *f = *value;
    return true;
}


bool
BMessage::FindDouble(const char *name, double *d) const
{
    return FindDouble(name, 0, d);
}


bool
BMessage::FindDouble(const char *name,  int32_t index, double *d) const
{
    const double *value = NULL;
    if(!FindData(name, B_DOUBLE_TYPE, index, (const void**)&value, NULL)) return false;
    if(d) *d = *value;
    return true;
}


bool
BMessage::FindPoint(const char *name, BPoint *pt) const
{
    return FindPoint(name, 0, pt);
}


bool
BMessage::FindPoint(const char *name,  int32_t index, BPoint *pt) const
{
    struct point_t {
        float x;
        float y;
    };

    const struct point_t *apt = NULL;

    if(!FindData(name, B_POINT_TYPE, index, (const void**)&apt, NULL)) return false;
    if(pt) pt->Set(apt->x, apt->y);
    return true;
}


bool
BMessage::FindRect(const char *name, BRect *r) const
{
    return FindRect(name, 0, r);
}


bool
BMessage::FindRect(const char *name,  int32_t index, BRect *r) const
{
    struct rect_t {
        float left;
        float top;
        float right;
        float bottom;
    };

    const struct rect_t *ar = NULL;

    if(!FindData(name, B_RECT_TYPE, index, (const void**)&ar, NULL)) return false;
    if(r) r->Set(ar->left, ar->top, ar->right, ar->bottom);
    return true;
}


bool
BMessage::FindPointer(const char *name, void **ptr) const
{
    return FindPointer(name, 0, ptr);
}


bool
BMessage::FindPointer(const char *name,  int32_t index, void **ptr) const
{
    return FindData(name, B_POINTER_TYPE, index, (const void**)ptr, NULL);
}


bool
BMessage::FindMessage(const char *name, BMessage *msg) const
{
    return FindMessage(name, 0, msg);
}


bool
BMessage::FindMessage(const char *name,  int32_t index, BMessage *msg) const
{
    const char *buffer = NULL;
    if(!FindData(name, B_MESSAGE_TYPE, index, (const void**)&buffer, NULL)) return false;

    if(!buffer) return false;

    if(msg)
    {
        size_t bufferSize = 0;
        memcpy(&bufferSize, buffer, sizeof(size_t));
        if(msg->Unflatten(buffer, bufferSize) == false) return false;
    }

    return true;
}


bool
BMessage::FindMessenger(const char *name, BMessenger *msgr) const
{
    return FindMessenger(name, 0, msgr);
}


bool
BMessage::FindMessenger(const char *name,  int32_t index, BMessenger *msgr) const
{
    const char *buffer = NULL;
    if(!FindData(name, B_MESSENGER_TYPE, index, (const void**)&buffer, NULL)) return false;

    if(!buffer) return false;

    if(msgr)
    {
        size_t bufferSize = 0;
        memcpy(&bufferSize, buffer, sizeof(size_t));
        if(msgr->Unflatten(buffer + sizeof(size_t), bufferSize) == false) return false;
    }

    return true;
}


bool
BMessage::HasData(const char *name, type_code type,  int32_t index) const
{
    return FindData(name, type, index, NULL, NULL);
}


bool
BMessage::HasString(const char *name,  int32_t index) const
{
    return FindString(name, index, (const char**)NULL);
}


bool
BMessage::HasInt8(const char *name,  int32_t index) const
{
    return FindInt8(name, index, NULL);
}


bool
BMessage::HasInt16(const char *name,  int32_t index) const
{
    return FindInt16(name, index, NULL);
}


bool
BMessage::HasInt32(const char *name,  int32_t index) const
{
    return FindInt32(name, index, NULL);
}


bool
BMessage::HasInt64(const char *name,  int32_t index) const
{
    return FindInt64(name, index, NULL);
}


bool
BMessage::HasBool(const char *name,  int32_t index) const
{
    return FindBool(name, index, NULL);
}


bool
BMessage::HasFloat(const char *name,  int32_t index) const
{
    return FindFloat(name, index, NULL);
}


bool
BMessage::HasDouble(const char *name,  int32_t index) const
{
    return FindDouble(name, index, NULL);
}


bool
BMessage::HasPoint(const char *name,  int32_t index) const
{
    return FindPoint(name, index, NULL);
}


bool
BMessage::HasRect(const char *name,  int32_t index) const
{
    return FindRect(name, index, NULL);
}


bool
BMessage::HasPointer(const char *name,  int32_t index) const
{
    return FindPointer(name, index, NULL);
}


bool
BMessage::HasMessage(const char *name,  int32_t index) const
{
    return FindMessage(name, index, NULL);
}


bool
BMessage::HasMessenger(const char *name,  int32_t index) const
{
    return FindMessenger(name, index, NULL);
}


bool
BMessage::RemoveData(const char *name, type_code type,  int32_t index)
{
    if(!name) return false;

    list_data *ldata = _find_list(&fObjectsList, name);
    if(!ldata) return false;

    type_list_data *tldata = _find_type_list(&(ldata->list), type);
    if(!tldata) return false;

    _object_t *Object = (_object_t*)tldata->list.RemoveItem(index);
    if(!Object) return false;

    if(Object->fixed_size && Object->data) free(Object->data);
    delete Object;

    if(tldata->list.IsEmpty())
    {
        ldata->list.RemoveItem(tldata);
        delete tldata;
    }

    if(ldata->list.IsEmpty())
    {
        fObjectsList.RemoveItem(ldata);
        delete[] ldata->name;
        delete ldata;
    }

    return true;
}


bool
BMessage::RemoveData(const char *name, type_code type)
{
    if(!name) return false;

    list_data *ldata = _find_list(&fObjectsList, name);
    if(!ldata) return false;

    type_list_data *tldata = _find_type_list(&(ldata->list), type);
    if(!tldata) return false;

    for(int32_t i = 0; i < tldata->list.CountItems(); i++)
    {
        _object_t *Object = (_object_t*)tldata->list.ItemAt(i);
        if(!Object) continue;

        if(Object->fixed_size && Object->data) free(Object->data);
        delete Object;
    }

    ldata->list.RemoveItem(tldata);
    delete tldata;

    if(ldata->list.IsEmpty())
    {
        fObjectsList.RemoveItem(ldata);
        delete[] ldata->name;
        delete ldata;
    }

    return true;
}


bool
BMessage::RemoveData(const char *name)
{
    if(!name) return false;

    list_data *ldata = _find_list(&fObjectsList, name);
    if(!ldata) return false;

    for(int32_t j = 0; j < ldata->list.CountItems(); j++)
    {
        type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(j);
        if(!tldata) continue;

        for(int32_t i = 0; i < tldata->list.CountItems(); i++)
        {
            _object_t *Object = (_object_t*)tldata->list.ItemAt(i);
            if(!Object) continue;

            if(Object->fixed_size && Object->data) free(Object->data);
            delete Object;
        }

        delete tldata;
    }

    fObjectsList.RemoveItem(ldata);
    delete[] ldata->name;
    delete ldata;

    return true;
}



bool
BMessage::RemovString(const char *name,  int32_t index)
{
    return RemoveData(name, B_STRING_TYPE, index);
}


bool
BMessage::RemoveInt8(const char *name,  int32_t index)
{
    return RemoveData(name, B_INT8_TYPE, index);
}


bool
BMessage::RemoveInt16(const char *name,  int32_t index)
{
    return RemoveData(name, B_INT16_TYPE, index);
}


bool
BMessage::RemoveInt32(const char *name,  int32_t index)
{
    return RemoveData(name, B_INT32_TYPE, index);
}


bool
BMessage::RemoveInt64(const char *name,  int32_t index)
{
    return RemoveData(name, B_INT64_TYPE, index);
}


bool
BMessage::RemoveBool(const char *name,  int32_t index)
{
    return RemoveData(name, B_BOOL_TYPE, index);
}


bool
BMessage::RemoveFloat(const char *name,  int32_t index)
{
    return RemoveData(name, B_FLOAT_TYPE, index);
}


bool
BMessage::RemoveDouble(const char *name,  int32_t index)
{
    return RemoveData(name, B_DOUBLE_TYPE, index);
}


bool
BMessage::RemovePoint(const char *name,  int32_t index)
{
    return RemoveData(name, B_POINT_TYPE, index);
}


bool
BMessage::RemoveRect(const char *name,  int32_t index)
{
    return RemoveData(name, B_RECT_TYPE, index);
}


bool
BMessage::RemovePointer(const char *name,  int32_t index)
{
    return RemoveData(name, B_POINTER_TYPE, index);
}


bool
BMessage::RemoveMessage(const char *name,  int32_t index)
{
    return RemoveData(name, B_MESSAGE_TYPE, index);
}


bool
BMessage::RemoveMessenger(const char *name,  int32_t index)
{
    return RemoveData(name, B_MESSENGER_TYPE, index);
}


bool
BMessage::ReplaceData(const char *name, type_code type,  int32_t index, const void *data, size_t numBytes, bool is_fixed_size)
{
    if(!name) return false;
    if(!data && (!is_fixed_size || numBytes != 0)) return false;

    list_data *ldata = _find_list(&fObjectsList, name);
    if(!ldata) return false;

    type_list_data *tldata = _find_type_list(&(ldata->list), type);
    if(!tldata) return false;

    _object_t *Object = new _object_t;
    if(!Object) return false;

    Object->bytes = is_fixed_size ? numBytes : sizeof(void*);
    Object->fixed_size = is_fixed_size;
    Object->data = NULL;
    if(is_fixed_size && numBytes > 0)
    {
        if((Object->data = malloc(numBytes + 1)) == NULL)
        {
            delete Object;
            return false;
        }

        bzero(Object->data, numBytes);

        if(memcpy(Object->data, data, numBytes) == NULL)
        {
            free(Object->data);
            delete Object;
            return false;
        }
    }
    else
    {
        Object->data = (void*)data;
    }

    _object_t *oldObject = NULL;
    if(tldata->list.ReplaceItem(index, (void*)Object, (void**)&oldObject) == false)
    {
        if(Object->fixed_size && Object->data) free(Object->data);
        delete Object;
        return false;
    }

    if(oldObject)
    {
        if(oldObject->fixed_size && oldObject->data) free(oldObject->data);
        delete oldObject;
    }

    return true;
}


bool
BMessage::ReplaceData(const char *name, type_code type, const void *data, size_t numBytes, bool is_fixed_size)
{
    return ReplaceData(name, type, 0, data, numBytes, is_fixed_size);
}


bool
BMessage::ReplacString(const char *name,  int32_t index, const char *aString)
{
    if(!name || !aString) return false;
    return ReplaceData(name, B_STRING_TYPE, index, (const void*)aString, (size_t)(strlen(aString) + 1), true);
}


bool
BMessage::ReplacString(const char *name, const char *aString)
{
    return ReplacString(name, 0, aString);
}


bool
BMessage::ReplacString(const char *name,  int32_t index, const BString &aString)
{
    return ReplacString(name, index, aString.String());
}


bool
BMessage::ReplacString(const char *name, const BString &aString)
{
    return ReplacString(name, 0, aString);
}


bool
BMessage::ReplaceInt8(const char *name,  int32_t index,  int8 val)
{
    if(!name) return false;
    return ReplaceData(name, B_INT8_TYPE, index, (const void*)&val, sizeof(int8), true);
}


bool
BMessage::ReplaceInt8(const char *name,  int8 val)
{
    return ReplaceInt8(name, 0, val);
}


bool
BMessage::ReplaceInt16(const char *name,  int32_t index,  int16 val)
{
    if(!name) return false;
    return ReplaceData(name, B_INT16_TYPE, index, (const void*)&val, sizeof(int16), true);
}


bool
BMessage::ReplaceInt16(const char *name,  int16 val)
{
    return ReplaceInt16(name, 0, val);
}


bool
BMessage::ReplaceInt32(const char *name,  int32_t index,  int32_t val)
{
    if(!name) return false;
    return ReplaceData(name, B_INT32_TYPE, index, (const void*)&val, sizeof(int32_t), true);
}


bool
BMessage::ReplaceInt32(const char *name,  int32_t val)
{
    return ReplaceInt32(name, 0, val);
}


bool
BMessage::ReplaceInt64(const char *name,  int32_t index,  int64 val)
{
    if(!name) return false;
    return ReplaceData(name, B_INT64_TYPE, index, (const void*)&val, sizeof(int64), true);
}


bool
BMessage::ReplaceInt64(const char *name,  int64 val)
{
    return ReplaceInt64(name, 0, val);
}


bool
BMessage::ReplaceBool(const char *name,  int32_t index, bool aBoolean)
{
    if(!name) return false;
    return ReplaceData(name, B_BOOL_TYPE, index, (const void*)&aBoolean, sizeof(bool), true);
}


bool
BMessage::ReplaceBool(const char *name, bool aBoolean)
{
    return ReplaceBool(name, 0, aBoolean);
}


bool
BMessage::ReplaceFloat(const char *name,  int32_t index, float f)
{
    if(!name) return false;
    return ReplaceData(name, B_FLOAT_TYPE, index, (const void*)&f, sizeof(float), true);
}


bool
BMessage::ReplaceFloat(const char *name, float f)
{
    return ReplaceFloat(name, 0, f);
}


bool
BMessage::ReplaceDouble(const char *name,  int32_t index, double d)
{
    if(!name) return false;
    return ReplaceData(name, B_DOUBLE_TYPE, index, (const void*)&d, sizeof(double), true);
}


bool
BMessage::ReplaceDouble(const char *name, double d)
{
    return ReplaceDouble(name, 0, d);
}


bool
BMessage::ReplacePoint(const char *name,  int32_t index, BPoint pt)
{
    if(!name) return false;

    struct point_t {
        float x;
        float y;
    } apt;

    apt.x = pt.x;
    apt.y = pt.y;

    return ReplaceData(name, B_POINT_TYPE, index, (const void*)&apt, sizeof(struct point_t), true);
}


bool
BMessage::ReplacePoint(const char *name, BPoint pt)
{
    return ReplacePoint(name, 0, pt);
}


bool
BMessage::ReplaceRect(const char *name,  int32_t index, BRect r)
{
    if(!name) return false;

    struct rect_t {
        float left;
        float top;
        float right;
        float bottom;
    } ar;

    ar.left = r.left;
    ar.top = r.top;
    ar.right = r.right;
    ar.bottom = r.bottom;

    return ReplaceData(name, B_RECT_TYPE, index, (const void*)&ar, sizeof(struct rect_t), true);
}


bool
BMessage::ReplaceRect(const char *name, BRect r)
{
    return ReplaceRect(name, 0, r);
}


bool
BMessage::ReplacePointer(const char *name,  int32_t index, const void *ptr)
{
    if(!name || !ptr) return false;
    return ReplaceData(name, B_POINTER_TYPE, index, ptr, 0, false);
}


bool
BMessage::ReplacePointer(const char *name, const void *ptr)
{
    return ReplacePointer(name, 0, ptr);
}


bool
BMessage::ReplaceMessage(const char *name,  int32_t index, const BMessage *msg)
{
    if(!name || !msg) return false;

    size_t flattenedSize = msg->FlattenedSize();
    if(flattenedSize <= 0) return false;

    char *buffer = (char*)malloc(flattenedSize);
    if(!buffer) return false;

    if(msg->Flatten(buffer, flattenedSize) == false)
    {
        free(buffer);
        return false;
    }

    bool retval = ReplaceData(name, B_MESSAGE_TYPE, index, buffer, flattenedSize, true);

    free(buffer);

    return retval;
}


bool
BMessage::ReplaceMessage(const char *name, const BMessage *msg)
{
    return ReplaceMessage(name, 0, msg);
}


bool
BMessage::ReplaceMessenger(const char *name,  int32_t index, const BMessenger *msgr)
{
    if(!name || !msgr) return false;

    size_t flattenedSize = msgr->FlattenedSize();
    if(flattenedSize <= 0) return false;

    char *buffer = (char*)malloc(flattenedSize + sizeof(size_t));
    if(!buffer) return false;

    memcpy(buffer, &flattenedSize, sizeof(size_t));
    if(msgr->Flatten(buffer + sizeof(size_t), flattenedSize) == false)
    {
        free(buffer);
        return false;
    }

    bool retval = ReplaceData(name, B_MESSENGER_TYPE, index, buffer, flattenedSize + sizeof(size_t), true);

    free(buffer);

    return retval;
}


bool
BMessage::ReplaceMessenger(const char *name, const BMessenger *msgr)
{
    return ReplaceMessenger(name, 0, msgr);
}


bool
BMessage::ReplaceMessenger(const char *name, const BMessenger &msgr)
{
    return ReplaceMessenger(name, 0, &msgr);
}


bool
BMessage::ReplaceMessenger(const char *name,  int32_t index, const BMessenger &msgr)
{
    return ReplaceMessenger(name, index, &msgr);
}


bool
BMessage::WasDelivered() const
{
    return(fReplyToken != B_MAXUINT64 || fSource != NULL);
}


bool
BMessage::IsReply() const
{
    return fIsReply;
}


bool
BMessage::IsSourceWaiting() const
{
    if(fSource == NULL) return false;
    void *tmpPort = bhapi::open_port_by_source((void*)fSource);
    if(tmpPort == NULL) return false;
    bhapi::delete_port((port_id)tmpPort);
    return true;
}


status_t
BMessage::SendReply(uint32_t command, BHandler *replyHandler) const
{
    BMessage msg(command);
    return SendReply(&msg, replyHandler, B_INFINITE_TIMEOUT);
}


status_t
BMessage::SendReply(const BMessage *message, BHandler *replyHandler, bigtime_t sendTimeout) const
{
    status_t retVal = B_BAD_VALUE;

    if(message != NULL)
    {
         uint64 replyToken = B_MAXUINT64;
        bigtime_t replyTokenTimeStamp = B_MAXINT64;

        if(!(replyHandler == NULL || replyHandler->fToken == NULL))
        {
            replyToken = replyHandler->fToken->Token();
            replyTokenTimeStamp = replyHandler->fToken->TimeStamp();
        }

        if(fSource != NULL)
        {
            BMessage msg(*message);

            msg.fIsReply = true;
            msg.fTargetToken = B_MAXUINT64;
            msg.fTargetTokenTimestamp = B_MAXINT64;
            msg.fReplyToken = replyToken;
            msg.fReplyTokenTimestamp = replyTokenTimeStamp;
            if(msg.fSource != NULL)
            {
                bhapi::delete_port(msg.fSource);
                msg.fSource = NULL;
            }

            retVal = (bhapi::port_count(fSource) > 0 ?
                    B_DUPLICATE_REPLY : BMessenger::_SendMessageToPort((void*)fSource, message, B_TIMEOUT, sendTimeout));
        }
        else
        {
            BMessenger msgr(fTeam, fReplyToken, fReplyTokenTimestamp, &retVal);
            if(retVal == B_OK)
            {
                BMessage msg(*message);

                msg.fIsReply = true;

                retVal = msgr._SendMessage(message, replyToken, sendTimeout);
            }
        }
    }

    return retVal;
}

/*
 * Copyright 2005-2015 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Lotz, mmlr@mlotz.ch
 */


#include <kits/netMessage.h>
#include <kits/netMessageAdapter.h>
#include <kits/netMessagePrivate.h>
#include <kits/netMessageUtils.h>

#include <kits/netDirectMessageTarget.h>
#include <kits/netMessengerPrivate.h>
#include <kits/netTokenSpace.h>
#include <kits/netutil/KMessage.h>

#include <kits/netAlignment.h>
#include <kits/netApplication.h>
#include <kits/netAppMisc.h>
#include <kits/netBlockCache.h>
#include <kits/storage/Entry.h>
#include <kits/netMessageQueue.h>
#include <kits/app/Messenger.h>
#include <kits/netPath.h>
#include <kits/interface/Point.h>
#include <kits/interface/Rect.h>
#include <kits/netString.h>
#include <kits/netStringList.h>

#include <kits/netassert.h>
#include <kits/netctype.h>
#include <kits/netstdio.h>
#include <kits/netstdlib.h>
#include <kits/netstring.h>

//#include <kits/nettracing_config.h>
    // kernel tracing configuration

//#define VERBOSE_DEBUG_OUTPUT
#ifdef VERBOSE_DEBUG_OUTPUT
#define DEBUG_FUNCTION_ENTER	\
    debug_printf("msg thread: %ld; this: %p; header: %p; fields: %p;" \
        " data: %p; what: 0x%08lx '%.4s'; line: %d; func: %s\n", \
        find_thread(NULL), this, fHeader, fFields, fData, what, (char*)&what, \
        __LINE__, __PRETTY_FUNCTION__);

#define DEBUG_FUNCTION_ENTER2	\
    debug_printf("msg thread: %ld; line: %d: func: %s\n", find_thread(NULL), \
        __LINE__, __PRETTY_FUNCTION__);
#else
#define DEBUG_FUNCTION_ENTER	/* nothing */
#define DEBUG_FUNCTION_ENTER2	/* nothing */
#endif

#if BMESSAGE_TRACING
#	define KTRACE(format...)	ktrace_printf(format)
#else
#	define KTRACE(format...)	;
#endif


const char* B_SPECIFIER_ENTRY = "specifiers";
const char* B_PROPERTY_ENTRY = "property";
const char* B_PROPERTY_NAME_ENTRY = "name";


static status_t handle_reply(port_id replyPort, int32_t* pCode,
    bigtime_t timeout, BMessage* reply);


extern "C" {
    // private os function to set the owning team of an area
    status_t _kern_transfer_area(area_id area, void** _address,
        uint32_t addressSpec, team_id target);
}


BBlockCache* BMessage::sMsgCache = NULL;
port_id BMessage::sReplyPorts[sNumReplyPorts];
int32_t BMessage::sReplyPortInUse[sNumReplyPorts];


template<typename Type>
static void
print_to_stream_type(uint8* pointer)
{
    Type* item = (Type*)pointer;
    item->PrintToStream();
}


template<typename Type>
static void
print_type(const char* format, uint8* pointer)
{
    Type* item = (Type*)pointer;
    printf(format,* item,* item);
}


template<typename Type>
static void
print_type3(const char* format, uint8* pointer)
{
    Type* item = (Type*)pointer;
    printf(format, *item, *item, *item);
}


static status_t
handle_reply(port_id replyPort, int32_t* _code, bigtime_t timeout,
    BMessage* reply)
{
    DEBUG_FUNCTION_ENTER2;
    ssize_t size;
    do {
        size = port_buffer_size_etc(replyPort, B_RELATIVE_TIMEOUT, timeout);
    } while (size == B_INTERRUPTED);

    if (size < 0)
        return size;

    status_t result;
    char* buffer = (char*)malloc(size);
    if (buffer == NULL)
        return B_NO_MEMORY;

    do {
        result = read_port(replyPort, _code, buffer, size);
    } while (result == B_INTERRUPTED);

    if (result < 0 || *_code != kPortMessageCode) {
        free(buffer);
        return result < 0 ? result : B_ERROR;
    }

    result = reply->Unflatten(buffer);
    free(buffer);
    return result;
}


//	#pragma mark -


BMessage::BMessage()
{
    DEBUG_FUNCTION_ENTER;
    _InitCommon(true);
}


BMessage::BMessage(BMessage* other)
{
    DEBUG_FUNCTION_ENTER;
    _InitCommon(false);
    *this = *other;
}


BMessage::BMessage(uint32_t _what)
{
    DEBUG_FUNCTION_ENTER;
    _InitCommon(true);
    fHeader->what = what = _what;
}


BMessage::BMessage(const BMessage& other)
{
    DEBUG_FUNCTION_ENTER;
    _InitCommon(false);
    *this = other;
}


BMessage::~BMessage()
{
    DEBUG_FUNCTION_ENTER;
    _Clear();
}


BMessage&
BMessage::operator=(const BMessage& other)
{
    DEBUG_FUNCTION_ENTER;

    if (this == &other)
        return *this;

    _Clear();

    fHeader = (message_header*)malloc(sizeof(message_header));
    if (fHeader == NULL)
        return *this;

    if (other.fHeader == NULL)
        return *this;

    memcpy(fHeader, other.fHeader, sizeof(message_header));

    // Clear some header flags inherited from the original message that don't
    // apply to the clone.
    fHeader->flags &= ~(MESSAGE_FLAG_REPLY_REQUIRED | MESSAGE_FLAG_REPLY_DONE
        | MESSAGE_FLAG_IS_REPLY | MESSAGE_FLAG_WAS_DELIVERED
        | MESSAGE_FLAG_PASS_BY_AREA);
    // Note, that BeOS R5 seems to keep the reply info.

    if (fHeader->field_count > 0) {
        size_t fieldsSize = fHeader->field_count * sizeof(field_header);
        if (other.fFields != NULL)
            fFields = (field_header*)malloc(fieldsSize);

        if (fFields == NULL) {
            fHeader->field_count = 0;
            fHeader->data_size = 0;
        } else if (other.fFields != NULL)
            memcpy(fFields, other.fFields, fieldsSize);
    }

    if (fHeader->data_size > 0) {
        if (other.fData != NULL)
            fData = (uint8*)malloc(fHeader->data_size);

        if (fData == NULL) {
            fHeader->field_count = 0;
            free(fFields);
            fFields = NULL;
        } else if (other.fData != NULL)
            memcpy(fData, other.fData, fHeader->data_size);
    }

    fHeader->what = what = other.what;
    fHeader->message_area = -1;
    fFieldsAvailable = 0;
    fDataAvailable = 0;

    return *this;
}


void*
BMessage::operator new(size_t size)
{
    DEBUG_FUNCTION_ENTER2;
    return sMsgCache->Get(size);
}


void*
BMessage::operator new(size_t size, const std::nothrow_t& noThrow)
{
    DEBUG_FUNCTION_ENTER2;
    return sMsgCache->Get(size);
}


void*
BMessage::operator new(size_t, void* pointer)
{
    DEBUG_FUNCTION_ENTER2;
    return pointer;
}


void
BMessage::operator delete(void* pointer, size_t size)
{
    DEBUG_FUNCTION_ENTER2;
    if (pointer == NULL)
        return;
    sMsgCache->Save(pointer, size);
}


bool
BMessage::HasSameData(const BMessage& other, bool ignoreFieldOrder,
    bool deep) const
{
    if (this == &other)
        return true;

    if (fHeader == NULL)
        return other.fHeader == NULL;

    if (fHeader->field_count != other.fHeader->field_count)
        return false;

    for (uint32_t i = 0; i < fHeader->field_count; i++) {
        field_header* field = &fFields[i];
        field_header* otherField = NULL;

        const char* name = (const char*)fData + field->offset;
        if (ignoreFieldOrder) {
            if (other._FindField(name, B_ANY_TYPE, &otherField) != B_OK)
                return false;
        } else {
            otherField = &other.fFields[i];
            if (otherField->name_length != field->name_length)
                return false;

            const char* otherName = (const char*)other.fData
                + otherField->offset;
            if (strncmp(name, otherName, field->name_length) != 0)
                return false;
        }

        if (otherField->type != field->type
            || otherField->count != field->count) {
            return false;
        }

        uint8* data = fData + field->offset + field->name_length;
        uint8* otherData = other.fData + otherField->offset
            + otherField->name_length;

        bool needsMemCompare = true;
        if (deep && field->type == B_MESSAGE_TYPE) {
            BMessage message, otherMessage;
            if (message.Unflatten((const char*)data) == B_OK
                && otherMessage.Unflatten((const char*)otherData) == B_OK) {
                if (!message.HasSameData(ignoreFieldOrder, deep))
                    return false;
                needsMemCompare = false;
            }
        }

        if (needsMemCompare) {
            if (otherField->data_size != field->data_size)
                return false;
            if (memcmp(data, otherData, field->data_size) != 0)
                return false;
        }
    }

    return true;
}


status_t
BMessage::_InitCommon(bool initHeader)
{
    DEBUG_FUNCTION_ENTER;
    what = 0;

    fHeader = NULL;
    fFields = NULL;
    fData = NULL;

    fFieldsAvailable = 0;
    fDataAvailable = 0;

    fOriginal = NULL;
    fQueueLink = NULL;

    fArchivingPointer = NULL;

    if (initHeader)
        return _InitHeader();

    return B_OK;
}


status_t
BMessage::_InitHeader()
{
    DEBUG_FUNCTION_ENTER;
    if (fHeader == NULL) {
        fHeader = (message_header*)malloc(sizeof(message_header));
        if (fHeader == NULL)
            return B_NO_MEMORY;
    }

    memset(fHeader, 0, sizeof(message_header) - sizeof(fHeader->hash_table));

    fHeader->format = MESSAGE_FORMAT_HAIKU;
    fHeader->flags = MESSAGE_FLAG_VALID;
    fHeader->what = what;
    fHeader->current_specifier = -1;
    fHeader->message_area = -1;

    fHeader->target = B_NULL_TOKEN;
    fHeader->reply_target = B_NULL_TOKEN;
    fHeader->reply_port = -1;
    fHeader->reply_team = -1;

    // initializing the hash table to -1 because 0 is a valid index
    fHeader->hash_table_size = MESSAGE_BODY_HASH_TABLE_SIZE;
    memset(&fHeader->hash_table, 255, sizeof(fHeader->hash_table));
    return B_OK;
}


status_t
BMessage::_Clear()
{
    DEBUG_FUNCTION_ENTER;
    if (fHeader != NULL) {
        // We're going to destroy all information of this message. If there's
        // still someone waiting for a reply to this message, we have to send
        // one now.
        if (IsSourceWaiting())
            SendReply(B_NO_REPLY);

        if (fHeader->message_area >= 0)
            _Dereference();

        free(fHeader);
        fHeader = NULL;
    }

    free(fFields);
    fFields = NULL;
    free(fData);
    fData = NULL;

    fArchivingPointer = NULL;

    fFieldsAvailable = 0;
    fDataAvailable = 0;

    delete fOriginal;
    fOriginal = NULL;

    return B_OK;
}


status_t
BMessage::GetInfo(type_code typeRequested, int32_t index, char** nameFound,
    type_code* typeFound, int32_t* countFound) const
{
    DEBUG_FUNCTION_ENTER;
    if (fHeader == NULL)
        return B_NO_INIT;

    if (index < 0 || (uint32_t)index >= fHeader->field_count)
        return B_BAD_INDEX;

    if (typeRequested == B_ANY_TYPE) {
        if (nameFound != NULL)
            *nameFound = (char*)fData + fFields[index].offset;
        if (typeFound != NULL)
            *typeFound = fFields[index].type;
        if (countFound != NULL)
            *countFound = fFields[index].count;
        return B_OK;
    }

    int32_t counter = -1;
    field_header* field = fFields;
    for (uint32_t i = 0; i < fHeader->field_count; i++, field++) {
        if (field->type == typeRequested)
            counter++;

        if (counter == index) {
            if (nameFound != NULL)
                *nameFound = (char*)fData + field->offset;
            if (typeFound != NULL)
                *typeFound = field->type;
            if (countFound != NULL)
                *countFound = field->count;
            return B_OK;
        }
    }

    if (counter == -1)
        return B_BAD_TYPE;

    return B_BAD_INDEX;
}


status_t
BMessage::GetInfo(const char* name, type_code* typeFound,
    int32_t* countFound) const
{
    DEBUG_FUNCTION_ENTER;
    if (countFound != NULL)
        *countFound = 0;

    field_header* field = NULL;
    status_t result = _FindField(name, B_ANY_TYPE, &field);
    if (result != B_OK)
        return result;

    if (typeFound != NULL)
        *typeFound = field->type;
    if (countFound != NULL)
        *countFound = field->count;

    return B_OK;
}


status_t
BMessage::GetInfo(const char* name, type_code* typeFound, bool* fixedSize)
    const
{
    DEBUG_FUNCTION_ENTER;
    field_header* field = NULL;
    status_t result = _FindField(name, B_ANY_TYPE, &field);
    if (result != B_OK)
        return result;

    if (typeFound != NULL)
        *typeFound = field->type;
    if (fixedSize != NULL)
        *fixedSize = (field->flags & FIELD_FLAG_FIXED_SIZE) != 0;

    return B_OK;
}


status_t
BMessage::GetInfo(const char* name, type_code* typeFound, int32_t* countFound,
    bool* fixedSize) const
{
    DEBUG_FUNCTION_ENTER;
    field_header* field = NULL;
    status_t result = _FindField(name, B_ANY_TYPE, &field);
    if (result != B_OK)
        return result;

    if (typeFound != NULL)
        *typeFound = field->type;
    if (countFound != NULL)
        *countFound = field->count;
    if (fixedSize != NULL)
        *fixedSize = (field->flags & FIELD_FLAG_FIXED_SIZE) != 0;

    return B_OK;
}


int32_t
BMessage::CountNames(type_code type) const
{
    DEBUG_FUNCTION_ENTER;
    if (fHeader == NULL)
        return 0;

    if (type == B_ANY_TYPE)
        return fHeader->field_count;

    int32_t count = 0;
    field_header* field = fFields;
    for (uint32_t i = 0; i < fHeader->field_count; i++, field++) {
        if (field->type == type)
            count++;
    }

    return count;
}


bool
BMessage::IsEmpty() const
{
    DEBUG_FUNCTION_ENTER;
    return fHeader == NULL || fHeader->field_count == 0;
}


bool
BMessage::IsSystem() const
{
    DEBUG_FUNCTION_ENTER;
    char a = char(what >> 24);
    char b = char(what >> 16);
    char c = char(what >> 8);
    char d = char(what);

    // The BeBook says:
    //		... we've adopted a strict convention for assigning values to all
    //		Be-defined constants.  The value assigned will always be formed by
    //		combining four characters into a multicharacter constant, with the
    //		characters limited to uppercase letters and the underbar
    // Between that and what's in AppDefs.h, this algo seems like a safe bet:
    if (a == '_' && isupper(b) && isupper(c) && isupper(d))
        return true;

    return false;
}


bool
BMessage::IsReply() const
{
    DEBUG_FUNCTION_ENTER;
    return fHeader != NULL && (fHeader->flags & MESSAGE_FLAG_IS_REPLY) != 0;
}


void
BMessage::PrintToStream() const
{
    _PrintToStream("");
    printf("}\n");
}


void
BMessage::_PrintToStream(const char* indent) const
{
    DEBUG_FUNCTION_ENTER;

    int32_t value = B_BENDIAN_TO_HOST_INT32(what);
    printf("BMessage(");
    if (isprint(*(char*)&value))
        printf("'%.4s'", (char*)&value);
    else
        printf("0x%" B_PRIx32, what);
    printf(") {\n");

    if (fHeader == NULL || fFields == NULL || fData == NULL)
        return;

    field_header* field = fFields;
    for (uint32_t i = 0; i < fHeader->field_count; i++, field++) {
        value = B_BENDIAN_TO_HOST_INT32(field->type);
        ssize_t size = 0;
        if ((field->flags & FIELD_FLAG_FIXED_SIZE) != 0 && field->count > 0)
            size = field->data_size / field->count;

        uint8* pointer = fData + field->offset + field->name_length;
        for (uint32_t j = 0; j < field->count; j++) {
            if (field->count == 1) {
                printf("%s        %s = ", indent,
                    (char*)(fData + field->offset));
            } else {
                printf("%s        %s[%" B_PRIu32 "] = ", indent,
                    (char*)(fData + field->offset), j);
            }

            if ((field->flags & FIELD_FLAG_FIXED_SIZE) == 0) {
                size = *(uint32_t*)pointer;
                pointer += sizeof(uint32_t);
            }

            switch (field->type) {
                case B_RECT_TYPE:
                    print_to_stream_type<BRect>(pointer);
                    break;

                case B_POINT_TYPE:
                    print_to_stream_type<BPoint>(pointer);
                    break;

                case B_STRING_TYPE:
                    printf("string(\"%.*s\", %ld bytes)\n", (int)size,
                        (char*)pointer, (long)size);
                    break;

                case B_INT8_TYPE:
                    print_type3<int8>("int8(0x%hx or %d or '%c')\n",
                        pointer);
                    break;

                case B_UINT8_TYPE:
                    print_type3<uint8>("uint8(0x%hx or %u or '%c')\n",
                        pointer);
                    break;

                case B_INT16_TYPE:
                    print_type<int16>("int16(0x%x or %d)\n", pointer);
                    break;

                case B_UINT16_TYPE:
                    print_type<uint16>("uint16(0x%x or %u\n", pointer);
                    break;

                case B_INT32_TYPE:
                    print_type<int32_t>("int32_t(0x%lx or %ld)\n", pointer);
                    break;

                case B_UINT32_TYPE:
                    print_type<uint32_t>("uint32_t(0x%lx or %lu\n", pointer);
                    break;

                case B_INT64_TYPE:
                    print_type<int64>("int64(0x%Lx or %Ld)\n", pointer);
                    break;

                case B_UINT64_TYPE:
                    print_type<uint64>("uint64(0x%Lx or %Ld\n", pointer);
                    break;

                case B_BOOL_TYPE:
                    printf("bool(%s)\n", *((bool*)pointer) != 0
                        ? "true" : "false");
                    break;

                case B_FLOAT_TYPE:
                    print_type<float>("float(%.4f)\n", pointer);
                    break;

                case B_DOUBLE_TYPE:
                    print_type<double>("double(%.8f)\n", pointer);
                    break;

                case B_REF_TYPE:
                {
                    entry_ref ref;
                    BPrivate::entry_ref_unflatten(&ref, (char*)pointer, size);

                    printf("entry_ref(device=%d, directory=%" B_PRIdINO
                        ", name=\"%s\", ", (int)ref.device, ref.directory,
                        ref.name);

                    BPath path(&ref);
                    printf("path=\"%s\")\n", path.Path());
                    break;
                }

                case B_MESSAGE_TYPE:
                {
                    char buffer[1024];
                    snprintf(buffer, sizeof(buffer), "%s        ", indent);

                    BMessage message;
                    status_t result = message.Unflatten((const char*)pointer);
                    if (result != B_OK) {
                        printf("failed unflatten: %s\n", strerror(result));
                        break;
                    }

                    message._PrintToStream(buffer);
                    printf("%s        }\n", indent);
                    break;
                }

                default:
                {
                    printf("(type = '%.4s')(size = %ld)\n", (char*)&value,
                        (long)size);
                    break;
                }
            }

            pointer += size;
        }
    }
}


status_t
BMessage::Rename(const char* oldEntry, const char* newEntry)
{
    DEBUG_FUNCTION_ENTER;
    if (oldEntry == NULL || newEntry == NULL)
        return B_BAD_VALUE;

    if (fHeader == NULL)
        return B_NO_INIT;

    status_t result;
    if (fHeader->message_area >= 0) {
        result = _CopyForWrite();
        if (result != B_OK)
            return result;
    }

    uint32_t hash = _HashName(oldEntry) % fHeader->hash_table_size;
    int32_t* nextField = &fHeader->hash_table[hash];

    while (*nextField >= 0) {
        field_header* field = &fFields[*nextField];

        if (strncmp((const char*)(fData + field->offset), oldEntry,
            field->name_length) == 0) {
            // nextField points to the field for oldEntry, save it and unlink
            int32_t index = *nextField;
            *nextField = field->next_field;
            field->next_field = -1;

            hash = _HashName(newEntry) % fHeader->hash_table_size;
            nextField = &fHeader->hash_table[hash];
            while (*nextField >= 0)
                nextField = &fFields[*nextField].next_field;
            *nextField = index;

            int32_t newLength = strlen(newEntry) + 1;
            result = _ResizeData(field->offset + 1,
                newLength - field->name_length);
            if (result != B_OK)
                return result;

            memcpy(fData + field->offset, newEntry, newLength);
            field->name_length = newLength;
            return B_OK;
        }

        nextField = &field->next_field;
    }

    return B_NAME_NOT_FOUND;
}


bool
BMessage::WasDelivered() const
{
    DEBUG_FUNCTION_ENTER;
    return fHeader != NULL
        && (fHeader->flags & MESSAGE_FLAG_WAS_DELIVERED) != 0;
}


bool
BMessage::IsSourceWaiting() const
{
    DEBUG_FUNCTION_ENTER;
    return fHeader != NULL
        && (fHeader->flags & MESSAGE_FLAG_REPLY_REQUIRED) != 0
        && (fHeader->flags & MESSAGE_FLAG_REPLY_DONE) == 0;
}


bool
BMessage::IsSourceRemote() const
{
    DEBUG_FUNCTION_ENTER;
    return fHeader != NULL
        && (fHeader->flags & MESSAGE_FLAG_WAS_DELIVERED) != 0
        && fHeader->reply_team != BPrivate::current_team();
}


BMessenger
BMessage::ReturnAddress() const
{
    DEBUG_FUNCTION_ENTER;
    if (fHeader == NULL || (fHeader->flags & MESSAGE_FLAG_WAS_DELIVERED) == 0)
        return BMessenger();

    BMessenger messenger;
    BMessenger::Private(messenger).SetTo(fHeader->reply_team,
        fHeader->reply_port, fHeader->reply_target);
    return messenger;
}


const BMessage*
BMessage::Previous() const
{
    DEBUG_FUNCTION_ENTER;
    /* ToDo: test if the "_previous_" field is used in R5 */
    if (fOriginal == NULL) {
        fOriginal = new BMessage();

        if (FindMessage("_previous_", fOriginal) != B_OK) {
            delete fOriginal;
            fOriginal = NULL;
        }
    }

    return fOriginal;
}


bool
BMessage::WasDropped() const
{
    DEBUG_FUNCTION_ENTER;
    return fHeader != NULL
        && (fHeader->flags & MESSAGE_FLAG_WAS_DROPPED) != 0;
}


BPoint
BMessage::DropPoint(BPoint* offset) const
{
    DEBUG_FUNCTION_ENTER;
    if (offset != NULL)
        *offset = FindPoint("_drop_offset_");

    return FindPoint("_drop_point_");
}


status_t
BMessage::SendReply(uint32_t command, BHandler* replyTo)
{
    DEBUG_FUNCTION_ENTER;
    BMessage message(command);
    return SendReply(&message, replyTo);
}


status_t
BMessage::SendReply(BMessage* reply, BHandler* replyTo, bigtime_t timeout)
{
    DEBUG_FUNCTION_ENTER;
    BMessenger messenger(replyTo);
    return SendReply(reply, messenger, timeout);
}


status_t
BMessage::SendReply(BMessage* reply, BMessenger replyTo, bigtime_t timeout)
{
    DEBUG_FUNCTION_ENTER;
    if (fHeader == NULL)
        return B_NO_INIT;

    BMessenger messenger;
    BMessenger::Private messengerPrivate(messenger);
    messengerPrivate.SetTo(fHeader->reply_team, fHeader->reply_port,
        fHeader->reply_target);
    if ((fHeader->flags & MESSAGE_FLAG_REPLY_AS_KMESSAGE) != 0)
        reply->fHeader->flags |= MESSAGE_FLAG_REPLY_AS_KMESSAGE;

    if ((fHeader->flags & MESSAGE_FLAG_REPLY_REQUIRED) != 0) {
        if ((fHeader->flags & MESSAGE_FLAG_REPLY_DONE) != 0)
            return B_DUPLICATE_REPLY;

        fHeader->flags |= MESSAGE_FLAG_REPLY_DONE;
        reply->fHeader->flags |= MESSAGE_FLAG_IS_REPLY;
        status_t result = messenger.SendMessage(reply, replyTo, timeout);
        reply->fHeader->flags &= ~MESSAGE_FLAG_IS_REPLY;

        if (result != B_OK && set_port_owner(messengerPrivate.Port(),
                messengerPrivate.Team()) == B_BAD_TEAM_ID) {
            delete_port(messengerPrivate.Port());
        }

        return result;
    }

    // no reply required
    if ((fHeader->flags & MESSAGE_FLAG_WAS_DELIVERED) == 0)
        return B_BAD_REPLY;

    reply->AddMessage("_previous_", this);
    reply->fHeader->flags |= MESSAGE_FLAG_IS_REPLY;
    status_t result = messenger.SendMessage(reply, replyTo, timeout);
    reply->fHeader->flags &= ~MESSAGE_FLAG_IS_REPLY;
    reply->RemoveName("_previous_");
    return result;
}


status_t
BMessage::SendReply(uint32_t command, BMessage* replyToReply)
{
    DEBUG_FUNCTION_ENTER;
    BMessage message(command);
    return SendReply(&message, replyToReply);
}


status_t
BMessage::SendReply(BMessage* reply, BMessage* replyToReply,
    bigtime_t sendTimeout, bigtime_t replyTimeout)
{
    DEBUG_FUNCTION_ENTER;
    if (fHeader == NULL)
        return B_NO_INIT;

    BMessenger messenger;
    BMessenger::Private messengerPrivate(messenger);
    messengerPrivate.SetTo(fHeader->reply_team, fHeader->reply_port,
        fHeader->reply_target);

    if ((fHeader->flags & MESSAGE_FLAG_REPLY_REQUIRED) != 0) {
        if ((fHeader->flags & MESSAGE_FLAG_REPLY_DONE) != 0)
            return B_DUPLICATE_REPLY;

        fHeader->flags |= MESSAGE_FLAG_REPLY_DONE;
        reply->fHeader->flags |= MESSAGE_FLAG_IS_REPLY;
        status_t result = messenger.SendMessage(reply, replyToReply,
            sendTimeout, replyTimeout);
        reply->fHeader->flags &= ~MESSAGE_FLAG_IS_REPLY;

        if (result != B_OK) {
            if (set_port_owner(messengerPrivate.Port(),
                messengerPrivate.Team()) == B_BAD_TEAM_ID) {
                delete_port(messengerPrivate.Port());
            }
        }

        return result;
    }

    // no reply required
    if ((fHeader->flags & MESSAGE_FLAG_WAS_DELIVERED) == 0)
        return B_BAD_REPLY;

    reply->AddMessage("_previous_", this);
    reply->fHeader->flags |= MESSAGE_FLAG_IS_REPLY
        | (fHeader->flags & MESSAGE_FLAG_REPLY_AS_KMESSAGE);
    status_t result = messenger.SendMessage(reply, replyToReply, sendTimeout,
        replyTimeout);
    reply->fHeader->flags &= ~MESSAGE_FLAG_IS_REPLY;
    reply->RemoveName("_previous_");
    return result;
}


ssize_t
BMessage::FlattenedSize() const
{
    DEBUG_FUNCTION_ENTER;
    if (fHeader == NULL)
        return B_NO_INIT;

    return sizeof(message_header) + fHeader->field_count * sizeof(field_header)
        + fHeader->data_size;
}


status_t
BMessage::Flatten(char* buffer, ssize_t size) const
{
    DEBUG_FUNCTION_ENTER;
    if (buffer == NULL || size < 0)
        return B_BAD_VALUE;

    if (fHeader == NULL)
        return B_NO_INIT;

    if (size < FlattenedSize())
        return B_BUFFER_OVERFLOW;

    /* we have to sync the what code as it is a public member */
    fHeader->what = what;

    memcpy(buffer, fHeader, sizeof(message_header));
    buffer += sizeof(message_header);

    size_t fieldsSize = fHeader->field_count * sizeof(field_header);
    memcpy(buffer, fFields, fieldsSize);
    buffer += fieldsSize;

    memcpy(buffer, fData, fHeader->data_size);

    return B_OK;
}


status_t
BMessage::Flatten(BDataIO* stream, ssize_t* size) const
{
    DEBUG_FUNCTION_ENTER;
    if (stream == NULL)
        return B_BAD_VALUE;

    if (fHeader == NULL)
        return B_NO_INIT;

    /* we have to sync the what code as it is a public member */
    fHeader->what = what;

    ssize_t result1 = stream->Write(fHeader, sizeof(message_header));
    if (result1 != sizeof(message_header))
        return result1 < 0 ? result1 : B_ERROR;

    ssize_t result2 = 0;
    if (fHeader->field_count > 0) {
        ssize_t fieldsSize = fHeader->field_count * sizeof(field_header);
        result2 = stream->Write(fFields, fieldsSize);
        if (result2 != fieldsSize)
            return result2 < 0 ? result2 : B_ERROR;
    }

    ssize_t result3 = 0;
    if (fHeader->data_size > 0) {
        result3 = stream->Write(fData, fHeader->data_size);
        if (result3 != (ssize_t)fHeader->data_size)
            return result3 < 0 ? result3 : B_ERROR;
    }

    if (size)
        *size = result1 + result2 + result3;

    return B_OK;
}


/*	The concept of message sending by area:

    The traditional way of sending a message is to send it by flattening it to
    a buffer, pushing it through a port, reading it into the outputbuffer and
    unflattening it from there (copying the data again). While this works ok
    for small messages it does not make any sense for larger ones and may even
    hit some port capacity limit.
    Often in the life of a BMessage, it will be sent to someone. Almost as
    often the one receiving the message will not need to change the message
    in any way, but uses it "read only" to get information from it. This means
    that all that copying is pretty pointless in the first place since we
    could simply pass the original buffers on.
    It's obviously not exactly as simple as this, since we cannot just use the
    memory of one application in another - but we can share areas with
    eachother.
    Therefore instead of flattening into a buffer, we copy the message data
    into an area, put this information into the message header and only push
    this through the port. The receiving looper then builds a BMessage from
    the header, that only references the data in the area (not copying it),
    allowing read only access to it.
    Only if write access is necessary the message will be copyed from the area
    to its own buffers (like in the unflatten step before).
    The double copying is reduced to a single copy in most cases and we safe
    the slower route of moving the data through a port.
    Additionally we save us the reference counting with the use of areas that
    are reference counted internally. So we don't have to worry about leaving
    an area behind or deleting one that is still in use.
*/

status_t
BMessage::_FlattenToArea(message_header** _header) const
{
    DEBUG_FUNCTION_ENTER;
    if (fHeader == NULL)
        return B_NO_INIT;

    message_header* header = (message_header*)malloc(sizeof(message_header));
    if (header == NULL)
        return B_NO_MEMORY;

    memcpy(header, fHeader, sizeof(message_header));

    header->what = what;
    header->message_area = -1;
    *_header = header;

    if (header->field_count == 0 && header->data_size == 0)
        return B_OK;

    char* address = NULL;
    size_t fieldsSize = header->field_count * sizeof(field_header);
    size_t size = fieldsSize + header->data_size;
    size = (size + B_PAGE_SIZE) & ~(B_PAGE_SIZE - 1);
    area_id area = create_area("BMessage data", (void**)&address,
        B_ANY_ADDRESS, size, B_NO_LOCK, B_READ_AREA | B_WRITE_AREA);

    if (area < 0) {
        free(header);
        *_header = NULL;
        return area;
    }

    memcpy(address, fFields, fieldsSize);
    memcpy(address + fieldsSize, fData, fHeader->data_size);
    header->flags |= MESSAGE_FLAG_PASS_BY_AREA;
    header->message_area = area;
    return B_OK;
}


status_t
BMessage::_Reference()
{
    DEBUG_FUNCTION_ENTER;
    if (fHeader == NULL)
        return B_NO_INIT;

    fHeader->flags &= ~MESSAGE_FLAG_PASS_BY_AREA;

    /* if there is no data at all we don't need the area */
    if (fHeader->field_count == 0 && fHeader->data_size == 0)
        return B_OK;

    area_info areaInfo;
    status_t result = get_area_info(fHeader->message_area, &areaInfo);
    if (result != B_OK)
        return result;

    if (areaInfo.team != BPrivate::current_team())
        return B_BAD_VALUE;

    set_area_protection(fHeader->message_area, B_READ_AREA);

    uint8* address = (uint8*)areaInfo.address;

    fFields = (field_header*)address;
    fData = address + fHeader->field_count * sizeof(field_header);
    return B_OK;
}


status_t
BMessage::_Dereference()
{
    DEBUG_FUNCTION_ENTER;
    if (fHeader == NULL)
        return B_NO_INIT;

    delete_area(fHeader->message_area);
    fHeader->message_area = -1;
    fFields = NULL;
    fData = NULL;
    return B_OK;
}


status_t
BMessage::_CopyForWrite()
{
    DEBUG_FUNCTION_ENTER;
    if (fHeader == NULL)
        return B_NO_INIT;

    field_header* newFields = NULL;
    uint8* newData = NULL;

    if (fHeader->field_count > 0) {
        size_t fieldsSize = fHeader->field_count * sizeof(field_header);
        newFields = (field_header*)malloc(fieldsSize);
        if (newFields == NULL)
            return B_NO_MEMORY;

        memcpy(newFields, fFields, fieldsSize);
    }

    if (fHeader->data_size > 0) {
        newData = (uint8*)malloc(fHeader->data_size);
        if (newData == NULL) {
            free(newFields);
            return B_NO_MEMORY;
        }

        memcpy(newData, fData, fHeader->data_size);
    }

    _Dereference();

    fFieldsAvailable = 0;
    fDataAvailable = 0;

    fFields = newFields;
    fData = newData;
    return B_OK;
}


status_t
BMessage::_ValidateMessage()
{
    DEBUG_FUNCTION_ENTER;
    if (fHeader == NULL)
        return B_NO_INIT;

    if (fHeader->field_count == 0)
        return B_OK;

    if (fFields == NULL)
        return B_NO_INIT;

    for (uint32_t i = 0; i < fHeader->field_count; i++) {
        field_header* field = &fFields[i];
        if ((field->next_field >= 0
                && (uint32_t)field->next_field > fHeader->field_count)
            || (field->offset + field->name_length + field->data_size
                > fHeader->data_size)) {
            // the message is corrupt
            MakeEmpty();
            return B_BAD_VALUE;
        }
    }

    return B_OK;
}


status_t
BMessage::Unflatten(const char* flatBuffer)
{
    DEBUG_FUNCTION_ENTER;
    if (flatBuffer == NULL)
        return B_BAD_VALUE;

    uint32_t format = *(uint32_t*)flatBuffer;
    if (format != MESSAGE_FORMAT_HAIKU)
        return BPrivate::MessageAdapter::Unflatten(format, this, flatBuffer);

    BMemoryIO io(flatBuffer, SSIZE_MAX);
    return Unflatten(&io);
}


status_t
BMessage::Unflatten(BDataIO* stream)
{
    DEBUG_FUNCTION_ENTER;
    if (stream == NULL)
        return B_BAD_VALUE;

    uint32_t format = 0;
    stream->Read(&format, sizeof(uint32_t));
    if (format != MESSAGE_FORMAT_HAIKU)
        return BPrivate::MessageAdapter::Unflatten(format, this, stream);

    // native message unflattening

    _Clear();

    fHeader = (message_header*)malloc(sizeof(message_header));
    if (fHeader == NULL)
        return B_NO_MEMORY;

    fHeader->format = format;
    uint8* header = (uint8*)fHeader;
    ssize_t result = stream->Read(header + sizeof(uint32_t),
        sizeof(message_header) - sizeof(uint32_t));
    if (result != sizeof(message_header) - sizeof(uint32_t)
        || (fHeader->flags & MESSAGE_FLAG_VALID) == 0) {
        _InitHeader();
        return result < 0 ? result : B_BAD_VALUE;
    }

    what = fHeader->what;

    if ((fHeader->flags & MESSAGE_FLAG_PASS_BY_AREA) != 0
        && fHeader->message_area >= 0) {
        status_t result = _Reference();
        if (result != B_OK) {
            _InitHeader();
            return result;
        }
    } else {
        fHeader->message_area = -1;

        if (fHeader->field_count > 0) {
            ssize_t fieldsSize = fHeader->field_count * sizeof(field_header);
            fFields = (field_header*)malloc(fieldsSize);
            if (fFields == NULL) {
                _InitHeader();
                return B_NO_MEMORY;
            }

            result = stream->Read(fFields, fieldsSize);
            if (result != fieldsSize)
                return result < 0 ? result : B_BAD_VALUE;
        }

        if (fHeader->data_size > 0) {
            fData = (uint8*)malloc(fHeader->data_size);
            if (fData == NULL) {
                free(fFields);
                fFields = NULL;
                _InitHeader();
                return B_NO_MEMORY;
            }

            result = stream->Read(fData, fHeader->data_size);
            if (result != (ssize_t)fHeader->data_size)
                return result < 0 ? result : B_BAD_VALUE;
        }
    }

    return _ValidateMessage();
}


status_t
BMessage::AddSpecifier(const char* property)
{
    DEBUG_FUNCTION_ENTER;
    BMessage message(B_DIRECT_SPECIFIER);
    status_t result = message.AddString(B_PROPERTY_ENTRY, property);
    if (result != B_OK)
        return result;

    return AddSpecifier(&message);
}


status_t
BMessage::AddSpecifier(const char* property, int32_t index)
{
    DEBUG_FUNCTION_ENTER;
    BMessage message(B_INDEX_SPECIFIER);
    status_t result = message.AddString(B_PROPERTY_ENTRY, property);
    if (result != B_OK)
        return result;

    result = message.AddInt32("index", index);
    if (result != B_OK)
        return result;

    return AddSpecifier(&message);
}


status_t
BMessage::AddSpecifier(const char* property, int32_t index, int32_t range)
{
    DEBUG_FUNCTION_ENTER;
    if (range < 0)
        return B_BAD_VALUE;

    BMessage message(B_RANGE_SPECIFIER);
    status_t result = message.AddString(B_PROPERTY_ENTRY, property);
    if (result != B_OK)
        return result;

    result = message.AddInt32("index", index);
    if (result != B_OK)
        return result;

    result = message.AddInt32("range", range);
    if (result != B_OK)
        return result;

    return AddSpecifier(&message);
}


status_t
BMessage::AddSpecifier(const char* property, const char* name)
{
    DEBUG_FUNCTION_ENTER;
    BMessage message(B_NAME_SPECIFIER);
    status_t result = message.AddString(B_PROPERTY_ENTRY, property);
    if (result != B_OK)
        return result;

    result = message.AddString(B_PROPERTY_NAME_ENTRY, name);
    if (result != B_OK)
        return result;

    return AddSpecifier(&message);
}


status_t
BMessage::AddSpecifier(const BMessage* specifier)
{
    DEBUG_FUNCTION_ENTER;
    status_t result = AddMessage(B_SPECIFIER_ENTRY, specifier);
    if (result != B_OK)
        return result;

    fHeader->current_specifier++;
    fHeader->flags |= MESSAGE_FLAG_HAS_SPECIFIERS;
    return B_OK;
}


status_t
BMessage::SetCurrentSpecifier(int32_t index)
{
    DEBUG_FUNCTION_ENTER;
    if (index < 0)
        return B_BAD_INDEX;

    type_code type;
    int32_t count;
    status_t result = GetInfo(B_SPECIFIER_ENTRY, &type, &count);
    if (result != B_OK)
        return result;

    if (index >= count)
        return B_BAD_INDEX;

    fHeader->current_specifier = index;
    return B_OK;
}


status_t
BMessage::GetCurrentSpecifier(int32_t* index, BMessage* specifier, int32_t* _what,
    const char** property) const
{
    DEBUG_FUNCTION_ENTER;
    if (fHeader == NULL)
        return B_NO_INIT;

    if (index != NULL)
        *index = fHeader->current_specifier;

    if (fHeader->current_specifier < 0
        || (fHeader->flags & MESSAGE_FLAG_WAS_DELIVERED) == 0)
        return B_BAD_SCRIPT_SYNTAX;

    if (specifier) {
        if (FindMessage(B_SPECIFIER_ENTRY, fHeader->current_specifier,
            specifier) != B_OK)
            return B_BAD_SCRIPT_SYNTAX;

        if (_what != NULL)
            *_what = specifier->what;

        if (property) {
            if (specifier->FindString(B_PROPERTY_ENTRY, property) != B_OK)
                return B_BAD_SCRIPT_SYNTAX;
        }
    }

    return B_OK;
}


bool
BMessage::HasSpecifiers() const
{
    DEBUG_FUNCTION_ENTER;
    return fHeader != NULL
        && (fHeader->flags & MESSAGE_FLAG_HAS_SPECIFIERS) != 0;
}


status_t
BMessage::PopSpecifier()
{
    DEBUG_FUNCTION_ENTER;
    if (fHeader == NULL)
        return B_NO_INIT;

    if (fHeader->current_specifier < 0 ||
        (fHeader->flags & MESSAGE_FLAG_WAS_DELIVERED) == 0)
        return B_BAD_VALUE;

    if (fHeader->current_specifier >= 0)
        fHeader->current_specifier--;

    return B_OK;
}


void
BMessage::_UpdateOffsets(uint32_t offset, int32_t change)
{
    // Update the header to match the new position of the fields
    if (offset < fHeader->data_size) {
        field_header* field = fFields;
        for (uint32_t i = 0; i < fHeader->field_count; i++, field++) {
            if (field->offset >= offset)
                field->offset += change;
        }
    }
}


status_t
BMessage::_ResizeData(uint32_t offset, int32_t change)
{
    if (change == 0)
        return B_OK;

    /* optimize for the most usual case: appending data */

    if (change > 0) {
        // We need to make the field bigger
        // check if there is enough free space allocated
        if (fDataAvailable >= (uint32_t)change) {
            // In this case, we just need to move the data after the growing
            // field to get the space at the right place
            if (offset < fHeader->data_size) {
                memmove(fData + offset + change, fData + offset,
                    fHeader->data_size - offset);
            }

            _UpdateOffsets(offset, change);

            fDataAvailable -= change;
            fHeader->data_size += change;
            return B_OK;
        }

        // We need to grow the buffer. We try to optimize reallocations by
        // preallocating space for more fields.
        size_t size = fHeader->data_size * 2;
        size = min_c(size, fHeader->data_size + MAX_DATA_PREALLOCATION);
        size = max_c(size, fHeader->data_size + change);

        uint8* newData = (uint8*)realloc(fData, size);
        if (size > 0 && newData == NULL)
            return B_NO_MEMORY;

        fData = newData;
        if (offset < fHeader->data_size) {
            memmove(fData + offset + change, fData + offset,
                fHeader->data_size - offset);
        }

        fHeader->data_size += change;
        fDataAvailable = size - fHeader->data_size;
    } else {
        ssize_t length = fHeader->data_size - offset + change;
        if (length > 0)
            memmove(fData + offset, fData + offset - change, length);

        // change is negative
        fHeader->data_size += change;
        fDataAvailable -= change;

        if (fDataAvailable > MAX_DATA_PREALLOCATION) {
            ssize_t available = MAX_DATA_PREALLOCATION / 2;
            ssize_t size = fHeader->data_size + available;
            uint8* newData = (uint8*)realloc(fData, size);
            if (size > 0 && newData == NULL) {
                // this is strange, but not really fatal
                _UpdateOffsets(offset, change);
                return B_OK;
            }

            fData = newData;
            fDataAvailable = available;
        }
    }

    _UpdateOffsets(offset, change);
    return B_OK;
}


uint32_t
BMessage::_HashName(const char* name) const
{
    char ch;
    uint32_t result = 0;

    while ((ch = *name++) != 0) {
        result = (result << 7) ^ (result >> 24);
        result ^= ch;
    }

    result ^= result << 12;
    return result;
}


status_t
BMessage::_FindField(const char* name, type_code type, field_header** result)
    const
{
    if (name == NULL)
        return B_BAD_VALUE;

    if (fHeader == NULL)
        return B_NO_INIT;

    if (fHeader->field_count == 0 || fFields == NULL || fData == NULL)
        return B_NAME_NOT_FOUND;

    uint32_t hash = _HashName(name) % fHeader->hash_table_size;
    int32_t nextField = fHeader->hash_table[hash];

    while (nextField >= 0) {
        field_header* field = &fFields[nextField];
        if ((field->flags & FIELD_FLAG_VALID) == 0)
            break;

        if (strncmp((const char*)(fData + field->offset), name,
            field->name_length) == 0) {
            if (type != B_ANY_TYPE && field->type != type)
                return B_BAD_TYPE;

            *result = field;
            return B_OK;
        }

        nextField = field->next_field;
    }

    return B_NAME_NOT_FOUND;
}


status_t
BMessage::_AddField(const char* name, type_code type, bool isFixedSize,
    field_header** result)
{
    if (fHeader == NULL)
        return B_NO_INIT;

    if (fFieldsAvailable <= 0) {
        uint32_t count = fHeader->field_count * 2 + 1;
        count = min_c(count, fHeader->field_count + MAX_FIELD_PREALLOCATION);

        field_header* newFields = (field_header*)realloc(fFields,
            count * sizeof(field_header));
        if (count > 0 && newFields == NULL)
            return B_NO_MEMORY;

        fFields = newFields;
        fFieldsAvailable = count - fHeader->field_count;
    }

    uint32_t hash = _HashName(name) % fHeader->hash_table_size;
    int32_t* nextField = &fHeader->hash_table[hash];
    while (*nextField >= 0)
        nextField = &fFields[*nextField].next_field;
    *nextField = fHeader->field_count;

    field_header* field = &fFields[fHeader->field_count];
    field->type = type;
    field->count = 0;
    field->data_size = 0;
    field->next_field = -1;
    field->offset = fHeader->data_size;
    field->name_length = strlen(name) + 1;
    status_t status = _ResizeData(field->offset, field->name_length);
    if (status != B_OK)
        return status;

    memcpy(fData + field->offset, name, field->name_length);
    field->flags = FIELD_FLAG_VALID;
    if (isFixedSize)
        field->flags |= FIELD_FLAG_FIXED_SIZE;

    fFieldsAvailable--;
    fHeader->field_count++;
    *result = field;
    return B_OK;
}


status_t
BMessage::_RemoveField(field_header* field)
{
    status_t result = _ResizeData(field->offset, -(field->data_size
        + field->name_length));
    if (result != B_OK)
        return result;

    int32_t index = ((uint8*)field - (uint8*)fFields) / sizeof(field_header);
    int32_t nextField = field->next_field;
    if (nextField > index)
        nextField--;

    int32_t* value = fHeader->hash_table;
    for (uint32_t i = 0; i < fHeader->hash_table_size; i++, value++) {
        if (*value > index)
            *value -= 1;
        else if (*value == index)
            *value = nextField;
    }

    field_header* other = fFields;
    for (uint32_t i = 0; i < fHeader->field_count; i++, other++) {
        if (other->next_field > index)
            other->next_field--;
        else if (other->next_field == index)
            other->next_field = nextField;
    }

    size_t size = (fHeader->field_count - index - 1) * sizeof(field_header);
    memmove(fFields + index, fFields + index + 1, size);
    fHeader->field_count--;
    fFieldsAvailable++;

    if (fFieldsAvailable > MAX_FIELD_PREALLOCATION) {
        ssize_t available = MAX_FIELD_PREALLOCATION / 2;
        size = (fHeader->field_count + available) * sizeof(field_header);
        field_header* newFields = (field_header*)realloc(fFields, size);
        if (size > 0 && newFields == NULL) {
            // this is strange, but not really fatal
            return B_OK;
        }

        fFields = newFields;
        fFieldsAvailable = available;
    }

    return B_OK;
}


status_t
BMessage::AddData(const char* name, type_code type, const void* data,
    ssize_t numBytes, bool isFixedSize, int32_t count)
{
    // Note that the "count" argument is only a hint at how many items
    // the caller expects to add to this field. Since we do no item pre-
    // allocation, we ignore this argument.
    DEBUG_FUNCTION_ENTER;
    if (numBytes <= 0 || data == NULL)
        return B_BAD_VALUE;

    if (fHeader == NULL)
        return B_NO_INIT;

    status_t result;
    if (fHeader->message_area >= 0) {
        result = _CopyForWrite();
        if (result != B_OK)
            return result;
    }

    field_header* field = NULL;
    result = _FindField(name, type, &field);
    if (result == B_NAME_NOT_FOUND)
        result = _AddField(name, type, isFixedSize, &field);

    if (result != B_OK)
        return result;

    if (field == NULL)
        return B_ERROR;

    uint32_t offset = field->offset + field->name_length + field->data_size;
    if ((field->flags & FIELD_FLAG_FIXED_SIZE) != 0) {
        if (field->count) {
            ssize_t size = field->data_size / field->count;
            if (size != numBytes)
                return B_BAD_VALUE;
        }

        result = _ResizeData(offset, numBytes);
        if (result != B_OK) {
            if (field->count == 0)
                _RemoveField(field);
            return result;
        }

        memcpy(fData + offset, data, numBytes);
        field->data_size += numBytes;
    } else {
        int32_t change = numBytes + sizeof(uint32_t);
        result = _ResizeData(offset, change);
        if (result != B_OK) {
            if (field->count == 0)
                _RemoveField(field);
            return result;
        }

        uint32_t size = (uint32_t)numBytes;
        memcpy(fData + offset, &size, sizeof(uint32_t));
        memcpy(fData + offset + sizeof(uint32_t), data, size);
        field->data_size += change;
    }

    field->count++;
    return B_OK;
}


status_t
BMessage::RemoveData(const char* name, int32_t index)
{
    DEBUG_FUNCTION_ENTER;
    if (index < 0)
        return B_BAD_INDEX;

    if (fHeader == NULL)
        return B_NO_INIT;

    status_t result;
    if (fHeader->message_area >= 0) {
        result = _CopyForWrite();
        if (result != B_OK)
            return result;
    }

    field_header* field = NULL;
    result = _FindField(name, B_ANY_TYPE, &field);
    if (result != B_OK)
        return result;

    if ((uint32_t)index >= field->count)
        return B_BAD_INDEX;

    if (field->count == 1)
        return _RemoveField(field);

    uint32_t offset = field->offset + field->name_length;
    if ((field->flags & FIELD_FLAG_FIXED_SIZE) != 0) {
        ssize_t size = field->data_size / field->count;
        result = _ResizeData(offset + index * size, -size);
        if (result != B_OK)
            return result;

        field->data_size -= size;
    } else {
        uint8* pointer = fData + offset;
        for (int32_t i = 0; i < index; i++) {
            offset += *(uint32_t*)pointer + sizeof(uint32_t);
            pointer = fData + offset;
        }

        size_t currentSize = *(uint32_t*)pointer + sizeof(uint32_t);
        result = _ResizeData(offset, -currentSize);
        if (result != B_OK)
            return result;

        field->data_size -= currentSize;
    }

    field->count--;
    return B_OK;
}


status_t
BMessage::RemoveName(const char* name)
{
    DEBUG_FUNCTION_ENTER;
    if (fHeader == NULL)
        return B_NO_INIT;

    status_t result;
    if (fHeader->message_area >= 0) {
        result = _CopyForWrite();
        if (result != B_OK)
            return result;
    }

    field_header* field = NULL;
    result = _FindField(name, B_ANY_TYPE, &field);
    if (result != B_OK)
        return result;

    return _RemoveField(field);
}


status_t
BMessage::MakeEmpty()
{
    DEBUG_FUNCTION_ENTER;
    _Clear();
    return _InitHeader();
}


status_t
BMessage::FindData(const char* name, type_code type, int32_t index,
    const void** data, ssize_t* numBytes) const
{
    DEBUG_FUNCTION_ENTER;
    if (data == NULL)
        return B_BAD_VALUE;

    *data = NULL;
    field_header* field = NULL;
    status_t result = _FindField(name, type, &field);
    if (result != B_OK)
        return result;

    if (index < 0 || (uint32_t)index >= field->count)
        return B_BAD_INDEX;

    if ((field->flags & FIELD_FLAG_FIXED_SIZE) != 0) {
        size_t bytes = field->data_size / field->count;
        *data = fData + field->offset + field->name_length + index * bytes;
        if (numBytes != NULL)
            *numBytes = bytes;
    } else {
        uint8* pointer = fData + field->offset + field->name_length;
        for (int32_t i = 0; i < index; i++)
            pointer += *(uint32_t*)pointer + sizeof(uint32_t);

        *data = pointer + sizeof(uint32_t);
        if (numBytes != NULL)
            *numBytes = *(uint32_t*)pointer;
    }

    return B_OK;
}


status_t
BMessage::ReplaceData(const char* name, type_code type, int32_t index,
    const void* data, ssize_t numBytes)
{
    DEBUG_FUNCTION_ENTER;
    if (numBytes <= 0 || data == NULL)
        return B_BAD_VALUE;

    status_t result;
    if (fHeader->message_area >= 0) {
        result = _CopyForWrite();
        if (result != B_OK)
            return result;
    }

    field_header* field = NULL;
    result = _FindField(name, type, &field);
    if (result != B_OK)
        return result;

    if (index < 0 || (uint32_t)index >= field->count)
        return B_BAD_INDEX;

    if ((field->flags & FIELD_FLAG_FIXED_SIZE) != 0) {
        ssize_t size = field->data_size / field->count;
        if (size != numBytes)
            return B_BAD_VALUE;

        memcpy(fData + field->offset + field->name_length + index * size, data,
            size);
    } else {
        uint32_t offset = field->offset + field->name_length;
        uint8* pointer = fData + offset;

        for (int32_t i = 0; i < index; i++) {
            offset += *(uint32_t*)pointer + sizeof(uint32_t);
            pointer = fData + offset;
        }

        size_t currentSize = *(uint32_t*)pointer;
        int32_t change = numBytes - currentSize;
        result = _ResizeData(offset, change);
        if (result != B_OK)
            return result;

        uint32_t newSize = (uint32_t)numBytes;
        memcpy(fData + offset, &newSize, sizeof(uint32_t));
        memcpy(fData + offset + sizeof(uint32_t), data, newSize);
        field->data_size += change;
    }

    return B_OK;
}


bool
BMessage::HasData(const char* name, type_code type, int32_t index) const
{
    DEBUG_FUNCTION_ENTER;
    field_header* field = NULL;
    status_t result = _FindField(name, type, &field);
    if (result != B_OK)
        return false;

    if (index < 0 || (uint32_t)index >= field->count)
        return false;

    return true;
}


/* Static functions for cache initialization and cleanup */
void
BMessage::_StaticInit()
{
    DEBUG_FUNCTION_ENTER2;
    sReplyPorts[0] = create_port(1, "tmp_rport0");
    sReplyPorts[1] = create_port(1, "tmp_rport1");
    sReplyPorts[2] = create_port(1, "tmp_rport2");

    sReplyPortInUse[0] = 0;
    sReplyPortInUse[1] = 0;
    sReplyPortInUse[2] = 0;

    sMsgCache = new BBlockCache(20, sizeof(BMessage), B_OBJECT_CACHE);
}


void
BMessage::_StaticReInitForkedChild()
{
    DEBUG_FUNCTION_ENTER2;

    // overwrite the inherited ports with a set of our own
    sReplyPorts[0] = create_port(1, "tmp_rport0");
    sReplyPorts[1] = create_port(1, "tmp_rport1");
    sReplyPorts[2] = create_port(1, "tmp_rport2");

    sReplyPortInUse[0] = 0;
    sReplyPortInUse[1] = 0;
    sReplyPortInUse[2] = 0;
}


void
BMessage::_StaticCleanup()
{
    DEBUG_FUNCTION_ENTER2;
    delete_port(sReplyPorts[0]);
    sReplyPorts[0] = -1;
    delete_port(sReplyPorts[1]);
    sReplyPorts[1] = -1;
    delete_port(sReplyPorts[2]);
    sReplyPorts[2] = -1;
}


void
BMessage::_StaticCacheCleanup()
{
    DEBUG_FUNCTION_ENTER2;
    delete sMsgCache;
    sMsgCache = NULL;
}


int32_t
BMessage::_StaticGetCachedReplyPort()
{
    DEBUG_FUNCTION_ENTER2;
    int index = -1;
    for (int32_t i = 0; i < sNumReplyPorts; i++) {
        int32_t old = atomic_add(&(sReplyPortInUse[i]), 1);
        if (old == 0) {
            // This entry is free
            index = i;
            break;
        } else {
            // This entry is being used.
            atomic_add(&(sReplyPortInUse[i]), -1);
        }
    }

    return index;
}


status_t
BMessage::_SendMessage(port_id port, team_id portOwner, int32_t token,
    bigtime_t timeout, bool replyRequired, BMessenger& replyTo) const
{
    DEBUG_FUNCTION_ENTER;
    ssize_t size = 0;
    char* buffer = NULL;
    message_header* header = NULL;
    status_t result = B_OK;

    BPrivate::BDirectMessageTarget* direct = NULL;
    BMessage* copy = NULL;
    if (portOwner == BPrivate::current_team())
        BPrivate::gDefaultTokens.AcquireHandlerTarget(token, &direct);

    if (direct != NULL) {
        // We have a direct local message target - we can just enqueue the
        // message in its message queue. This will also prevent possible
        // deadlocks when the queue is full.
        copy = new BMessage(*this);
        if (copy != NULL) {
            header = copy->fHeader;
            header->flags = fHeader->flags;
        } else {
            direct->Release();
            return B_NO_MEMORY;
        }
#ifndef HAIKU_TARGET_PLATFORM_LIBBE_TEST
    } else if ((fHeader->flags & MESSAGE_FLAG_REPLY_AS_KMESSAGE) != 0) {
        KMessage toMessage;
        result = BPrivate::MessageAdapter::ConvertToKMessage(this, toMessage);
        if (result != B_OK)
            return result;

        return toMessage.SendTo(port, token);
    } else if (fHeader->data_size > B_PAGE_SIZE * 10) {
        // ToDo: bind the above size to the max port message size
        // use message passing by area for such a large message
        result = _FlattenToArea(&header);
        if (result != B_OK)
            return result;

        buffer = (char*)header;
        size = sizeof(message_header);

        if (header->message_area >= 0) {
            team_id target = portOwner;
            if (target < 0) {
                port_info info;
                result = get_port_info(port, &info);
                if (result != B_OK) {
                    free(header);
                    return result;
                }
                target = info.team;
            }

            void* address = NULL;
            area_id transfered = _kern_transfer_area(header->message_area,
                &address, B_ANY_ADDRESS, target);
            if (transfered < 0) {
                delete_area(header->message_area);
                free(header);
                return transfered;
            }

            header->message_area = transfered;
        }
#endif
    } else {
        size = FlattenedSize();
        buffer = (char*)malloc(size);
        if (buffer == NULL)
            return B_NO_MEMORY;

        result = Flatten(buffer, size);
        if (result != B_OK) {
            free(buffer);
            return result;
        }

        header = (message_header*)buffer;
    }

    if (!replyTo.IsValid()) {
        BMessenger::Private(replyTo).SetTo(fHeader->reply_team,
            fHeader->reply_port, fHeader->reply_target);

        if (!replyTo.IsValid())
            replyTo = be_app_messenger;
    }

    BMessenger::Private replyToPrivate(replyTo);

    if (replyRequired) {
        header->flags |= MESSAGE_FLAG_REPLY_REQUIRED;
        header->flags &= ~MESSAGE_FLAG_REPLY_DONE;
    }

    header->target = token;
    header->reply_team = replyToPrivate.Team();
    header->reply_port = replyToPrivate.Port();
    header->reply_target = replyToPrivate.Token();
    header->flags |= MESSAGE_FLAG_WAS_DELIVERED;

    if (direct == NULL) {
        KTRACE("BMessage send remote: team: %ld, port: %ld, token: %ld, "
            "message: '%c%c%c%c'", portOwner, port, token,
            char(what >> 24), char(what >> 16), char(what >> 8), (char)what);

        do {
            result = write_port_etc(port, kPortMessageCode, (void*)buffer,
                size, B_RELATIVE_TIMEOUT, timeout);
        } while (result == B_INTERRUPTED);
    }

    if (result == B_OK && IsSourceWaiting()) {
        // the forwarded message will handle the reply - we must not do
        // this anymore
        fHeader->flags |= MESSAGE_FLAG_REPLY_DONE;
    }

    // we need to do this last because it is possible our
    // message might be destroyed after it's enqueued in the
    // target looper. Thus we don't want to do any ops that depend on
    // members of this after the enqueue.
    if (direct != NULL) {
        KTRACE("BMessage send direct: port: %ld, token: %ld, "
            "message: '%c%c%c%c'", port, token,
            char(what >> 24), char(what >> 16), char(what >> 8), (char)what);

        // this is a local message transmission
        direct->AddMessage(copy);
        if (direct->Queue()->IsNextMessage(copy) && port_count(port) <= 0) {
            // there is currently no message waiting, and we need to wakeup the
            // looper
            write_port_etc(port, 0, NULL, 0, B_RELATIVE_TIMEOUT, 0);
        }
        direct->Release();
    }

    free(buffer);
    return result;
}


// Sends a message and waits synchronously for a reply.
status_t
BMessage::_SendMessage(port_id port, team_id portOwner, int32_t token,
    BMessage* reply, bigtime_t sendTimeout, bigtime_t replyTimeout) const
{
    if (IsSourceWaiting()) {
        // we can't forward this message synchronously when it's already
        // waiting for a reply
        return B_ERROR;
    }

    DEBUG_FUNCTION_ENTER;
    const int32_t cachedReplyPort = _StaticGetCachedReplyPort();
    port_id replyPort = B_BAD_PORT_ID;
    status_t result = B_OK;

    if (cachedReplyPort < 0) {
        // All the cached reply ports are in use; create a new one
        replyPort = create_port(1 /* for one message */, "tmp_reply_port");
        if (replyPort < 0)
            return replyPort;
    } else {
        assert(cachedReplyPort < sNumReplyPorts);
        replyPort = sReplyPorts[cachedReplyPort];
    }

    bool recreateCachedPort = false;

    team_id team = B_BAD_TEAM_ID;
    if (be_app != NULL)
        team = be_app->Team();
    else {
        port_info portInfo;
        result = get_port_info(replyPort, &portInfo);
        if (result != B_OK)
            goto error;

        team = portInfo.team;
    }

    result = set_port_owner(replyPort, portOwner);
    if (result != B_OK)
        goto error;

    // tests if the queue of the reply port is really empty
#if 0
    port_info portInfo;
    if (get_port_info(replyPort, &portInfo) == B_OK
        && portInfo.queue_count > 0) {
        debugger("reply port not empty!");
        printf("  reply port not empty! %ld message(s) in queue\n",
            portInfo.queue_count);

        // fetch and print the messages
        for (int32_t i = 0; i < portInfo.queue_count; i++) {
            char buffer[1024];
            int32_t code;
            ssize_t size = read_port(replyPort, &code, buffer, sizeof(buffer));
            if (size < 0) {
                printf("failed to read message from reply port\n");
                continue;
            }
            if (size >= (ssize_t)sizeof(buffer)) {
                printf("message from reply port too big\n");
                continue;
            }

            BMemoryIO stream(buffer, size);
            BMessage reply;
            if (reply.Unflatten(&stream) != B_OK) {
                printf("failed to unflatten message from reply port\n");
                continue;
            }

            printf("message %ld from reply port:\n", i);
            reply.PrintToStream();
        }
    }
#endif

    {
        BMessenger replyTarget;
        BMessenger::Private(replyTarget).SetTo(team, replyPort,
            B_PREFERRED_TOKEN);
        // TODO: replying could also use a BDirectMessageTarget like mechanism
        // for local targets
        result = _SendMessage(port, -1, token, sendTimeout, true,
            replyTarget);
    }

    if (result != B_OK)
        goto error;

    int32_t code;
    result = handle_reply(replyPort, &code, replyTimeout, reply);
    if (result != B_OK && cachedReplyPort >= 0) {
        delete_port(replyPort);
        recreateCachedPort = true;
    }

error:
    if (cachedReplyPort >= 0) {
        // Reclaim ownership of cached port, if possible
        if (!recreateCachedPort && set_port_owner(replyPort, team) == B_OK) {
            // Flag as available
            atomic_add(&sReplyPortInUse[cachedReplyPort], -1);
        } else
            sReplyPorts[cachedReplyPort] = create_port(1, "tmp_rport");

        return result;
    }

    delete_port(replyPort);
    return result;
}


status_t
BMessage::_SendFlattenedMessage(void* data, int32_t size, port_id port,
    int32_t token, bigtime_t timeout)
{
    DEBUG_FUNCTION_ENTER2;
    if (data == NULL)
        return B_BAD_VALUE;

    uint32_t magic = *(uint32_t*)data;

    if (magic == MESSAGE_FORMAT_HAIKU
        || magic == MESSAGE_FORMAT_HAIKU_SWAPPED) {
        message_header* header = (message_header*)data;
        header->target = token;
        header->flags |= MESSAGE_FLAG_WAS_DELIVERED;
    } else if (magic == MESSAGE_FORMAT_R5) {
        uint8* header = (uint8*)data;
        header += sizeof(uint32_t) /* magic */ + sizeof(uint32_t) /* checksum */
            + sizeof(ssize_t) /* flattenedSize */ + sizeof(int32_t) /* what */
            + sizeof(uint8) /* flags */;
        *(int32_t*)header = token;
    } else if (((KMessage::Header*)data)->magic
            == KMessage::kMessageHeaderMagic) {
        KMessage::Header* header = (KMessage::Header*)data;
        header->targetToken = token;
    } else {
        return B_NOT_A_MESSAGE;
    }

    // send the message
    status_t result;

    do {
        result = write_port_etc(port, kPortMessageCode, data, size,
            B_RELATIVE_TIMEOUT, timeout);
    } while (result == B_INTERRUPTED);

    return result;
}


void BMessage::_ReservedMessage1() {}
void BMessage::_ReservedMessage2() {}
void BMessage::_ReservedMessage3() {}


// #pragma mark - Macro definitions for data access methods


/* Relay functions from here on (Add... -> AddData, Find... -> FindData) */

#define DEFINE_FUNCTIONS(type, typeName, typeCode)							\
status_t																	\
BMessage::Add##typeName(const char* name, type val)							\
{																			\
    return AddData(name, typeCode, &val, sizeof(type), true);				\
}																			\
                                                                            \
                                                                            \
status_t																	\
BMessage::Find##typeName(const char* name, type* p) const					\
{																			\
    void* ptr = NULL;														\
    ssize_t bytes = 0;														\
    status_t error = B_OK;													\
                                                                            \
    *p = type();															\
    error = FindData(name, typeCode, 0, (const void**)&ptr, &bytes);		\
                                                                            \
    if (error == B_OK)														\
        memcpy(p, ptr, sizeof(type));										\
                                                                            \
    return error;															\
}																			\
                                                                            \
                                                                            \
status_t																	\
BMessage::Find##typeName(const char* name, int32_t index, type* p) const		\
{																			\
    void* ptr = NULL;														\
    ssize_t bytes = 0;														\
    status_t error = B_OK;													\
                                                                            \
    *p = type();															\
    error = FindData(name, typeCode, index, (const void**)&ptr, &bytes);	\
                                                                            \
    if (error == B_OK)														\
        memcpy(p, ptr, sizeof(type));										\
                                                                            \
    return error;															\
}																			\
                                                                            \
                                                                            \
status_t																	\
BMessage::Replace##typeName(const char* name, type value)					\
{																			\
    return ReplaceData(name, typeCode, 0, &value, sizeof(type));			\
}																			\
                                                                            \
                                                                            \
status_t																	\
BMessage::Replace##typeName(const char* name, int32_t index, type value)		\
{																			\
    return ReplaceData(name, typeCode, index, &value, sizeof(type));		\
}																			\
                                                                            \
                                                                            \
bool																		\
BMessage::Has##typeName(const char* name, int32_t index) const				\
{																			\
    return HasData(name, typeCode, index);									\
}

DEFINE_FUNCTIONS(BPoint, Point, B_POINT_TYPE);
DEFINE_FUNCTIONS(BRect, Rect, B_RECT_TYPE);
DEFINE_FUNCTIONS(BSize, Size, B_SIZE_TYPE);
DEFINE_FUNCTIONS(int8, Int8, B_INT8_TYPE);
DEFINE_FUNCTIONS(uint8, UInt8, B_UINT8_TYPE);
DEFINE_FUNCTIONS(int16, Int16, B_INT16_TYPE);
DEFINE_FUNCTIONS(uint16, UInt16, B_UINT16_TYPE);
DEFINE_FUNCTIONS(int32_t, Int32, B_INT32_TYPE);
DEFINE_FUNCTIONS(uint32_t, UInt32, B_UINT32_TYPE);
DEFINE_FUNCTIONS(int64, Int64, B_INT64_TYPE);
DEFINE_FUNCTIONS(uint64, UInt64, B_UINT64_TYPE);
DEFINE_FUNCTIONS(bool, Bool, B_BOOL_TYPE);
DEFINE_FUNCTIONS(float, Float, B_FLOAT_TYPE);
DEFINE_FUNCTIONS(double, Double, B_DOUBLE_TYPE);

#undef DEFINE_FUNCTIONS

#define DEFINE_HAS_FUNCTION(typeName, typeCode)								\
bool																		\
BMessage::Has##typeName(const char* name, int32_t index) const				\
{																			\
    return HasData(name, typeCode, index);									\
}


DEFINE_HAS_FUNCTION(Alignment, B_ALIGNMENT_TYPE);
DEFINE_HAS_FUNCTION(String, B_STRING_TYPE);
DEFINE_HAS_FUNCTION(Pointer, B_POINTER_TYPE);
DEFINE_HAS_FUNCTION(Messenger, B_MESSENGER_TYPE);
DEFINE_HAS_FUNCTION(Ref, B_REF_TYPE);
DEFINE_HAS_FUNCTION(Message, B_MESSAGE_TYPE);

#undef DEFINE_HAS_FUNCTION


#define DEFINE_LAZY_FIND_FUNCTION(type, typeName, initialize)				\
type																		\
BMessage::Find##typeName(const char* name, int32_t index) const				\
{																			\
    type val = initialize;													\
    Find##typeName(name, index, &val);										\
    return val;																\
}


DEFINE_LAZY_FIND_FUNCTION(BRect, Rect, BRect());
DEFINE_LAZY_FIND_FUNCTION(BPoint, Point, BPoint());
DEFINE_LAZY_FIND_FUNCTION(const char*, String, NULL);
DEFINE_LAZY_FIND_FUNCTION(int8, Int8, 0);
DEFINE_LAZY_FIND_FUNCTION(int16, Int16, 0);
DEFINE_LAZY_FIND_FUNCTION(int32_t, Int32, 0);
DEFINE_LAZY_FIND_FUNCTION(int64, Int64, 0);
DEFINE_LAZY_FIND_FUNCTION(bool, Bool, false);
DEFINE_LAZY_FIND_FUNCTION(float, Float, 0);
DEFINE_LAZY_FIND_FUNCTION(double, Double, 0);

#undef DEFINE_LAZY_FIND_FUNCTION


#define DEFINE_SET_GET_FUNCTIONS(type, typeName, typeCode)					\
type																		\
BMessage::Get##typeName(const char* name, type defaultValue) const			\
{																			\
    return Get##typeName(name, 0, defaultValue);							\
}																			\
                                                                            \
                                                                            \
type																		\
BMessage::Get##typeName(const char* name, int32_t index,						\
    type defaultValue) const												\
{																			\
    type value;																\
    if (Find##typeName(name, index, &value) == B_OK)						\
        return value;														\
                                                                            \
    return defaultValue;													\
}																			\
                                                                            \
                                                                            \
status_t																	\
BMessage::Set##typeName(const char* name, type value)						\
{																			\
    return SetData(name, typeCode, &value, sizeof(type));					\
}																			\


DEFINE_SET_GET_FUNCTIONS(int8, Int8, B_INT8_TYPE);
DEFINE_SET_GET_FUNCTIONS(uint8, UInt8, B_UINT8_TYPE);
DEFINE_SET_GET_FUNCTIONS(int16, Int16, B_INT16_TYPE);
DEFINE_SET_GET_FUNCTIONS(uint16, UInt16, B_UINT16_TYPE);
DEFINE_SET_GET_FUNCTIONS(int32_t, Int32, B_INT32_TYPE);
DEFINE_SET_GET_FUNCTIONS(uint32_t, UInt32, B_UINT32_TYPE);
DEFINE_SET_GET_FUNCTIONS(int64, Int64, B_INT64_TYPE);
DEFINE_SET_GET_FUNCTIONS(uint64, UInt64, B_UINT64_TYPE);
DEFINE_SET_GET_FUNCTIONS(bool, Bool, B_BOOL_TYPE);
DEFINE_SET_GET_FUNCTIONS(float, Float, B_FLOAT_TYPE);
DEFINE_SET_GET_FUNCTIONS(double, Double, B_DOUBLE_TYPE);

#undef DEFINE_SET_GET_FUNCTION


#define DEFINE_SET_GET_BY_REFERENCE_FUNCTIONS(type, typeName, typeCode)		\
type																		\
BMessage::Get##typeName(const char* name, const type& defaultValue) const	\
{																			\
    return Get##typeName(name, 0, defaultValue);							\
}																			\
                                                                            \
                                                                            \
type																		\
BMessage::Get##typeName(const char* name, int32_t index,						\
    const type& defaultValue) const											\
{																			\
    type value;																\
    if (Find##typeName(name, index, &value) == B_OK)						\
        return value;														\
                                                                            \
    return defaultValue;													\
}																			\
                                                                            \
                                                                            \
status_t																	\
BMessage::Set##typeName(const char* name, const type& value)				\
{																			\
    return SetData(name, typeCode, &value, sizeof(type));					\
}																			\


DEFINE_SET_GET_BY_REFERENCE_FUNCTIONS(BPoint, Point, B_POINT_TYPE);
DEFINE_SET_GET_BY_REFERENCE_FUNCTIONS(BRect, Rect, B_RECT_TYPE);
DEFINE_SET_GET_BY_REFERENCE_FUNCTIONS(BSize, Size, B_SIZE_TYPE);

#undef DEFINE_SET_GET_BY_REFERENCE_FUNCTIONS


status_t
BMessage::AddAlignment(const char* name, const BAlignment& alignment)
{
    int32_t data[2] = { alignment.horizontal, alignment.vertical };
    return AddData(name, B_ALIGNMENT_TYPE, data, sizeof(data));
}


status_t
BMessage::AddString(const char* name, const char* string)
{
    return AddData(name, B_STRING_TYPE, string, string ? strlen(string) + 1 : 0,
        false);
}


status_t
BMessage::AddString(const char* name, const BString& string)
{
    return AddData(name, B_STRING_TYPE, string.String(), string.Length() + 1,
        false);
}


status_t
BMessage::AddStrings(const char* name, const BStringList& list)
{
    int32_t count = list.CountStrings();
    for (int32_t i = 0; i < count; i++) {
        status_t error = AddString(name, list.StringAt(i));
        if (error != B_OK)
            return error;
    }

    return B_OK;
}


status_t
BMessage::AddPointer(const char* name, const void* pointer)
{
    return AddData(name, B_POINTER_TYPE, &pointer, sizeof(pointer), true);
}


status_t
BMessage::AddMessenger(const char* name, BMessenger messenger)
{
    return AddData(name, B_MESSENGER_TYPE, &messenger, sizeof(messenger), true);
}


status_t
BMessage::AddRef(const char* name, const entry_ref* ref)
{
    size_t size = sizeof(entry_ref) + B_PATH_NAME_LENGTH;
    char buffer[size];

    status_t error = BPrivate::entry_ref_flatten(buffer, &size, ref);

    if (error >= B_OK)
        error = AddData(name, B_REF_TYPE, buffer, size, false);

    return error;
}


status_t
BMessage::AddMessage(const char* name, const BMessage* message)
{
    if (message == NULL)
        return B_BAD_VALUE;

    // TODO: This and the following functions waste time by allocating and
    // copying an extra buffer. Functions can be added that return a direct
    // pointer into the message.

    char stackBuffer[16384];
    ssize_t size = message->FlattenedSize();

    char* buffer;
    if (size > (ssize_t)sizeof(stackBuffer)) {
        buffer = (char*)malloc(size);
        if (buffer == NULL)
            return B_NO_MEMORY;
    } else
        buffer = stackBuffer;

    status_t error = message->Flatten(buffer, size);

    if (error >= B_OK)
        error = AddData(name, B_MESSAGE_TYPE, buffer, size, false);

    if (buffer != stackBuffer)
        free(buffer);

    return error;
}


status_t
BMessage::AddFlat(const char* name, BFlattenable* object, int32_t count)
{
    return AddFlat(name, (const BFlattenable*)object, count);
}


status_t
BMessage::AddFlat(const char* name, const BFlattenable* object, int32_t count)
{
    if (object == NULL)
        return B_BAD_VALUE;

    char stackBuffer[16384];
    ssize_t size = object->FlattenedSize();

    char* buffer;
    if (size > (ssize_t)sizeof(stackBuffer)) {
        buffer = (char*)malloc(size);
        if (buffer == NULL)
            return B_NO_MEMORY;
    } else
        buffer = stackBuffer;

    status_t error = object->Flatten(buffer, size);

    if (error >= B_OK)
        error = AddData(name, object->TypeCode(), buffer, size, false);

    if (buffer != stackBuffer)
        free(buffer);

    return error;
}


status_t
BMessage::Append(const BMessage& other)
{
    field_header* field = other.fFields;
    for (uint32_t i = 0; i < other.fHeader->field_count; i++, field++) {
        const char* name = (const char*)(other.fData + field->offset);
        const void* data = (const void*)(other.fData + field->offset
            + field->name_length);
        bool isFixed = (field->flags & FIELD_FLAG_FIXED_SIZE) != 0;
        size_t size = field->data_size / field->count;

        for (uint32_t j = 0; j < field->count; j++) {
            if (!isFixed) {
                size = *(uint32_t*)data;
                data = (const void*)((const char*)data + sizeof(uint32_t));
            }

            status_t status = AddData(name, field->type, data, size,
                isFixed, 1);
            if (status != B_OK)
                return status;

            data = (const void*)((const char*)data + size);
        }
    }
    return B_OK;
}


status_t
BMessage::FindAlignment(const char* name, BAlignment* alignment) const
{
    return FindAlignment(name, 0, alignment);
}


status_t
BMessage::FindAlignment(const char* name, int32_t index, BAlignment* alignment)
    const
{
    if (!alignment)
        return B_BAD_VALUE;

    int32_t* data;
    ssize_t bytes;

    status_t err = FindData(name, B_ALIGNMENT_TYPE, index,
        (const void**)&data, &bytes);

    if (err == B_OK) {
        if (bytes != sizeof(int32_t[2]))
            return B_ERROR;

        alignment->horizontal = (enum alignment)(*data);
        alignment->vertical = (vertical_alignment)*(data + 1);
    }

    return err;
}


status_t
BMessage::FindString(const char* name, const char** string) const
{
    return FindString(name, 0, string);
}


status_t
BMessage::FindString(const char* name, int32_t index, const char** string) const
{
    ssize_t bytes;
    return FindData(name, B_STRING_TYPE, index, (const void**)string, &bytes);
}


status_t
BMessage::FindString(const char* name, BString* string) const
{
    return FindString(name, 0, string);
}


status_t
BMessage::FindString(const char* name, int32_t index, BString* string) const
{
    if (string == NULL)
        return B_BAD_VALUE;

    const char* value;
    status_t error = FindString(name, index, &value);

    // Find*() clobbers the object even on failure
    string->SetTo(value);
    return error;
}


status_t
BMessage::FindStrings(const char* name, BStringList* list) const
{
    if (list == NULL)
        return B_BAD_VALUE;

    list->MakeEmpty();

    // get the number of items
    type_code type;
    int32_t count;
    if (GetInfo(name, &type, &count) != B_OK)
        return B_NAME_NOT_FOUND;

    if (type != B_STRING_TYPE)
        return B_BAD_DATA;

    for (int32_t i = 0; i < count; i++) {
        BString string;
        status_t error = FindString(name, i, &string);
        if (error != B_OK)
            return error;
        if (!list->Add(string))
            return B_NO_MEMORY;
    }

    return B_OK;
}


status_t
BMessage::FindPointer(const char* name, void** pointer) const
{
    return FindPointer(name, 0, pointer);
}


status_t
BMessage::FindPointer(const char* name, int32_t index, void** pointer) const
{
    if (pointer == NULL)
        return B_BAD_VALUE;

    void** data = NULL;
    ssize_t size = 0;
    status_t error = FindData(name, B_POINTER_TYPE, index,
        (const void**)&data, &size);

    if (error == B_OK)
        *pointer = *data;
    else
        *pointer = NULL;

    return error;
}


status_t
BMessage::FindMessenger(const char* name, BMessenger* messenger) const
{
    return FindMessenger(name, 0, messenger);
}


status_t
BMessage::FindMessenger(const char* name, int32_t index,
    BMessenger* messenger) const
{
    if (messenger == NULL)
        return B_BAD_VALUE;

    void* data = NULL;
    ssize_t size = 0;
    status_t error = FindData(name, B_MESSENGER_TYPE, index,
        (const void**)&data, &size);

    if (error == B_OK)
        memcpy(messenger, data, sizeof(BMessenger));
    else
        *messenger = BMessenger();

    return error;
}


status_t
BMessage::FindRef(const char* name, entry_ref* ref) const
{
    return FindRef(name, 0, ref);
}


status_t
BMessage::FindRef(const char* name, int32_t index, entry_ref* ref) const
{
    if (ref == NULL)
        return B_BAD_VALUE;

    void* data = NULL;
    ssize_t size = 0;
    status_t error = FindData(name, B_REF_TYPE, index,
        (const void**)&data, &size);

    if (error == B_OK)
        error = BPrivate::entry_ref_unflatten(ref, (char*)data, size);
    else
        *ref = entry_ref();

    return error;
}


status_t
BMessage::FindMessage(const char* name, BMessage* message) const
{
    return FindMessage(name, 0, message);
}


status_t
BMessage::FindMessage(const char* name, int32_t index, BMessage* message) const
{
    if (message == NULL)
        return B_BAD_VALUE;

    void* data = NULL;
    ssize_t size = 0;
    status_t error = FindData(name, B_MESSAGE_TYPE, index,
        (const void**)&data, &size);

    if (error == B_OK)
        error = message->Unflatten((const char*)data);
    else
        *message = BMessage();

    return error;
}


status_t
BMessage::FindFlat(const char* name, BFlattenable* object) const
{
    return FindFlat(name, 0, object);
}


status_t
BMessage::FindFlat(const char* name, int32_t index, BFlattenable* object) const
{
    if (object == NULL)
        return B_BAD_VALUE;

    void* data = NULL;
    ssize_t numBytes = 0;
    status_t error = FindData(name, object->TypeCode(), index,
        (const void**)&data, &numBytes);

    if (error == B_OK)
        error = object->Unflatten(object->TypeCode(), data, numBytes);

    return error;
}


status_t
BMessage::FindData(const char* name, type_code type, const void** data,
    ssize_t* numBytes) const
{
    return FindData(name, type, 0, data, numBytes);
}


status_t
BMessage::ReplaceAlignment(const char* name, const BAlignment& alignment)
{
    int32_t data[2] = {alignment.horizontal, alignment.vertical};
    return ReplaceData(name, B_ALIGNMENT_TYPE, 0, data, sizeof(data));
}


status_t
BMessage::ReplaceAlignment(const char* name, int32_t index,
    const BAlignment& alignment)
{
    int32_t data[2] = {alignment.horizontal, alignment.vertical};
    return ReplaceData(name, B_ALIGNMENT_TYPE, index, data, sizeof(data));
}


status_t
BMessage::ReplaceString(const char* name, const char* string)
{
    if (string == NULL)
        return B_BAD_VALUE;

    return ReplaceData(name, B_STRING_TYPE, 0, string, strlen(string) + 1);
}


status_t
BMessage::ReplaceString(const char* name, int32_t index, const char* string)
{
    if (string == NULL)
        return B_BAD_VALUE;

    return ReplaceData(name, B_STRING_TYPE, index, string, strlen(string) + 1);
}


status_t
BMessage::ReplaceString(const char* name, const BString& string)
{
    return ReplaceData(name, B_STRING_TYPE, 0, string.String(),
        string.Length() + 1);
}


status_t
BMessage::ReplaceString(const char* name, int32_t index, const BString& string)
{
    return ReplaceData(name, B_STRING_TYPE, index, string.String(),
        string.Length() + 1);
}


status_t
BMessage::ReplacePointer(const char* name, const void* pointer)
{
    return ReplaceData(name, B_POINTER_TYPE, 0, &pointer, sizeof(pointer));
}


status_t
BMessage::ReplacePointer(const char* name, int32_t index, const void* pointer)
{
    return ReplaceData(name, B_POINTER_TYPE, index, &pointer, sizeof(pointer));
}


status_t
BMessage::ReplaceMessenger(const char* name, BMessenger messenger)
{
    return ReplaceData(name, B_MESSENGER_TYPE, 0, &messenger,
        sizeof(BMessenger));
}


status_t
BMessage::ReplaceMessenger(const char* name, int32_t index, BMessenger messenger)
{
    return ReplaceData(name, B_MESSENGER_TYPE, index, &messenger,
        sizeof(BMessenger));
}


status_t
BMessage::ReplaceRef(const char* name, const entry_ref* ref)
{
    return ReplaceRef(name, 0, ref);
}


status_t
BMessage::ReplaceRef(const char* name, int32_t index, const entry_ref* ref)
{
    size_t size = sizeof(entry_ref) + B_PATH_NAME_LENGTH;
    char buffer[size];

    status_t error = BPrivate::entry_ref_flatten(buffer, &size, ref);

    if (error >= B_OK)
        error = ReplaceData(name, B_REF_TYPE, index, buffer, size);

    return error;
}


status_t
BMessage::ReplaceMessage(const char* name, const BMessage* message)
{
    return ReplaceMessage(name, 0, message);
}


status_t
BMessage::ReplaceMessage(const char* name, int32_t index, const BMessage* message)
{
    if (message == NULL)
        return B_BAD_VALUE;

    ssize_t size = message->FlattenedSize();
    char buffer[size];

    status_t error = message->Flatten(buffer, size);

    if (error >= B_OK)
        error = ReplaceData(name, B_MESSAGE_TYPE, index, &buffer, size);

    return error;
}


status_t
BMessage::ReplaceFlat(const char* name, BFlattenable* object)
{
    return ReplaceFlat(name, 0, object);
}


status_t
BMessage::ReplaceFlat(const char* name, int32_t index, BFlattenable* object)
{
    if (object == NULL)
        return B_BAD_VALUE;

    ssize_t size = object->FlattenedSize();
    char buffer[size];

    status_t error = object->Flatten(buffer, size);

    if (error >= B_OK)
        error = ReplaceData(name, object->TypeCode(), index, &buffer, size);

    return error;
}


status_t
BMessage::ReplaceData(const char* name, type_code type, const void* data,
    ssize_t numBytes)
{
    return ReplaceData(name, type, 0, data, numBytes);
}


bool
BMessage::HasFlat(const char* name, const BFlattenable* object) const
{
    return HasFlat(name, 0, object);
}


bool
BMessage::HasFlat(const char* name, int32_t index, const BFlattenable* object)
    const
{
    return HasData(name, object->TypeCode(), index);
}


const char*
BMessage::GetString(const char* name, const char* defaultValue) const
{
    return GetString(name, 0, defaultValue);
}


const char*
BMessage::GetString(const char* name, int32_t index,
    const char* defaultValue) const
{
    const char* value;
    if (FindString(name, index, &value) == B_OK)
        return value;

    return defaultValue;
}


status_t
BMessage::SetString(const char* name, const BString& value)
{
    return SetData(name, B_STRING_TYPE, value.String(), value.Length() + 1,
        false);
}


status_t
BMessage::SetString(const char* name, const char* value)
{
    return SetData(name, B_STRING_TYPE, value, strlen(value) + 1, false);
}


status_t
BMessage::SetData(const char* name, type_code type, const void* data,
    ssize_t numBytes, bool fixedSize, int count)
{
    if (numBytes <= 0 || data == NULL)
        return B_BAD_VALUE;

    if (ReplaceData(name, type, data, numBytes) == B_OK)
        return B_OK;

    return AddData(name, type, data, numBytes, fixedSize, count);
}
