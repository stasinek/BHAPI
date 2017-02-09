/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./../kernel/Debug.h"
#include "./../private/Token.h"
#include "./../support/StreamIO.h"
#include "./../support/StringMe.h"
#include "./../support/Errors.h"
#include "./../kernel/OS.h"
#include "./../interface/Point.h"
#include "./../interface/Rect.h"


#include "Message.h"
#include "Messenger.h"
#include "Handler.h"

b_status_t
BMessage::BGetInfo(b_type_code type, b_int32 index,
           char **nameFound, b_type_code *typeFound, b_int32 *countFound) const
{
    if(index < 0) return B_BAD_INDEX;
    b_int32 aIndex = index;

    for(b_int32 i = 0; i < CountNames(B_ANY_TYPE, true); i++)
    {
        b_int32 typesCount = CountTypesByName(i);
        for(b_int32 k = 0; k < typesCount; k++)
        {
            b_type_code aType;
            b_int32 count = CountItems(i, k, &aType);
            if(!(type == B_ANY_TYPE || aType == type) || (aIndex--) > 0) continue;
            if(nameFound) *nameFound = (char*)NameAt(i);
            if(typeFound) *typeFound = aType;
            if(countFound) *countFound = count;
            return B_OK;
        }
    }

    return(aIndex == index ? B_BAD_TYPE : B_BAD_INDEX);
}


b_status_t
BMessage::BFindData(const char *name, b_type_code type, b_int32 index,
            const void **data, b_size_t *numBytes) const
{
    if(index < 0) return B_BAD_INDEX;

    b_int32 nameIndex = FindName(name);
    if(nameIndex < 0) return B_NAME_NOT_FOUND;

    b_int32 typesCount = CountTypesByName(nameIndex);
    b_int32 aIndex = index;

    for(b_int32 k = 0; k < typesCount; k++)
    {
        b_type_code aType;
        b_int32 count = CountItems(nameIndex, k, &aType);
        if(!(type == B_ANY_TYPE || aType == type)) continue;

        if(aIndex < count)
            return(FindData(nameIndex, k, aIndex, data, numBytes) ? B_OK : B_ERROR);

        aIndex -= count;
    }

    return(aIndex == index ? B_BAD_TYPE : B_BAD_INDEX);
}


b_status_t
BMessage::BFindData(const char *name, b_type_code type,
            const void **data, b_size_t *numBytes) const
{
    return BFindData(name, type, 0, data, numBytes);
}

BMessage::BMessage()
	: what(0),
	  fTargetToken(B_MAXUINT64), fTargetTokenTimestamp(B_INT64_CONSTANT(0)),
	  fReplyToken(B_MAXUINT64), fReplyTokenTimestamp(B_INT64_CONSTANT(0)),
	  fNoticeSource(false), fSource(NULL), fIsReply(false)
{
	fTeam = bhapi_get_current_team_id();
}


BMessage::BMessage(b_uint32 a_what)
	: fTargetToken(B_MAXUINT64), fTargetTokenTimestamp(B_INT64_CONSTANT(0)),
	  fReplyToken(B_MAXUINT64), fReplyTokenTimestamp(B_INT64_CONSTANT(0)),
	  fNoticeSource(false), fSource(NULL), fIsReply(false)
{
    BMessage::what = a_what;
	fTeam = bhapi_get_current_team_id();
}


BMessage::BMessage(const BMessage &msg)
	: what(0), fTeam(B_INT64_CONSTANT(0)),
	  fTargetToken(B_MAXUINT64), fTargetTokenTimestamp(B_INT64_CONSTANT(0)),
	  fReplyToken(B_MAXUINT64), fReplyTokenTimestamp(B_INT64_CONSTANT(0)),
	  fNoticeSource(false), fSource(NULL), fIsReply(false)
{
	operator=(msg);
}


BMessage&
BMessage::operator=(const BMessage &msg)
{
	what = msg.what;

	MakeEmpty();

	for(b_int32 k = 0; k < msg.fObjectsList.CountItems(); k++)
	{
		list_data *ldata = (list_data*)msg.fObjectsList.ItemAt(k);
		if(!ldata) continue;

		for(b_int32 j = 0; j < ldata->list.CountItems(); j++)
		{
			type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(j);
			if(!tldata) continue;

			for(b_int32 i = 0; i < tldata->list.CountItems(); i++)
			{
				_object_t *Object = (_object_t*)tldata->list.ItemAt(i);
				if(!Object) continue;

				AddData(ldata->name, tldata->type, Object->data, Object->bytes, Object->fixed_size);
			}
		}
	}

	if(fSource != NULL)
	{
		if(fNoticeSource) bhapi_close_port(fSource);
		bhapi_delete_port(fSource);
		fSource = NULL;
	}

	fTargetToken = B_MAXUINT64;
	fTargetTokenTimestamp = B_MAXINT64;
	fReplyToken = B_MAXUINT64;
	fReplyTokenTimestamp = B_MAXINT64;

	if(msg.fTeam == bhapi_get_current_team_id())
	{
		fTargetToken = msg.fTargetToken;
		fTargetTokenTimestamp = msg.fTargetTokenTimestamp;
	}

	if(msg.fSource == NULL)
	{
		fReplyToken = msg.fReplyToken;
		fReplyTokenTimestamp = msg.fReplyTokenTimestamp;
	}
	else if(msg.fTeam == bhapi_get_current_team_id())
	{
		fSource = bhapi_open_port_by_source(msg.fSource);
	}

	fTeam = msg.fTeam;
	fIsReply = msg.fIsReply;
	fNoticeSource = false;

	return *this;
}


size_t
BMessage::FlattenedSize() const
{
	size_t size = sizeof(size_t) + sizeof(b_uint32) + sizeof(b_uint64); // FlattenSize + msg->what + recordCount

	size += sizeof(b_int64); // fTeam
	size += sizeof(b_uint64) + sizeof(b_bigtime_t); // fTargetToken + fTargetTokenTimestamp
	size += sizeof(b_uint64) + sizeof(b_bigtime_t); // fReplyToken + fReplyTokenTimestamp
	size += sizeof(b_address_t); // fSource
	size += sizeof(bool); // fIsReply

	for(b_int32 k = 0; k < fObjectsList.CountItems(); k++)
	{
		list_data *ldata = (list_data*)fObjectsList.ItemAt(k);
		if(!ldata) continue;

		size_t nameLen = (size_t)(ldata->name ? strlen(ldata->name) : 0);

		for(b_int32 j = 0; j < ldata->list.CountItems(); j++)
		{
			type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(j);
			if(!tldata) continue;

			for(b_int32 i = 0; i < tldata->list.CountItems(); i++)
			{
				_object_t *Object = (_object_t*)tldata->list.ItemAt(i);
				if(!Object) continue;

				// msg->_object_t
				size_t objectSize = sizeof(size_t) + nameLen + sizeof(b_type_code) + sizeof(bool) + sizeof(size_t);
				size_t dataLen = Object->fixed_size ? Object->bytes : sizeof(b_address_t);
				objectSize += dataLen;

				size += objectSize;
			}
		}
	}

	return size;
}


bool
BMessage::Flatten(char *buffer, size_t bufferSize) const
{
	if(buffer == NULL ||
	   bufferSize < sizeof(size_t) + sizeof(b_uint32) + sizeof(b_uint64) +
			sizeof(b_int64) + 2 * (sizeof(b_uint64) + sizeof(b_bigtime_t)) + sizeof(b_address_t) + sizeof(bool)) return false;

	size_t size = 0;

	// msg->what
	size += sizeof(size_t) + sizeof(b_uint32) + sizeof(b_uint64) +
		sizeof(b_int64) + 2 * (sizeof(b_uint64) + sizeof(b_bigtime_t)) + sizeof(b_address_t) + sizeof(bool);
	char *dst = buffer;
	dst += sizeof(size_t);
	memcpy(dst, &what, sizeof(b_uint32)); dst += sizeof(b_uint32);

	// fTeam
	memcpy(dst, &fTeam, sizeof(b_int64)); dst += sizeof(b_int64);

	// fTargetToken + fTargetTokenTimestamp
	memcpy(dst, &fTargetToken, sizeof(b_uint64)); dst += sizeof(b_uint64);
	memcpy(dst, &fTargetTokenTimestamp, sizeof(b_bigtime_t)); dst += sizeof(b_bigtime_t);

	// fReplyToken + fReplyTokenTimestamp
	memcpy(dst, &fReplyToken, sizeof(b_uint64)); dst += sizeof(b_uint64);
	memcpy(dst, &fReplyTokenTimestamp, sizeof(b_bigtime_t)); dst += sizeof(b_bigtime_t);

	// fSource
    b_address_t sourcb_address = reinterpret_cast<b_address_t>(fSource);
    memcpy(dst, &sourcb_address, sizeof(b_address_t)); dst += sizeof(b_address_t);

	// fIsReply
	memcpy(dst, &fIsReply, sizeof(bool)); dst += sizeof(bool);

	// recordCount
	char *countDst = dst; dst += sizeof(b_uint64);
    b_uint64 count = (b_uint64)B_INT64_CONSTANT(0);

	for(b_int32 k = 0; k < fObjectsList.CountItems(); k++)
	{
		list_data *ldata = (list_data*)fObjectsList.ItemAt(k);
		if(!ldata) continue;

		size_t nameLen = (size_t)(ldata->name ? strlen(ldata->name) : 0);

		for(b_int32 j = 0; j < ldata->list.CountItems(); j++)
		{
			type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(j);
			if(!tldata) continue;

			for(b_int32 i = 0; i < tldata->list.CountItems(); i++)
			{
				_object_t *Object = (_object_t*)tldata->list.ItemAt(i);
				if(!Object) continue;

				// msg->_object_t
				size_t dataLen = Object->fixed_size ? Object->bytes : sizeof(b_address_t);
				size_t objectSize = sizeof(size_t) + nameLen + sizeof(b_type_code) + sizeof(bool) + sizeof(size_t) + dataLen;
				size += objectSize;

				if(bufferSize < size) return false;

				memcpy(dst, &nameLen, sizeof(size_t)); dst += sizeof(size_t);
				if(nameLen > 0) {memcpy(dst, ldata->name, nameLen); dst += nameLen;}
				memcpy(dst, &(tldata->type), sizeof(b_type_code)); dst += sizeof(b_type_code);
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
                        b_address_t address = reinterpret_cast<b_address_t>(Object->data);
						memcpy(dst, &address, dataLen);
					}

					dst += dataLen;
				}

				count++;
			}
		}
	}

	// recordCount
	memcpy(countDst, &count, sizeof(b_uint64));

	memcpy(buffer, &size, sizeof(size_t));

	return true;
}


bool
BMessage::Unflatten(const char *buffer, size_t bufferSize)
{
	if(buffer == NULL ||
	   bufferSize < sizeof(size_t) + sizeof(b_uint32) + sizeof(b_uint64) +
			sizeof(b_int64) + 2 * (sizeof(b_uint64) + sizeof(b_bigtime_t)) + sizeof(b_address_t) + sizeof(bool)) return false;

	const char *src = buffer;
	BMessage msg;
	b_uint64 recordCount = 0;
	size_t _bufferSize = 0;

	memcpy(&_bufferSize, src, sizeof(size_t));
	if(bufferSize < _bufferSize) return false;
	src += sizeof(size_t); bufferSize -= sizeof(size_t);

	// msg->what
	memcpy(&msg.what, src, sizeof(b_uint32)); src += sizeof(b_uint32); bufferSize -= sizeof(b_uint32);

	// fTeam
	memcpy(&msg.fTeam, src, sizeof(b_int64)); src += sizeof(b_int64); bufferSize -= sizeof(b_int64);

	// fTargetToken + fTargetTokenTimestamp
	memcpy(&msg.fTargetToken, src, sizeof(b_uint64)); src += sizeof(b_uint64); bufferSize -= sizeof(b_uint64);
	memcpy(&msg.fTargetTokenTimestamp, src, sizeof(b_bigtime_t)); src += sizeof(b_bigtime_t); bufferSize -= sizeof(b_bigtime_t);

	// fReplyToken + fReplyTokenTimestamp
	memcpy(&msg.fReplyToken, src, sizeof(b_uint64)); src += sizeof(b_uint64); bufferSize -= sizeof(b_uint64);
	memcpy(&msg.fReplyTokenTimestamp, src, sizeof(b_bigtime_t)); src += sizeof(b_bigtime_t); bufferSize -= sizeof(b_bigtime_t);

	// fSource
    b_address_t sourcb_address;
    memcpy(&sourcb_address, src, sizeof(b_address_t)); src += sizeof(b_address_t); bufferSize -= sizeof(b_address_t);

	// fIsReply
	memcpy(&msg.fIsReply, src, sizeof(bool)); src += sizeof(bool); bufferSize -= sizeof(bool);

	// recordCount
	memcpy(&recordCount, src, sizeof(b_uint64)); src += sizeof(b_uint64); bufferSize -= sizeof(b_uint64);

    for(b_uint64 i = (b_uint64)B_INT64_CONSTANT(0); i < recordCount; i++)
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
		if(bufferSize < sizeof(b_type_code)) {if(name) free(name); return false;}
		b_type_code type;
		memcpy(&type, src, sizeof(b_type_code)); src += sizeof(b_type_code); bufferSize -= sizeof(b_type_code);

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
				size_t dataLen = sizeof(b_address_t);
				if(bytes != dataLen || bufferSize < dataLen) {if(name) free(name); return false;}

				b_address_t address = 0;
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
		if(fNoticeSource) bhapi_close_port(fSource);
		bhapi_delete_port(fSource);
		fSource = NULL;
	}

	fTargetToken = B_MAXUINT64;
	fTargetTokenTimestamp = B_MAXINT64;
	fReplyToken = B_MAXUINT64;
	fReplyTokenTimestamp = B_MAXINT64;

	if(msg.fTeam == bhapi_get_current_team_id())
	{
		fTargetToken = msg.fTargetToken;
		fTargetTokenTimestamp = msg.fTargetTokenTimestamp;
	}

    void *source = (msg.fTeam != bhapi_get_current_team_id() ? (void*)NULL : reinterpret_cast<void*>(sourcb_address));
	if(source == NULL)
	{
		if(msg.fTeam == bhapi_get_current_team_id())
		{
			fReplyToken = msg.fReplyToken;
			fReplyTokenTimestamp = msg.fReplyTokenTimestamp;
		}
	}
	else
	{
		// TODO: not safe
#if 0
		fSource = bhapi_open_port_by_source(source);
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

	for(b_int32 k = 0; k < fObjectsList.CountItems(); k++)
	{
		list_data *ldata = (list_data*)fObjectsList.ItemAt(k);
		if(!ldata)
		{
			BHAPI_WARNING("Message::(%I32i) is NULL!", k);
			continue;
		}

		const char *name = ldata->name ? (strlen(ldata->name) > 0 ? (const char*)ldata->name : "NULL") : "ERROR-NULL";
        b_uint64 count = (b_uint64)B_INT64_CONSTANT(0);

		for(b_int32 j = 0; j < ldata->list.CountItems(); j++)
		{
			type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(j);
			if(!tldata)
			{
				BHAPI_WARNING("Message::(%I32i - %I32i) is NULL!", k, j);
				continue;
			}

			for(b_int32 i = 0; i < tldata->list.CountItems(); i++)
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
								BHAPI_OUTPUT("\tINT8\t%I8i\n", *((b_int8*)Object->data));
								break;

							case B_INT16_TYPE:
								BHAPI_OUTPUT("\tINT16\t%I16i\n", *((b_int16*)Object->data));
								break;

							case B_INT32_TYPE:
								BHAPI_OUTPUT("\tINT32\t%I32i\n", *((b_int32*)Object->data));
								break;

							case B_INT64_TYPE:
								BHAPI_OUTPUT("\tINT64\t%I64i\n", *((b_int64*)Object->data));
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
		if(fNoticeSource) bhapi_close_port(fSource);
		bhapi_delete_port(fSource);
	}
}


BMessage::list_data*
BMessage::_find_list(const BList *list, const char *name, b_int32 *index)
{
	if(!list || !name) return NULL;

	for(b_int32 i = 0; i < list->CountItems(); i++)
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
BMessage::_find_type_list(const BList *list, b_type_code type)
{
	if(!list) return NULL;

	for(b_int32 i = 0; i < list->CountItems(); i++)
	{
		type_list_data *data = (type_list_data*)list->ItemAt(i);
		if(!data) continue;
		if(data->type == type) return data;
	}

	return NULL;
}


b_int32
BMessage::CountItems(const char *name, b_type_code type) const
{
	if(!name) return -1;

	list_data *ldata = _find_list(&fObjectsList, name);
	if(!ldata) return -1;

	type_list_data *tldata = _find_type_list(&(ldata->list), type);
	if(!tldata) return -1;

	return tldata->list.CountItems();
}


b_int32
BMessage::CountItems(b_int32 nameIndex, b_int32 typeIndex, b_type_code *type) const
{
	list_data *ldata = (list_data*)fObjectsList.ItemAt(nameIndex);
	if(!ldata) return -1;

	type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(typeIndex);
	if(!tldata) return -1;

	if(type) *type = tldata->type;
	return tldata->list.CountItems();
}


bool
BMessage::TypeAt(const char *name, b_int32 typeIndex, b_type_code *type) const
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
BMessage::TypeAt(b_int32 nameIndex, b_int32 typeIndex, b_type_code *type) const
{
	if(!type) return false;

	list_data *ldata = (list_data*)fObjectsList.ItemAt(nameIndex);
	if(!ldata) return false;

	type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(typeIndex);
	if(!tldata) return false;

	*type = tldata->type;

	return true;
}


b_int32
BMessage::CountTypesByName(const char *name) const
{
	if(!name) return -1;

	list_data *ldata = _find_list(&fObjectsList, name);
	if(!ldata) return -1;

	return ldata->list.CountItems();
}


b_int32
BMessage::CountTypesByName(b_int32 nameIndex) const
{
	list_data *ldata = (list_data*)fObjectsList.ItemAt(nameIndex);
	if(!ldata) return -1;

	return ldata->list.CountItems();
}


b_int32
BMessage::CountNames(b_type_code type, bool count_all_names_when_any_type) const
{
	if(type == B_ANY_TYPE && count_all_names_when_any_type) return fObjectsList.CountItems();

	b_int32 retVal = 0;

	for(b_int32 i = 0; i < fObjectsList.CountItems(); i++)
	{
		list_data *ldata = (list_data*)fObjectsList.ItemAt(i);
		if(ldata == NULL) continue;

		for(b_int32 k = 0; k < ldata->list.CountItems(); k++)
		{
			type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(i);
			if(tldata == NULL) continue;
			if(tldata->type == type) {retVal++; break;}
		}
	}

	return retVal;
}


b_int32
BMessage::FindName(const char *name) const
{
	b_int32 index = -1;
	_find_list(&fObjectsList, name, &index);
	return index;
}


const char*
BMessage::NameAt(b_int32 nameIndex) const
{
	list_data *data = (list_data*)fObjectsList.ItemAt(nameIndex);
	return(data ? data->name : NULL);
}


void
BMessage::MakeEmpty()
{
	for(b_int32 k = 0; k < fObjectsList.CountItems(); k++)
	{
		list_data *ldata = (list_data*)fObjectsList.ItemAt(k);
		if(!ldata) continue;

		for(b_int32 j = 0; j < ldata->list.CountItems(); j++)
		{
			type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(j);
			if(!tldata) continue;

			for(b_int32 i = 0; i < tldata->list.CountItems(); i++)
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

	char *newName = b_strdup(new_entry);
	if(!newName) return false;

	delete[] ldata->name;
	ldata->name = newName;

	return true;
}


bool
BMessage::AddData(const char *name, b_type_code type, const void *data, size_t numBytes, bool is_fixed_size)
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
			if((ldata->name = b_strdup(name)) != NULL)
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
BMessage::AddInt8(const char *name, b_int8 val)
{
	if(!name) return false;
	return AddData(name, B_INT8_TYPE, (const void*)&val, sizeof(b_int8), true);
}


bool
BMessage::AddInt16(const char *name, b_int16 val)
{
	if(!name) return false;
	return AddData(name, B_INT16_TYPE, (const void*)&val, sizeof(b_int16), true);
}


bool
BMessage::AddInt32(const char *name, b_int32 val)
{
	if(!name) return false;
	return AddData(name, B_INT32_TYPE, (const void*)&val, sizeof(b_int32), true);
}


bool
BMessage::AddInt64(const char *name, b_int64 val)
{
	if(!name) return false;
	return AddData(name, B_INT64_TYPE, (const void*)&val, sizeof(b_int64), true);
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
BMessage::FindData(const char *name, b_type_code type, const void **data, b_size_t *numBytes) const
{
	return FindData(name, type, 0, data, numBytes);
}


bool
BMessage::FindData(const char *name, b_type_code type, b_int32 index, const void **data, b_size_t *numBytes) const
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
			*numBytes = (b_size_t)Object->bytes;
		else
			*numBytes = -1;
	}

	return true;
}


bool
BMessage::FindData(b_int32 nameIndex, b_int32 typeIndex, b_int32 index, const void **data, b_size_t *numBytes) const
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
			*numBytes = (b_size_t)Object->bytes;
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
BMessage::FindString(const char *name, b_int32 index, const char **str) const
{
	return FindData(name, B_STRING_TYPE, index, (const void**)str, NULL);
}


bool
BMessage::FindString(const char *name, BString *str) const
{
    return FindString(name, 0, str);
}


bool
BMessage::FindString(const char *name, b_int32 index, BString *str) const
{
	const char *string = NULL;
	if(!FindData(name, B_STRING_TYPE, index, (const void**)&string, NULL)) return false;
	if(str) str->SetTo(string);
	return true;
}


bool
BMessage::FindInt8(const char *name, b_int8 *val) const
{
	return FindInt8(name, 0, val);
}


bool
BMessage::FindInt8(const char *name, b_int32 index, b_int8 *val) const
{
	const b_int8 *value = NULL;
	if(!FindData(name, B_INT8_TYPE, index, (const void**)&value, NULL)) return false;
	if(val) *val = *value;
	return true;
}


bool
BMessage::FindInt16(const char *name, b_int16 *val) const
{
	return FindInt16(name, 0, val);
}


bool
BMessage::FindInt16(const char *name, b_int32 index, b_int16 *val) const
{
	const b_int16 *value = NULL;
	if(!FindData(name, B_INT16_TYPE, index, (const void**)&value, NULL)) return false;
	if(val) *val = *value;
	return true;
}


bool
BMessage::FindInt32(const char *name, b_int32 *val) const
{
	return FindInt32(name, 0, val);
}


bool
BMessage::FindInt32(const char *name, b_int32 index, b_int32 *val) const
{
	const b_int32 *value = NULL;
	if(!FindData(name, B_INT32_TYPE, index, (const void**)&value, NULL)) return false;
	if(val) *val = *value;
	return true;
}


bool
BMessage::FindInt64(const char *name, b_int64 *val) const
{
	return FindInt64(name, 0, val);
}


bool
BMessage::FindInt64(const char *name, b_int32 index, b_int64 *val) const
{
	const b_int64 *value = NULL;
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
BMessage::FindBool(const char *name, b_int32 index, bool *aBoolean) const
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
BMessage::FindFloat(const char *name, b_int32 index, float *f) const
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
BMessage::FindDouble(const char *name, b_int32 index, double *d) const
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
BMessage::FindPoint(const char *name, b_int32 index, BPoint *pt) const
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
BMessage::FindRect(const char *name, b_int32 index, BRect *r) const
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
BMessage::FindPointer(const char *name, b_int32 index, void **ptr) const
{
	return FindData(name, B_POINTER_TYPE, index, (const void**)ptr, NULL);
}


bool
BMessage::FindMessage(const char *name, BMessage *msg) const
{
	return FindMessage(name, 0, msg);
}


bool
BMessage::FindMessage(const char *name, b_int32 index, BMessage *msg) const
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
BMessage::FindMessenger(const char *name, b_int32 index, BMessenger *msgr) const
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
BMessage::HasData(const char *name, b_type_code type, b_int32 index) const
{
	return FindData(name, type, index, NULL, NULL);
}


bool
BMessage::HasString(const char *name, b_int32 index) const
{
    return FindString(name, index, (const char**)NULL);
}


bool
BMessage::HasInt8(const char *name, b_int32 index) const
{
	return FindInt8(name, index, NULL);
}


bool
BMessage::HasInt16(const char *name, b_int32 index) const
{
	return FindInt16(name, index, NULL);
}


bool
BMessage::HasInt32(const char *name, b_int32 index) const
{
	return FindInt32(name, index, NULL);
}


bool
BMessage::HasInt64(const char *name, b_int32 index) const
{
	return FindInt64(name, index, NULL);
}


bool
BMessage::HasBool(const char *name, b_int32 index) const
{
	return FindBool(name, index, NULL);
}


bool
BMessage::HasFloat(const char *name, b_int32 index) const
{
	return FindFloat(name, index, NULL);
}


bool
BMessage::HasDouble(const char *name, b_int32 index) const
{
	return FindDouble(name, index, NULL);
}


bool
BMessage::HasPoint(const char *name, b_int32 index) const
{
	return FindPoint(name, index, NULL);
}


bool
BMessage::HasRect(const char *name, b_int32 index) const
{
	return FindRect(name, index, NULL);
}


bool
BMessage::HasPointer(const char *name, b_int32 index) const
{
	return FindPointer(name, index, NULL);
}


bool
BMessage::HasMessage(const char *name, b_int32 index) const
{
	return FindMessage(name, index, NULL);
}


bool
BMessage::HasMessenger(const char *name, b_int32 index) const
{
	return FindMessenger(name, index, NULL);
}


bool
BMessage::RemoveData(const char *name, b_type_code type, b_int32 index)
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
BMessage::RemoveData(const char *name, b_type_code type)
{
	if(!name) return false;

	list_data *ldata = _find_list(&fObjectsList, name);
	if(!ldata) return false;

	type_list_data *tldata = _find_type_list(&(ldata->list), type);
	if(!tldata) return false;

	for(b_int32 i = 0; i < tldata->list.CountItems(); i++)
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

	for(b_int32 j = 0; j < ldata->list.CountItems(); j++)
	{
		type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(j);
		if(!tldata) continue;

		for(b_int32 i = 0; i < tldata->list.CountItems(); i++)
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
BMessage::RemovString(const char *name, b_int32 index)
{
	return RemoveData(name, B_STRING_TYPE, index);
}


bool
BMessage::RemoveInt8(const char *name, b_int32 index)
{
	return RemoveData(name, B_INT8_TYPE, index);
}


bool
BMessage::RemoveInt16(const char *name, b_int32 index)
{
	return RemoveData(name, B_INT16_TYPE, index);
}


bool
BMessage::RemoveInt32(const char *name, b_int32 index)
{
	return RemoveData(name, B_INT32_TYPE, index);
}


bool
BMessage::RemoveInt64(const char *name, b_int32 index)
{
	return RemoveData(name, B_INT64_TYPE, index);
}


bool
BMessage::RemoveBool(const char *name, b_int32 index)
{
	return RemoveData(name, B_BOOL_TYPE, index);
}


bool
BMessage::RemoveFloat(const char *name, b_int32 index)
{
	return RemoveData(name, B_FLOAT_TYPE, index);
}


bool
BMessage::RemoveDouble(const char *name, b_int32 index)
{
	return RemoveData(name, B_DOUBLE_TYPE, index);
}


bool
BMessage::RemovePoint(const char *name, b_int32 index)
{
	return RemoveData(name, B_POINT_TYPE, index);
}


bool
BMessage::RemoveRect(const char *name, b_int32 index)
{
	return RemoveData(name, B_RECT_TYPE, index);
}


bool
BMessage::RemovePointer(const char *name, b_int32 index)
{
	return RemoveData(name, B_POINTER_TYPE, index);
}


bool
BMessage::RemoveMessage(const char *name, b_int32 index)
{
	return RemoveData(name, B_MESSAGE_TYPE, index);
}


bool
BMessage::RemoveMessenger(const char *name, b_int32 index)
{
	return RemoveData(name, B_MESSENGER_TYPE, index);
}


bool
BMessage::ReplaceData(const char *name, b_type_code type, b_int32 index, const void *data, size_t numBytes, bool is_fixed_size)
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
BMessage::ReplaceData(const char *name, b_type_code type, const void *data, size_t numBytes, bool is_fixed_size)
{
	return ReplaceData(name, type, 0, data, numBytes, is_fixed_size);
}


bool
BMessage::ReplacString(const char *name, b_int32 index, const char *aString)
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
BMessage::ReplacString(const char *name, b_int32 index, const BString &aString)
{
	return ReplacString(name, index, aString.String());
}


bool
BMessage::ReplacString(const char *name, const BString &aString)
{
	return ReplacString(name, 0, aString);
}


bool
BMessage::ReplaceInt8(const char *name, b_int32 index, b_int8 val)
{
	if(!name) return false;
	return ReplaceData(name, B_INT8_TYPE, index, (const void*)&val, sizeof(b_int8), true);
}


bool
BMessage::ReplaceInt8(const char *name, b_int8 val)
{
	return ReplaceInt8(name, 0, val);
}


bool
BMessage::ReplaceInt16(const char *name, b_int32 index, b_int16 val)
{
	if(!name) return false;
	return ReplaceData(name, B_INT16_TYPE, index, (const void*)&val, sizeof(b_int16), true);
}


bool
BMessage::ReplaceInt16(const char *name, b_int16 val)
{
	return ReplaceInt16(name, 0, val);
}


bool
BMessage::ReplaceInt32(const char *name, b_int32 index, b_int32 val)
{
	if(!name) return false;
	return ReplaceData(name, B_INT32_TYPE, index, (const void*)&val, sizeof(b_int32), true);
}


bool
BMessage::ReplaceInt32(const char *name, b_int32 val)
{
	return ReplaceInt32(name, 0, val);
}


bool
BMessage::ReplaceInt64(const char *name, b_int32 index, b_int64 val)
{
	if(!name) return false;
	return ReplaceData(name, B_INT64_TYPE, index, (const void*)&val, sizeof(b_int64), true);
}


bool
BMessage::ReplaceInt64(const char *name, b_int64 val)
{
	return ReplaceInt64(name, 0, val);
}


bool
BMessage::ReplaceBool(const char *name, b_int32 index, bool aBoolean)
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
BMessage::ReplaceFloat(const char *name, b_int32 index, float f)
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
BMessage::ReplaceDouble(const char *name, b_int32 index, double d)
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
BMessage::ReplacePoint(const char *name, b_int32 index, BPoint pt)
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
BMessage::ReplaceRect(const char *name, b_int32 index, BRect r)
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
BMessage::ReplacePointer(const char *name, b_int32 index, const void *ptr)
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
BMessage::ReplaceMessage(const char *name, b_int32 index, const BMessage *msg)
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
BMessage::ReplaceMessenger(const char *name, b_int32 index, const BMessenger *msgr)
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
BMessage::ReplaceMessenger(const char *name, b_int32 index, const BMessenger &msgr)
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
	void *tmpPort = bhapi_open_port_by_source(fSource);
	if(tmpPort == NULL) return false;
	bhapi_delete_port(tmpPort);
	return true;
}


b_status_t
BMessage::SendReply(b_uint32 command, BHandler *replyHandler) const
{
	BMessage msg(command);
	return SendReply(&msg, replyHandler, B_INFINITE_TIMEOUT);
}


b_status_t
BMessage::SendReply(const BMessage *message, BHandler *replyHandler, b_bigtime_t sendTimeout) const
{
	b_status_t retVal = B_BAD_VALUE;

	if(message != NULL)
	{
		b_uint64 replyToken = B_MAXUINT64;
		b_bigtime_t replyTokenTimeStamp = B_MAXINT64;

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
				bhapi_delete_port(msg.fSource);
				msg.fSource = NULL;
			}

			retVal = (bhapi_port_count(fSource) > 0 ?
				    B_DUPLICATE_REPLY : BMessenger::_SendMessageToPort(fSource, message, B_TIMEOUT, sendTimeout));
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

