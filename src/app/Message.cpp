/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
 *
 * ETK++ library is a freeware; it may be used and distributed according to
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

#include "./../private/Token.h"
#include "./../support/StreamIO.h"

#include "Message.h"
#include "Messenger.h"
#include "Handler.h"


EMessage::EMessage()
	: what(0),
	  fTargetToken(E_MAXUINT64), fTargetTokenTimestamp(E_INT64_CONSTANT(0)),
	  fReplyToken(E_MAXUINT64), fReplyTokenTimestamp(E_INT64_CONSTANT(0)),
	  fNoticeSource(false), fSource(NULL), fIsReply(false)
{
	fTeam = etk_get_current_team_id();
}


EMessage::EMessage(euint32 a_what)
	: fTargetToken(E_MAXUINT64), fTargetTokenTimestamp(E_INT64_CONSTANT(0)),
	  fReplyToken(E_MAXUINT64), fReplyTokenTimestamp(E_INT64_CONSTANT(0)),
	  fNoticeSource(false), fSource(NULL), fIsReply(false)
{
    EMessage::what = a_what;
	fTeam = etk_get_current_team_id();
}


EMessage::EMessage(const EMessage &msg)
	: what(0), fTeam(E_INT64_CONSTANT(0)),
	  fTargetToken(E_MAXUINT64), fTargetTokenTimestamp(E_INT64_CONSTANT(0)),
	  fReplyToken(E_MAXUINT64), fReplyTokenTimestamp(E_INT64_CONSTANT(0)),
	  fNoticeSource(false), fSource(NULL), fIsReply(false)
{
	operator=(msg);
}


EMessage&
EMessage::operator=(const EMessage &msg)
{
	what = msg.what;

	MakeEmpty();

	for(eint32 k = 0; k < msg.fObjectsList.CountItems(); k++)
	{
		list_data *ldata = (list_data*)msg.fObjectsList.ItemAt(k);
		if(!ldata) continue;

		for(eint32 j = 0; j < ldata->list.CountItems(); j++)
		{
			type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(j);
			if(!tldata) continue;

			for(eint32 i = 0; i < tldata->list.CountItems(); i++)
			{
				_object_t *Object = (_object_t*)tldata->list.ItemAt(i);
				if(!Object) continue;

				AddData(ldata->name, tldata->type, Object->data, Object->bytes, Object->fixed_size);
			}
		}
	}

	if(fSource != NULL)
	{
		if(fNoticeSource) etk_close_port(fSource);
		etk_delete_port(fSource);
		fSource = NULL;
	}

	fTargetToken = E_MAXUINT64;
	fTargetTokenTimestamp = E_MAXINT64;
	fReplyToken = E_MAXUINT64;
	fReplyTokenTimestamp = E_MAXINT64;

	if(msg.fTeam == etk_get_current_team_id())
	{
		fTargetToken = msg.fTargetToken;
		fTargetTokenTimestamp = msg.fTargetTokenTimestamp;
	}

	if(msg.fSource == NULL)
	{
		fReplyToken = msg.fReplyToken;
		fReplyTokenTimestamp = msg.fReplyTokenTimestamp;
	}
	else if(msg.fTeam == etk_get_current_team_id())
	{
		fSource = etk_open_port_by_source(msg.fSource);
	}

	fTeam = msg.fTeam;
	fIsReply = msg.fIsReply;
	fNoticeSource = false;

	return *this;
}


size_t
EMessage::FlattenedSize() const
{
	size_t size = sizeof(size_t) + sizeof(euint32) + sizeof(euint64); // FlattenSize + msg->what + recordCount

	size += sizeof(eint64); // fTeam
	size += sizeof(euint64) + sizeof(e_bigtime_t); // fTargetToken + fTargetTokenTimestamp
	size += sizeof(euint64) + sizeof(e_bigtime_t); // fReplyToken + fReplyTokenTimestamp
	size += sizeof(e_address_t); // fSource
	size += sizeof(bool); // fIsReply

	for(eint32 k = 0; k < fObjectsList.CountItems(); k++)
	{
		list_data *ldata = (list_data*)fObjectsList.ItemAt(k);
		if(!ldata) continue;

		size_t nameLen = (size_t)(ldata->name ? strlen(ldata->name) : 0);

		for(eint32 j = 0; j < ldata->list.CountItems(); j++)
		{
			type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(j);
			if(!tldata) continue;

			for(eint32 i = 0; i < tldata->list.CountItems(); i++)
			{
				_object_t *Object = (_object_t*)tldata->list.ItemAt(i);
				if(!Object) continue;

				// msg->_object_t
				size_t objectSize = sizeof(size_t) + nameLen + sizeof(e_type_code) + sizeof(bool) + sizeof(size_t);
				size_t dataLen = Object->fixed_size ? Object->bytes : sizeof(e_address_t);
				objectSize += dataLen;

				size += objectSize;
			}
		}
	}

	return size;
}


bool
EMessage::Flatten(char *buffer, size_t bufferSize) const
{
	if(buffer == NULL ||
	   bufferSize < sizeof(size_t) + sizeof(euint32) + sizeof(euint64) +
			sizeof(eint64) + 2 * (sizeof(euint64) + sizeof(e_bigtime_t)) + sizeof(e_address_t) + sizeof(bool)) return false;

	size_t size = 0;

	// msg->what
	size += sizeof(size_t) + sizeof(euint32) + sizeof(euint64) +
		sizeof(eint64) + 2 * (sizeof(euint64) + sizeof(e_bigtime_t)) + sizeof(e_address_t) + sizeof(bool);
	char *dst = buffer;
	dst += sizeof(size_t);
	memcpy(dst, &what, sizeof(euint32)); dst += sizeof(euint32);

	// fTeam
	memcpy(dst, &fTeam, sizeof(eint64)); dst += sizeof(eint64);

	// fTargetToken + fTargetTokenTimestamp
	memcpy(dst, &fTargetToken, sizeof(euint64)); dst += sizeof(euint64);
	memcpy(dst, &fTargetTokenTimestamp, sizeof(e_bigtime_t)); dst += sizeof(e_bigtime_t);

	// fReplyToken + fReplyTokenTimestamp
	memcpy(dst, &fReplyToken, sizeof(euint64)); dst += sizeof(euint64);
	memcpy(dst, &fReplyTokenTimestamp, sizeof(e_bigtime_t)); dst += sizeof(e_bigtime_t);

	// fSource
	e_address_t source_address = reinterpret_cast<e_address_t>(fSource);
	memcpy(dst, &source_address, sizeof(e_address_t)); dst += sizeof(e_address_t);

	// fIsReply
	memcpy(dst, &fIsReply, sizeof(bool)); dst += sizeof(bool);

	// recordCount
	char *countDst = dst; dst += sizeof(euint64);
	euint64 count = (euint64)E_INT64_CONSTANT(0);

	for(eint32 k = 0; k < fObjectsList.CountItems(); k++)
	{
		list_data *ldata = (list_data*)fObjectsList.ItemAt(k);
		if(!ldata) continue;

		size_t nameLen = (size_t)(ldata->name ? strlen(ldata->name) : 0);

		for(eint32 j = 0; j < ldata->list.CountItems(); j++)
		{
			type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(j);
			if(!tldata) continue;

			for(eint32 i = 0; i < tldata->list.CountItems(); i++)
			{
				_object_t *Object = (_object_t*)tldata->list.ItemAt(i);
				if(!Object) continue;

				// msg->_object_t
				size_t dataLen = Object->fixed_size ? Object->bytes : sizeof(e_address_t);
				size_t objectSize = sizeof(size_t) + nameLen + sizeof(e_type_code) + sizeof(bool) + sizeof(size_t) + dataLen;
				size += objectSize;

				if(bufferSize < size) return false;

				memcpy(dst, &nameLen, sizeof(size_t)); dst += sizeof(size_t);
				if(nameLen > 0) {memcpy(dst, ldata->name, nameLen); dst += nameLen;}
				memcpy(dst, &(tldata->type), sizeof(e_type_code)); dst += sizeof(e_type_code);
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
						e_address_t address = reinterpret_cast<e_address_t>(Object->data);
						memcpy(dst, &address, dataLen);
					}

					dst += dataLen;
				}

				count++;
			}
		}
	}

	// recordCount
	memcpy(countDst, &count, sizeof(euint64));

	memcpy(buffer, &size, sizeof(size_t));

	return true;
}


bool
EMessage::Unflatten(const char *buffer, size_t bufferSize)
{
	if(buffer == NULL ||
	   bufferSize < sizeof(size_t) + sizeof(euint32) + sizeof(euint64) +
			sizeof(eint64) + 2 * (sizeof(euint64) + sizeof(e_bigtime_t)) + sizeof(e_address_t) + sizeof(bool)) return false;

	const char *src = buffer;
	EMessage msg;
	euint64 recordCount = 0;
	size_t _bufferSize = 0;

	memcpy(&_bufferSize, src, sizeof(size_t));
	if(bufferSize < _bufferSize) return false;
	src += sizeof(size_t); bufferSize -= sizeof(size_t);

	// msg->what
	memcpy(&msg.what, src, sizeof(euint32)); src += sizeof(euint32); bufferSize -= sizeof(euint32);

	// fTeam
	memcpy(&msg.fTeam, src, sizeof(eint64)); src += sizeof(eint64); bufferSize -= sizeof(eint64);

	// fTargetToken + fTargetTokenTimestamp
	memcpy(&msg.fTargetToken, src, sizeof(euint64)); src += sizeof(euint64); bufferSize -= sizeof(euint64);
	memcpy(&msg.fTargetTokenTimestamp, src, sizeof(e_bigtime_t)); src += sizeof(e_bigtime_t); bufferSize -= sizeof(e_bigtime_t);

	// fReplyToken + fReplyTokenTimestamp
	memcpy(&msg.fReplyToken, src, sizeof(euint64)); src += sizeof(euint64); bufferSize -= sizeof(euint64);
	memcpy(&msg.fReplyTokenTimestamp, src, sizeof(e_bigtime_t)); src += sizeof(e_bigtime_t); bufferSize -= sizeof(e_bigtime_t);

	// fSource
	e_address_t source_address;
	memcpy(&source_address, src, sizeof(e_address_t)); src += sizeof(e_address_t); bufferSize -= sizeof(e_address_t);

	// fIsReply
	memcpy(&msg.fIsReply, src, sizeof(bool)); src += sizeof(bool); bufferSize -= sizeof(bool);

	// recordCount
	memcpy(&recordCount, src, sizeof(euint64)); src += sizeof(euint64); bufferSize -= sizeof(euint64);

	for(euint64 i = (euint64)E_INT64_CONSTANT(0); i < recordCount; i++)
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
		if(bufferSize < sizeof(e_type_code)) {if(name) free(name); return false;}
		e_type_code type;
		memcpy(&type, src, sizeof(e_type_code)); src += sizeof(e_type_code); bufferSize -= sizeof(e_type_code);

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
				size_t dataLen = sizeof(e_address_t);
				if(bytes != dataLen || bufferSize < dataLen) {if(name) free(name); return false;}

				e_address_t address = 0;
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
		if(fNoticeSource) etk_close_port(fSource);
		etk_delete_port(fSource);
		fSource = NULL;
	}

	fTargetToken = E_MAXUINT64;
	fTargetTokenTimestamp = E_MAXINT64;
	fReplyToken = E_MAXUINT64;
	fReplyTokenTimestamp = E_MAXINT64;

	if(msg.fTeam == etk_get_current_team_id())
	{
		fTargetToken = msg.fTargetToken;
		fTargetTokenTimestamp = msg.fTargetTokenTimestamp;
	}

	void *source = (msg.fTeam != etk_get_current_team_id() ? (void*)NULL : reinterpret_cast<void*>(source_address));
	if(source == NULL)
	{
		if(msg.fTeam == etk_get_current_team_id())
		{
			fReplyToken = msg.fReplyToken;
			fReplyTokenTimestamp = msg.fReplyTokenTimestamp;
		}
	}
	else
	{
		// TODO: not safe
#if 0
		fSource = etk_open_port_by_source(source);
#endif
	}

	fTeam = msg.fTeam;
	fIsReply = msg.fIsReply;
	fNoticeSource = false;

	return true;
}


bool
EMessage::IsSystem() const
{
#ifdef ETK_BIG_ENDIAN
	return((what & 0xff) == '_');
#else
	return(((what >> 24) & 0xff) == '_');
#endif
}


void
EMessage::PrintToStream(EStreamIO &stream) const
{
	// TODO
}


void
EMessage::PrintToStream() const
{
	ETK_OUTPUT("what = '%c%c%c%c'\t\tteam = %I64i\n",
#ifdef ETK_BIG_ENDIAN
		   what & 0xff, (what >> 8) & 0xff, (what >> 16) & 0xff, (what >> 24) & 0xff,
#else
		   (what >> 24) & 0xff, (what >> 16) & 0xff, (what >> 8) & 0xff, what & 0xff,
#endif
		   fTeam);
	if(fTargetToken != E_MAXUINT64)
		ETK_OUTPUT("Target token: %I64u\t\t", fTargetToken);
	else
		ETK_OUTPUT("No Target token\t\t");
	if(fReplyToken != E_MAXUINT64)
		ETK_OUTPUT("Reply token: %I64u\n", fReplyToken);
	else
		ETK_OUTPUT("No Reply token\n");
	ETK_OUTPUT("%s\t\t%s\n", (fIsReply ? "Reply message" : "Not reply message"), (fSource ? "Has source" : "No source"));

	for(eint32 k = 0; k < fObjectsList.CountItems(); k++)
	{
		list_data *ldata = (list_data*)fObjectsList.ItemAt(k);
		if(!ldata)
		{
			ETK_WARNING("Message::(%I32i) is NULL!", k);
			continue;
		}

		const char *name = ldata->name ? (strlen(ldata->name) > 0 ? (const char*)ldata->name : "NULL") : "ERROR-NULL";
		euint64 count = (euint64)E_INT64_CONSTANT(0);

		for(eint32 j = 0; j < ldata->list.CountItems(); j++)
		{
			type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(j);
			if(!tldata)
			{
				ETK_WARNING("Message::(%I32i - %I32i) is NULL!", k, j);
				continue;
			}

			for(eint32 i = 0; i < tldata->list.CountItems(); i++)
			{
				_object_t *Object = (_object_t*)tldata->list.ItemAt(i);

				if(!Object)
				{
					ETK_WARNING("Message::(%I32i - %I32i - %I32i) is NULL!",
						   k, j, i);
					continue;
				}
				else
				{
					count++;
					ETK_OUTPUT("%s[%I64u]:", name, count);
					if(Object->data == NULL)
					{
						ETK_OUTPUT("\tWARNING: *** NO DATA ***\n");
					}
					else
					{
						switch(tldata->type)
						{
							case E_STRING_TYPE:
								ETK_OUTPUT("\tSTRING\t\"%s\"\n", (char*)Object->data);
								break;

							case E_INT8_TYPE:
								ETK_OUTPUT("\tINT8\t%I8i\n", *((eint8*)Object->data));
								break;

							case E_INT16_TYPE:
								ETK_OUTPUT("\tINT16\t%I16i\n", *((eint16*)Object->data));
								break;

							case E_INT32_TYPE:
								ETK_OUTPUT("\tINT32\t%I32i\n", *((eint32*)Object->data));
								break;

							case E_INT64_TYPE:
								ETK_OUTPUT("\tINT64\t%I64i\n", *((eint64*)Object->data));
								break;

							case E_BOOL_TYPE:
								ETK_OUTPUT("\tBOOL\t%s\n", (*((bool*)Object->data) ? "true" : "false"));
								break;

							case E_FLOAT_TYPE:
								ETK_OUTPUT("\tFLOAT\t%g\n", *((float*)Object->data));
								break;

							case E_DOUBLE_TYPE:
								ETK_OUTPUT("\tDOUBLE\t%g\n", *((double*)Object->data));
								break;

							case E_POINT_TYPE:
								{
									struct point_t {
										float x;
										float y;
									} *pt;

									pt = (struct point_t *)Object->data;

									ETK_OUTPUT("\tPOINT\t(%g,%g)\n", pt->x, pt->y);
								}
								break;

							case E_RECT_TYPE:
								{
									struct rect_t {
										float l;
										float t;
										float r;
										float b;
									} *r;

									r = (struct rect_t *)Object->data;

									ETK_OUTPUT("\tRECT\t(%g,%g,%g,%g)\n", r->l, r->t, r->r, r->b);
								}
								break;

							default:
								ETK_OUTPUT("\t'%c%c%c%c'\tbytes[%lu]  fixed_size[%s]  address[%p]\n",
#ifdef ETK_BIG_ENDIAN
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


EMessage::~EMessage()
{
	MakeEmpty();

	if(fSource != NULL)
	{
		if(fNoticeSource) etk_close_port(fSource);
		etk_delete_port(fSource);
	}
}


EMessage::list_data*
EMessage::_find_list(const EList *list, const char *name, eint32 *index)
{
	if(!list || !name) return NULL;

	for(eint32 i = 0; i < list->CountItems(); i++)
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


EMessage::type_list_data*
EMessage::_find_type_list(const EList *list, e_type_code type)
{
	if(!list) return NULL;

	for(eint32 i = 0; i < list->CountItems(); i++)
	{
		type_list_data *data = (type_list_data*)list->ItemAt(i);
		if(!data) continue;
		if(data->type == type) return data;
	}

	return NULL;
}


eint32
EMessage::CountItems(const char *name, e_type_code type) const
{
	if(!name) return -1;

	list_data *ldata = _find_list(&fObjectsList, name);
	if(!ldata) return -1;

	type_list_data *tldata = _find_type_list(&(ldata->list), type);
	if(!tldata) return -1;

	return tldata->list.CountItems();
}


eint32
EMessage::CountItems(eint32 nameIndex, eint32 typeIndex, e_type_code *type) const
{
	list_data *ldata = (list_data*)fObjectsList.ItemAt(nameIndex);
	if(!ldata) return -1;

	type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(typeIndex);
	if(!tldata) return -1;

	if(type) *type = tldata->type;
	return tldata->list.CountItems();
}


bool
EMessage::TypeAt(const char *name, eint32 typeIndex, e_type_code *type) const
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
EMessage::TypeAt(eint32 nameIndex, eint32 typeIndex, e_type_code *type) const
{
	if(!type) return false;

	list_data *ldata = (list_data*)fObjectsList.ItemAt(nameIndex);
	if(!ldata) return false;

	type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(typeIndex);
	if(!tldata) return false;

	*type = tldata->type;

	return true;
}


eint32
EMessage::CountTypesByName(const char *name) const
{
	if(!name) return -1;

	list_data *ldata = _find_list(&fObjectsList, name);
	if(!ldata) return -1;

	return ldata->list.CountItems();
}


eint32
EMessage::CountTypesByName(eint32 nameIndex) const
{
	list_data *ldata = (list_data*)fObjectsList.ItemAt(nameIndex);
	if(!ldata) return -1;

	return ldata->list.CountItems();
}


eint32
EMessage::CountNames(e_type_code type, bool count_all_names_when_any_type) const
{
	if(type == E_ANY_TYPE && count_all_names_when_any_type) return fObjectsList.CountItems();

	eint32 retVal = 0;

	for(eint32 i = 0; i < fObjectsList.CountItems(); i++)
	{
		list_data *ldata = (list_data*)fObjectsList.ItemAt(i);
		if(ldata == NULL) continue;

		for(eint32 k = 0; k < ldata->list.CountItems(); k++)
		{
			type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(i);
			if(tldata == NULL) continue;
			if(tldata->type == type) {retVal++; break;}
		}
	}

	return retVal;
}


eint32
EMessage::FindName(const char *name) const
{
	eint32 index = -1;
	_find_list(&fObjectsList, name, &index);
	return index;
}


const char*
EMessage::NameAt(eint32 nameIndex) const
{
	list_data *data = (list_data*)fObjectsList.ItemAt(nameIndex);
	return(data ? data->name : NULL);
}


void
EMessage::MakeEmpty()
{
	for(eint32 k = 0; k < fObjectsList.CountItems(); k++)
	{
		list_data *ldata = (list_data*)fObjectsList.ItemAt(k);
		if(!ldata) continue;

		for(eint32 j = 0; j < ldata->list.CountItems(); j++)
		{
			type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(j);
			if(!tldata) continue;

			for(eint32 i = 0; i < tldata->list.CountItems(); i++)
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
EMessage::IsEmpty() const
{
	return fObjectsList.IsEmpty();
}


bool
EMessage::Rename(const char *old_entry, const char *new_entry)
{
	if(!old_entry || !new_entry) return false;
	if(strcmp(old_entry, new_entry) == 0 && strlen(old_entry) == strlen(new_entry)) return true;

	list_data *ldata = _find_list(&fObjectsList, new_entry);
	if(ldata) return false;

	ldata = _find_list(&fObjectsList, old_entry);
	if(!ldata) return false;

	char *newName = EStrdup(new_entry);
	if(!newName) return false;

	delete[] ldata->name;
	ldata->name = newName;

	return true;
}


bool
EMessage::AddData(const char *name, e_type_code type, const void *data, size_t numBytes, bool is_fixed_size)
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
			if((ldata->name = EStrdup(name)) != NULL)
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
EMessage::AddString(const char *name, const char *aString)
{
	if(!name || !aString) return false;

	return AddData(name, E_STRING_TYPE, (const void*)aString, (size_t)(strlen(aString) + 1), true);
}


bool
EMessage::AddString(const char *name, const EString &aString)
{
	return AddString(name, aString.String());
}


bool
EMessage::AddInt8(const char *name, eint8 val)
{
	if(!name) return false;
	return AddData(name, E_INT8_TYPE, (const void*)&val, sizeof(eint8), true);
}


bool
EMessage::AddInt16(const char *name, eint16 val)
{
	if(!name) return false;
	return AddData(name, E_INT16_TYPE, (const void*)&val, sizeof(eint16), true);
}


bool
EMessage::AddInt32(const char *name, eint32 val)
{
	if(!name) return false;
	return AddData(name, E_INT32_TYPE, (const void*)&val, sizeof(eint32), true);
}


bool
EMessage::AddInt64(const char *name, eint64 val)
{
	if(!name) return false;
	return AddData(name, E_INT64_TYPE, (const void*)&val, sizeof(eint64), true);
}


bool
EMessage::AddBool(const char *name, bool aBoolean)
{
	if(!name) return false;
	return AddData(name, E_BOOL_TYPE, (const void*)&aBoolean, sizeof(bool), true);
}


bool
EMessage::AddFloat(const char *name, float aFloat)
{
	if(!name) return false;
	return AddData(name, E_FLOAT_TYPE, (const void*)&aFloat, sizeof(float), true);
}


bool
EMessage::AddDouble(const char *name, double aDouble)
{
	if(!name) return false;
	return AddData(name, E_DOUBLE_TYPE, (const void*)&aDouble, sizeof(double), true);
}


bool
EMessage::AddPoint(const char *name, EPoint pt)
{
	if(!name) return false;

	struct point_t {
		float x;
		float y;
	} apt;

	apt.x = pt.x;
	apt.y = pt.y;

	return AddData(name, E_POINT_TYPE, (const void*)&apt, sizeof(struct point_t), true);
}


bool
EMessage::AddRect(const char *name, ERect r)
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

	return AddData(name, E_RECT_TYPE, (const void*)&ar, sizeof(struct rect_t), true);
}


bool
EMessage::AddPointer(const char *name, const void *ptr)
{
	if(!name || !ptr) return false;
	return AddData(name, E_POINTER_TYPE, ptr, 0, false);
}


bool
EMessage::AddMessage(const char *name, const EMessage *msg)
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

	bool retval = AddData(name, E_MESSAGE_TYPE, buffer, flattenedSize, true);

	free(buffer);

	return retval;
}


bool
EMessage::AddMessenger(const char *name, const EMessenger *msgr)
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

	bool retval = AddData(name, E_MESSENGER_TYPE, buffer, flattenedSize + sizeof(size_t), true);

	free(buffer);

	return retval;
}


bool
EMessage::AddMessenger(const char *name, const EMessenger &msgr)
{
	return AddMessenger(name, &msgr);
}


bool
EMessage::FindData(const char *name, e_type_code type, const void **data, ssize_t *numBytes) const
{
	return FindData(name, type, 0, data, numBytes);
}


bool
EMessage::FindData(const char *name, e_type_code type, eint32 index, const void **data, ssize_t *numBytes) const
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
EMessage::FindData(eint32 nameIndex, eint32 typeIndex, eint32 index, const void **data, ssize_t *numBytes) const
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
EMessage::FindString(const char *name, const char **str) const
{
	return FindString(name, 0, str);
}


bool
EMessage::FindString(const char *name, eint32 index, const char **str) const
{
	return FindData(name, E_STRING_TYPE, index, (const void**)str, NULL);
}


bool
EMessage::FindString(const char *name, EString *str) const
{
	return FindString(name, 0, str);
}


bool
EMessage::FindString(const char *name, eint32 index, EString *str) const
{
	const char *string = NULL;
	if(!FindData(name, E_STRING_TYPE, index, (const void**)&string, NULL)) return false;
	if(str) str->SetTo(string);
	return true;
}


bool
EMessage::FindInt8(const char *name, eint8 *val) const
{
	return FindInt8(name, 0, val);
}


bool
EMessage::FindInt8(const char *name, eint32 index, eint8 *val) const
{
	const eint8 *value = NULL;
	if(!FindData(name, E_INT8_TYPE, index, (const void**)&value, NULL)) return false;
	if(val) *val = *value;
	return true;
}


bool
EMessage::FindInt16(const char *name, eint16 *val) const
{
	return FindInt16(name, 0, val);
}


bool
EMessage::FindInt16(const char *name, eint32 index, eint16 *val) const
{
	const eint16 *value = NULL;
	if(!FindData(name, E_INT16_TYPE, index, (const void**)&value, NULL)) return false;
	if(val) *val = *value;
	return true;
}


bool
EMessage::FindInt32(const char *name, eint32 *val) const
{
	return FindInt32(name, 0, val);
}


bool
EMessage::FindInt32(const char *name, eint32 index, eint32 *val) const
{
	const eint32 *value = NULL;
	if(!FindData(name, E_INT32_TYPE, index, (const void**)&value, NULL)) return false;
	if(val) *val = *value;
	return true;
}


bool
EMessage::FindInt64(const char *name, eint64 *val) const
{
	return FindInt64(name, 0, val);
}


bool
EMessage::FindInt64(const char *name, eint32 index, eint64 *val) const
{
	const eint64 *value = NULL;
	if(!FindData(name, E_INT64_TYPE, index, (const void**)&value, NULL)) return false;
	if(val) *val = *value;
	return true;
}


bool
EMessage::FindBool(const char *name, bool *aBoolean) const
{
	return FindBool(name, 0, aBoolean);
}


bool
EMessage::FindBool(const char *name, eint32 index, bool *aBoolean) const
{
	const bool *value = NULL;
	if(!FindData(name, E_BOOL_TYPE, index, (const void**)&value, NULL)) return false;
	if(aBoolean) *aBoolean = *value;
	return true;
}


bool
EMessage::FindFloat(const char *name, float *f) const
{
	return FindFloat(name, 0, f);
}


bool
EMessage::FindFloat(const char *name, eint32 index, float *f) const
{
	const float *value = NULL;
	if(!FindData(name, E_FLOAT_TYPE, index, (const void**)&value, NULL)) return false;
	if(f) *f = *value;
	return true;
}


bool
EMessage::FindDouble(const char *name, double *d) const
{
	return FindDouble(name, 0, d);
}


bool
EMessage::FindDouble(const char *name, eint32 index, double *d) const
{
	const double *value = NULL;
	if(!FindData(name, E_DOUBLE_TYPE, index, (const void**)&value, NULL)) return false;
	if(d) *d = *value;
	return true;
}


bool
EMessage::FindPoint(const char *name, EPoint *pt) const
{
	return FindPoint(name, 0, pt);
}


bool
EMessage::FindPoint(const char *name, eint32 index, EPoint *pt) const
{
	struct point_t {
		float x;
		float y;
	};

	const struct point_t *apt = NULL;

	if(!FindData(name, E_POINT_TYPE, index, (const void**)&apt, NULL)) return false;
	if(pt) pt->Set(apt->x, apt->y);
	return true;
}


bool
EMessage::FindRect(const char *name, ERect *r) const
{
	return FindRect(name, 0, r);
}


bool
EMessage::FindRect(const char *name, eint32 index, ERect *r) const
{
	struct rect_t {
		float left;
		float top;
		float right;
		float bottom;
	};

	const struct rect_t *ar = NULL;

	if(!FindData(name, E_RECT_TYPE, index, (const void**)&ar, NULL)) return false;
	if(r) r->Set(ar->left, ar->top, ar->right, ar->bottom);
	return true;
}


bool
EMessage::FindPointer(const char *name, void **ptr) const
{
	return FindPointer(name, 0, ptr);
}


bool
EMessage::FindPointer(const char *name, eint32 index, void **ptr) const
{
	return FindData(name, E_POINTER_TYPE, index, (const void**)ptr, NULL);
}


bool
EMessage::FindMessage(const char *name, EMessage *msg) const
{
	return FindMessage(name, 0, msg);
}


bool
EMessage::FindMessage(const char *name, eint32 index, EMessage *msg) const
{
	const char *buffer = NULL;
	if(!FindData(name, E_MESSAGE_TYPE, index, (const void**)&buffer, NULL)) return false;

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
EMessage::FindMessenger(const char *name, EMessenger *msgr) const
{
	return FindMessenger(name, 0, msgr);
}


bool
EMessage::FindMessenger(const char *name, eint32 index, EMessenger *msgr) const
{
	const char *buffer = NULL;
	if(!FindData(name, E_MESSENGER_TYPE, index, (const void**)&buffer, NULL)) return false;

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
EMessage::HasData(const char *name, e_type_code type, eint32 index) const
{
	return FindData(name, type, index, NULL, NULL);
}


bool
EMessage::HasString(const char *name, eint32 index) const
{
	return FindString(name, index, (const char**)NULL);
}


bool
EMessage::HasInt8(const char *name, eint32 index) const
{
	return FindInt8(name, index, NULL);
}


bool
EMessage::HasInt16(const char *name, eint32 index) const
{
	return FindInt16(name, index, NULL);
}


bool
EMessage::HasInt32(const char *name, eint32 index) const
{
	return FindInt32(name, index, NULL);
}


bool
EMessage::HasInt64(const char *name, eint32 index) const
{
	return FindInt64(name, index, NULL);
}


bool
EMessage::HasBool(const char *name, eint32 index) const
{
	return FindBool(name, index, NULL);
}


bool
EMessage::HasFloat(const char *name, eint32 index) const
{
	return FindFloat(name, index, NULL);
}


bool
EMessage::HasDouble(const char *name, eint32 index) const
{
	return FindDouble(name, index, NULL);
}


bool
EMessage::HasPoint(const char *name, eint32 index) const
{
	return FindPoint(name, index, NULL);
}


bool
EMessage::HasRect(const char *name, eint32 index) const
{
	return FindRect(name, index, NULL);
}


bool
EMessage::HasPointer(const char *name, eint32 index) const
{
	return FindPointer(name, index, NULL);
}


bool
EMessage::HasMessage(const char *name, eint32 index) const
{
	return FindMessage(name, index, NULL);
}


bool
EMessage::HasMessenger(const char *name, eint32 index) const
{
	return FindMessenger(name, index, NULL);
}


bool
EMessage::RemoveData(const char *name, e_type_code type, eint32 index)
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
EMessage::RemoveData(const char *name, e_type_code type)
{
	if(!name) return false;

	list_data *ldata = _find_list(&fObjectsList, name);
	if(!ldata) return false;

	type_list_data *tldata = _find_type_list(&(ldata->list), type);
	if(!tldata) return false;

	for(eint32 i = 0; i < tldata->list.CountItems(); i++)
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
EMessage::RemoveData(const char *name)
{
	if(!name) return false;

	list_data *ldata = _find_list(&fObjectsList, name);
	if(!ldata) return false;

	for(eint32 j = 0; j < ldata->list.CountItems(); j++)
	{
		type_list_data *tldata = (type_list_data*)ldata->list.ItemAt(j);
		if(!tldata) continue;

		for(eint32 i = 0; i < tldata->list.CountItems(); i++)
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
EMessage::RemoveString(const char *name, eint32 index)
{
	return RemoveData(name, E_STRING_TYPE, index);
}


bool
EMessage::RemoveInt8(const char *name, eint32 index)
{
	return RemoveData(name, E_INT8_TYPE, index);
}


bool
EMessage::RemoveInt16(const char *name, eint32 index)
{
	return RemoveData(name, E_INT16_TYPE, index);
}


bool
EMessage::RemoveInt32(const char *name, eint32 index)
{
	return RemoveData(name, E_INT32_TYPE, index);
}


bool
EMessage::RemoveInt64(const char *name, eint32 index)
{
	return RemoveData(name, E_INT64_TYPE, index);
}


bool
EMessage::RemoveBool(const char *name, eint32 index)
{
	return RemoveData(name, E_BOOL_TYPE, index);
}


bool
EMessage::RemoveFloat(const char *name, eint32 index)
{
	return RemoveData(name, E_FLOAT_TYPE, index);
}


bool
EMessage::RemoveDouble(const char *name, eint32 index)
{
	return RemoveData(name, E_DOUBLE_TYPE, index);
}


bool
EMessage::RemovePoint(const char *name, eint32 index)
{
	return RemoveData(name, E_POINT_TYPE, index);
}


bool
EMessage::RemoveRect(const char *name, eint32 index)
{
	return RemoveData(name, E_RECT_TYPE, index);
}


bool
EMessage::RemovePointer(const char *name, eint32 index)
{
	return RemoveData(name, E_POINTER_TYPE, index);
}


bool
EMessage::RemoveMessage(const char *name, eint32 index)
{
	return RemoveData(name, E_MESSAGE_TYPE, index);
}


bool
EMessage::RemoveMessenger(const char *name, eint32 index)
{
	return RemoveData(name, E_MESSENGER_TYPE, index);
}


bool
EMessage::ReplaceData(const char *name, e_type_code type, eint32 index, const void *data, size_t numBytes, bool is_fixed_size)
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
EMessage::ReplaceData(const char *name, e_type_code type, const void *data, size_t numBytes, bool is_fixed_size)
{
	return ReplaceData(name, type, 0, data, numBytes, is_fixed_size);
}


bool
EMessage::ReplaceString(const char *name, eint32 index, const char *aString)
{
	if(!name || !aString) return false;
	return ReplaceData(name, E_STRING_TYPE, index, (const void*)aString, (size_t)(strlen(aString) + 1), true);
}


bool
EMessage::ReplaceString(const char *name, const char *aString)
{
	return ReplaceString(name, 0, aString);
}


bool
EMessage::ReplaceString(const char *name, eint32 index, const EString &aString)
{
	return ReplaceString(name, index, aString.String());
}


bool
EMessage::ReplaceString(const char *name, const EString &aString)
{
	return ReplaceString(name, 0, aString);
}


bool
EMessage::ReplaceInt8(const char *name, eint32 index, eint8 val)
{
	if(!name) return false;
	return ReplaceData(name, E_INT8_TYPE, index, (const void*)&val, sizeof(eint8), true);
}


bool
EMessage::ReplaceInt8(const char *name, eint8 val)
{
	return ReplaceInt8(name, 0, val);
}


bool
EMessage::ReplaceInt16(const char *name, eint32 index, eint16 val)
{
	if(!name) return false;
	return ReplaceData(name, E_INT16_TYPE, index, (const void*)&val, sizeof(eint16), true);
}


bool
EMessage::ReplaceInt16(const char *name, eint16 val)
{
	return ReplaceInt16(name, 0, val);
}


bool
EMessage::ReplaceInt32(const char *name, eint32 index, eint32 val)
{
	if(!name) return false;
	return ReplaceData(name, E_INT32_TYPE, index, (const void*)&val, sizeof(eint32), true);
}


bool
EMessage::ReplaceInt32(const char *name, eint32 val)
{
	return ReplaceInt32(name, 0, val);
}


bool
EMessage::ReplaceInt64(const char *name, eint32 index, eint64 val)
{
	if(!name) return false;
	return ReplaceData(name, E_INT64_TYPE, index, (const void*)&val, sizeof(eint64), true);
}


bool
EMessage::ReplaceInt64(const char *name, eint64 val)
{
	return ReplaceInt64(name, 0, val);
}


bool
EMessage::ReplaceBool(const char *name, eint32 index, bool aBoolean)
{
	if(!name) return false;
	return ReplaceData(name, E_BOOL_TYPE, index, (const void*)&aBoolean, sizeof(bool), true);
}


bool
EMessage::ReplaceBool(const char *name, bool aBoolean)
{
	return ReplaceBool(name, 0, aBoolean);
}


bool
EMessage::ReplaceFloat(const char *name, eint32 index, float f)
{
	if(!name) return false;
	return ReplaceData(name, E_FLOAT_TYPE, index, (const void*)&f, sizeof(float), true);
}


bool
EMessage::ReplaceFloat(const char *name, float f)
{
	return ReplaceFloat(name, 0, f);
}


bool
EMessage::ReplaceDouble(const char *name, eint32 index, double d)
{
	if(!name) return false;
	return ReplaceData(name, E_DOUBLE_TYPE, index, (const void*)&d, sizeof(double), true);
}


bool
EMessage::ReplaceDouble(const char *name, double d)
{
	return ReplaceDouble(name, 0, d);
}


bool
EMessage::ReplacePoint(const char *name, eint32 index, EPoint pt)
{
	if(!name) return false;

	struct point_t {
		float x;
		float y;
	} apt;

	apt.x = pt.x;
	apt.y = pt.y;

	return ReplaceData(name, E_POINT_TYPE, index, (const void*)&apt, sizeof(struct point_t), true);
}


bool
EMessage::ReplacePoint(const char *name, EPoint pt)
{
	return ReplacePoint(name, 0, pt);
}


bool
EMessage::ReplaceRect(const char *name, eint32 index, ERect r)
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

	return ReplaceData(name, E_RECT_TYPE, index, (const void*)&ar, sizeof(struct rect_t), true);
}


bool
EMessage::ReplaceRect(const char *name, ERect r)
{
	return ReplaceRect(name, 0, r);
}


bool
EMessage::ReplacePointer(const char *name, eint32 index, const void *ptr)
{
	if(!name || !ptr) return false;
	return ReplaceData(name, E_POINTER_TYPE, index, ptr, 0, false);
}


bool
EMessage::ReplacePointer(const char *name, const void *ptr)
{
	return ReplacePointer(name, 0, ptr);
}


bool
EMessage::ReplaceMessage(const char *name, eint32 index, const EMessage *msg)
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

	bool retval = ReplaceData(name, E_MESSAGE_TYPE, index, buffer, flattenedSize, true);

	free(buffer);

	return retval;
}


bool
EMessage::ReplaceMessage(const char *name, const EMessage *msg)
{
	return ReplaceMessage(name, 0, msg);
}


bool
EMessage::ReplaceMessenger(const char *name, eint32 index, const EMessenger *msgr)
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

	bool retval = ReplaceData(name, E_MESSENGER_TYPE, index, buffer, flattenedSize + sizeof(size_t), true);

	free(buffer);

	return retval;
}


bool
EMessage::ReplaceMessenger(const char *name, const EMessenger *msgr)
{
	return ReplaceMessenger(name, 0, msgr);
}


bool
EMessage::ReplaceMessenger(const char *name, const EMessenger &msgr)
{
	return ReplaceMessenger(name, 0, &msgr);
}


bool
EMessage::ReplaceMessenger(const char *name, eint32 index, const EMessenger &msgr)
{
	return ReplaceMessenger(name, index, &msgr);
}


bool
EMessage::WasDelivered() const
{
	return(fReplyToken != E_MAXUINT64 || fSource != NULL);
}


bool
EMessage::IsReply() const
{
	return fIsReply;
}


bool
EMessage::IsSourceWaiting() const
{
	if(fSource == NULL) return false;
	void *tmpPort = etk_open_port_by_source(fSource);
	if(tmpPort == NULL) return false;
	etk_delete_port(tmpPort);
	return true;
}


e_status_t
EMessage::SendReply(euint32 command, EHandler *replyHandler) const
{
	EMessage msg(command);
	return SendReply(&msg, replyHandler, E_INFINITE_TIMEOUT);
}


e_status_t
EMessage::SendReply(const EMessage *message, EHandler *replyHandler, e_bigtime_t sendTimeout) const
{
	e_status_t retVal = E_BAD_VALUE;

	if(message != NULL)
	{
		euint64 replyToken = E_MAXUINT64;
		e_bigtime_t replyTokenTimeStamp = E_MAXINT64;

		if(!(replyHandler == NULL || replyHandler->fToken == NULL))
		{
			replyToken = replyHandler->fToken->Token();
			replyTokenTimeStamp = replyHandler->fToken->TimeStamp();
		}

		if(fSource != NULL)
		{
			EMessage msg(*message);

			msg.fIsReply = true;
			msg.fTargetToken = E_MAXUINT64;
			msg.fTargetTokenTimestamp = E_MAXINT64;
			msg.fReplyToken = replyToken;
			msg.fReplyTokenTimestamp = replyTokenTimeStamp;
			if(msg.fSource != NULL)
			{
				etk_delete_port(msg.fSource);
				msg.fSource = NULL;
			}

			retVal = (etk_port_count(fSource) > 0 ?
					E_DUPLICATE_REPLY : EMessenger::_SendMessageToPort(fSource, message, E_TIMEOUT, sendTimeout));
		}
		else
		{
			EMessenger msgr(fTeam, fReplyToken, fReplyTokenTimestamp, &retVal);
			if(retVal == E_OK)
			{
				EMessage msg(*message);

				msg.fIsReply = true;

				retVal = msgr._SendMessage(message, replyToken, sendTimeout);
			}
		}
	}

	return retVal;
}

