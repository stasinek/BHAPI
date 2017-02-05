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
 * File: Message.h
 * Description: Message for communication between loopers/teams
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_MESSAGE_H__
#define __ETK_MESSAGE_H__

#include "./../kernel/OS.h"
#include "./../support/String.h"
#include "./../support/List.h"
#include "./../interface/Point.h"
#include "./../interface/Rect.h"

#ifdef __cplusplus /* Just for C++ */

class EList;
class EMessenger;
class EHandler;
class EStreamIO;

class _IMPEXP_ETK EMessage {
public:
	EMessage();
	EMessage(euint32 what);
	EMessage(const EMessage &msg);
	virtual ~EMessage();

	euint32		what;

	EMessage	&operator=(const EMessage &msg);

	eint32		CountTypesByName(const char *name) const;
	eint32		CountTypesByName(eint32 nameIndex) const;
	bool		TypeAt(const char *name, eint32 typeIndex, e_type_code *type) const;
	bool		TypeAt(eint32 nameIndex, eint32 typeIndex, e_type_code *type) const;

	// CountItems():
	// 	It don't count all items when you pass "E_ANY_TYPE" to "type",
	// 	In EMessage, "E_ANY_TYPE" IS AND JUST a type code!
	eint32		CountItems(const char *name, e_type_code type) const;
	eint32		CountItems(eint32 nameIndex, eint32 typeIndex, e_type_code *type = NULL) const;

	// CountNames():
	// 	Counts all named fields when you pass "E_ANY_TYPE" to first argument and "true" to second argument.
	// 	If you wanna iterate through all it's data, for example:
	// 	...
	// 	for(eint32 i = 0; i < msg->CountNames(E_ANY_TYPE, true); i++)
	// 	{
	// 		const char *name = msg->NameAt(i);
	// 		eint32 typesCount = msg->CountTypesByName(i);
	// 		for(eint32 k = 0; k < typesCount; k++)
	// 		{
	// 			e_type_code type;
	// 			eint32 count = msg->CountItems(i, k, &type);
	// 			for(eint32 m = 0; m < count; m++)
	// 			{
	// 				ssize_t numBytes = 0;
	// 				const void *data = NULL;
	// 				msg->FindData(i, k, m, &data, &numBytes);
	// 				...
	// 			}
	// 		}
	// 	}
	eint32		CountNames(e_type_code type, bool count_all_names_when_any_type = true) const;
	eint32		FindName(const char *name) const;
	const char	*NameAt(eint32 nameIndex) const;

	void		MakeEmpty();
	bool		IsEmpty() const;

	bool		Rename(const char *old_entry, const char *new_entry);

	bool		AddString(const char *name, const char *aString);
	bool		AddString(const char *name, const EString &aString);
	bool		AddInt8(const char *name, eint8 val);
	bool		AddInt16(const char *name, eint16 val);
	bool		AddInt32(const char *name, eint32 val);
	bool		AddInt64(const char *name, eint64 val);
	bool		AddBool(const char *name, bool aBoolean);
	bool		AddFloat(const char *name, float aFloat);
	bool		AddDouble(const char *name, double aDouble);
	bool		AddPoint(const char *name, EPoint pt);
	bool		AddRect(const char *name, ERect r);
	bool		AddPointer(const char *name, const void *ptr);
	bool		AddMessage(const char *name, const EMessage *msg);
	bool		AddMessenger(const char *name, const EMessenger *msgr);
	bool		AddMessenger(const char *name, const EMessenger &msgr);
	bool		AddData(const char *name, e_type_code type, const void *data, size_t numBytes, bool is_fixed_size = true);

	bool		FindString(const char *name, eint32 index, const char **str) const;
	bool		FindString(const char *name, const char **str) const;
	bool		FindString(const char *name, eint32 index, EString *str) const;
	bool		FindString(const char *name, EString *str) const;
	bool		FindInt8(const char *name, eint8 *val) const;
	bool		FindInt8(const char *name, eint32 index, eint8 *val) const;
	bool		FindInt16(const char *name, eint16 *val) const;
	bool		FindInt16(const char *name, eint32 index, eint16 *val) const;
	bool		FindInt32(const char *name, eint32 *val) const;
	bool		FindInt32(const char *name, eint32 index, eint32 *val) const;
	bool		FindInt64(const char *name, eint64 *val) const;
	bool		FindInt64(const char *name, eint32 index, eint64 *val) const;
	bool		FindBool(const char *name, bool *aBoolean) const;
	bool		FindBool(const char *name, eint32 index, bool *aBoolean) const;
	bool		FindFloat(const char *name, float *f) const;
	bool		FindFloat(const char *name, eint32 index, float *f) const;
	bool		FindDouble(const char *name, double *d) const;
	bool		FindDouble(const char *name, eint32 index, double *d) const;
	bool		FindPoint(const char *name, EPoint *pt) const;
	bool		FindPoint(const char *name, eint32 index, EPoint *pt) const;
	bool		FindRect(const char *name, ERect *r) const;
	bool		FindRect(const char *name, eint32 index, ERect *r) const;
	bool		FindPointer(const char *name, void **ptr) const;
	bool		FindPointer(const char *name, eint32 index, void **ptr) const;
	bool		FindMessage(const char *name, EMessage *msg) const;
	bool		FindMessage(const char *name, eint32 index, EMessage *msg) const;
	bool		FindMessenger(const char *name, EMessenger *msgr) const;
	bool		FindMessenger(const char *name, eint32 index, EMessenger *msgr) const;
	bool		FindData(const char *name, e_type_code type, const void **data, ssize_t *numBytes) const;
	bool		FindData(const char *name, e_type_code type, eint32 index, const void **data, ssize_t *numBytes) const;
	bool		FindData(eint32 nameIndex, eint32 typeIndex, eint32 index, const void **data, ssize_t *numBytes) const;

	bool		HasString(const char *name, eint32 index = 0) const;
	bool		HasInt8(const char *name, eint32 index = 0) const;
	bool		HasInt16(const char *name, eint32 index = 0) const;
	bool		HasInt32(const char *name, eint32 index = 0) const;
	bool		HasInt64(const char *name, eint32 index = 0) const;
	bool		HasBool(const char *name, eint32 index = 0) const;
	bool		HasFloat(const char *name, eint32 index = 0) const;
	bool		HasDouble(const char *name, eint32 index = 0) const;
	bool		HasPoint(const char *name, eint32 index = 0) const;
	bool		HasRect(const char *name, eint32 index = 0) const;
	bool		HasPointer(const char *name, eint32 index = 0) const;
	bool		HasMessage(const char *name, eint32 index = 0) const;
	bool		HasMessenger(const char *name, eint32 index = 0) const;
	bool		HasData(const char *name, e_type_code type, eint32 index = 0) const;

	bool		RemoveString(const char *name, eint32 index = 0);
	bool		RemoveInt8(const char *name, eint32 index = 0);
	bool		RemoveInt16(const char *name, eint32 index = 0);
	bool		RemoveInt32(const char *name, eint32 index = 0);
	bool		RemoveInt64(const char *name, eint32 index = 0);
	bool		RemoveBool(const char *name, eint32 index = 0);
	bool		RemoveFloat(const char *name, eint32 index = 0);
	bool		RemoveDouble(const char *name, eint32 index = 0);
	bool		RemovePoint(const char *name, eint32 index = 0);
	bool		RemoveRect(const char *name, eint32 index = 0);
	bool		RemovePointer(const char *name, eint32 index = 0);
	bool		RemoveMessage(const char *name, eint32 index = 0);
	bool		RemoveMessenger(const char *name, eint32 index = 0);
	bool		RemoveData(const char *name, e_type_code type, eint32 index = 0);
	bool		RemoveData(const char *name, e_type_code type);
	bool		RemoveData(const char *name);

	bool		ReplaceString(const char *name, eint32 index, const char *aString);
	bool		ReplaceString(const char *name, const char *aString);
	bool		ReplaceString(const char *name, eint32 index, const EString &aString);
	bool		ReplaceString(const char *name, const EString &aString);
	bool		ReplaceInt8(const char *name, eint8 val);
	bool		ReplaceInt8(const char *name, eint32 index, eint8 val);
	bool		ReplaceInt16(const char *name, eint16 val);
	bool		ReplaceInt16(const char *name, eint32 index, eint16 val);
	bool		ReplaceInt32(const char *name, eint32 val);
	bool		ReplaceInt32(const char *name, eint32 index, eint32 val);
	bool		ReplaceInt64(const char *name, eint64 val);
	bool		ReplaceInt64(const char *name, eint32 index, eint64 val);
	bool		ReplaceBool(const char *name, bool aBoolean);
	bool		ReplaceBool(const char *name, eint32 index, bool aBoolean);
	bool		ReplaceFloat(const char *name, float f);
	bool		ReplaceFloat(const char *name, eint32 index, float f);
	bool		ReplaceDouble(const char *name, double d);
	bool		ReplaceDouble(const char *name, eint32 index, double d);
	bool		ReplacePoint(const char *name, EPoint pt);
	bool		ReplacePoint(const char *name, eint32 index, EPoint pt);
	bool		ReplaceRect(const char *name, ERect r);
	bool		ReplaceRect(const char *name, eint32 index, ERect r);
	bool		ReplacePointer(const char *name, const void *ptr);
	bool		ReplacePointer(const char *name, eint32 index, const void *ptr);
	bool		ReplaceMessage(const char *name, const EMessage *msg);
	bool		ReplaceMessage(const char *name, eint32 index, const EMessage *msg);
	bool		ReplaceMessenger(const char *name, const EMessenger *msgr);
	bool		ReplaceMessenger(const char *name, eint32 index, const EMessenger *msgr);
	bool		ReplaceMessenger(const char *name, const EMessenger &msgr);
	bool		ReplaceMessenger(const char *name, eint32 index, const EMessenger &msgr);
	bool		ReplaceData(const char *name, e_type_code type, const void *data, size_t numBytes, bool is_fixed_size);
	bool		ReplaceData(const char *name, e_type_code type, eint32 index, const void *data, size_t numBytes, bool is_fixed_size);

	bool		IsSystem() const;
	void		PrintToStream(EStreamIO &stream) const;
	void		PrintToStream() const;

	size_t		FlattenedSize() const;
	bool		Flatten(char *buffer, size_t bufferSize) const;
	bool		Unflatten(const char *buffer, size_t bufferSize);

	bool		WasDelivered() const;
	bool		IsReply() const;
	bool		IsSourceWaiting() const;

	e_status_t	SendReply(euint32 command, EHandler *replyHandler = NULL) const;
	e_status_t	SendReply(const EMessage *message,
				  EHandler *replyHandler = NULL,
				  e_bigtime_t sendTimeout = E_INFINITE_TIMEOUT) const;

	/* BGetInfo()/BFindData(): likes BMessage::GetInfo()/FindData() */
	e_status_t	BGetInfo(e_type_code type, eint32 index,
				 char **nameFound, e_type_code *typeFound, eint32 *countFound = NULL) const;
	e_status_t	BFindData(const char *name, e_type_code type, eint32 index,
				  const void **data, ssize_t *numBytes) const;
	e_status_t	BFindData(const char *name, e_type_code type,
				  const void **data, ssize_t *numBytes) const;

private:
	friend class ELooper;
	friend class EMessenger;

	typedef struct list_data {
		char 		*name;
		EList		list;
	} list_data;

	typedef struct type_list_data {
		e_type_code	type;
		EList		list;
	} type_list_data;

	typedef struct _object_t {
		size_t		bytes;
		bool		fixed_size;
		void		*data;
	} _object_t;

	EList fObjectsList;

	static list_data *_find_list(const EList *list, const char *name, eint32 *index = NULL);
	static type_list_data *_find_type_list(const EList *list, e_type_code type);

	eint64 fTeam;

	euint64 fTargetToken;
	e_bigtime_t fTargetTokenTimestamp;
	euint64 fReplyToken;
	e_bigtime_t fReplyTokenTimestamp;

	bool fNoticeSource;
	void *fSource;

	bool fIsReply;
};


inline e_status_t
EMessage::BGetInfo(e_type_code type, eint32 index,
		   char **nameFound, e_type_code *typeFound, eint32 *countFound) const
{
	if(index < 0) return E_BAD_INDEX;
	eint32 aIndex = index;

	for(eint32 i = 0; i < CountNames(E_ANY_TYPE, true); i++)
	{
		eint32 typesCount = CountTypesByName(i);
		for(eint32 k = 0; k < typesCount; k++)
		{
			e_type_code aType;
			eint32 count = CountItems(i, k, &aType);
			if(!(type == E_ANY_TYPE || aType == type) || (aIndex--) > 0) continue;
			if(nameFound) *nameFound = (char*)NameAt(i);
			if(typeFound) *typeFound = aType;
			if(countFound) *countFound = count;
			return E_OK;
		}
	}

	return(aIndex == index ? E_BAD_TYPE : E_BAD_INDEX);
}


inline e_status_t
EMessage::BFindData(const char *name, e_type_code type, eint32 index,
		    const void **data, ssize_t *numBytes) const
{
	if(index < 0) return E_BAD_INDEX;

	eint32 nameIndex = FindName(name);
	if(nameIndex < 0) return E_NAME_NOT_FOUND;

	eint32 typesCount = CountTypesByName(nameIndex);
	eint32 aIndex = index;

	for(eint32 k = 0; k < typesCount; k++)
	{
		e_type_code aType;
		eint32 count = CountItems(nameIndex, k, &aType);
		if(!(type == E_ANY_TYPE || aType == type)) continue;

		if(aIndex < count)
			return(FindData(nameIndex, k, aIndex, data, numBytes) ? E_OK : E_ERROR);

		aIndex -= count;
	}

	return(aIndex == index ? E_BAD_TYPE : E_BAD_INDEX);
}


inline e_status_t
EMessage::BFindData(const char *name, e_type_code type,
		    const void **data, ssize_t *numBytes) const
{
	return BFindData(name, type, 0, data, numBytes);
}

#endif /* __cplusplus */

#endif /* __ETK_MESSAGE_H__ */

