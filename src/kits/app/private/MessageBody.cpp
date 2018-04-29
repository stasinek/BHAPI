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
 * File: MessageBody.cpp
 *
 * --------------------------------------------------------------------------*/

#include <kits/netMessageBody.h>
#include <kits/netMemory.h>

#include <kits/support/String.h>
#include <os/debug.h>

class BHAPI_LOCAL BMessageNode
{
public:
	BMessageNode(const char *name);
	BMessageNode(type_code type);
	~BMessageNode();

	const char	*Name() const;
	type_code	TypeCode() const;

	bool		AddItem(BMessageItem *item);
	void		RemoveItem(BMessageItem *item);

	__be_int32		CountItems() const;
	__be_int32		IndexOf(BMessageItem *item) const;
	BMessageItem	*ItemAt(__be_int32 index) const;

private:
	char *fName;
	type_code fType;
	BList fItems;
};


BMessageNode::BMessageNode(const char *name)
	: fName(bhapi::strdup(name)), fType((type_code)0)
{
}


BMessageNode::BMessageNode(type_code type)
	: fName(NULL), fType(type)
{
}


BMessageNode::~BMessageNode()
{
	if(fName != NULL) delete[] fName;
}


const char*
BMessageNode::Name() const
{
	return fName;
}


type_code
BMessageNode::TypeCode() const
{
	return fType;
}


bool 
BMessageNode::AddItem(BMessageItem *item)
{
	return fItems.AddItem(item);
}


void 
BMessageNode::RemoveItem(BMessageItem *item)
{
	if(fItems.RemoveItem(fItems.IndexOf(item)) != item)
		BHAPI_ERROR("[PRIVATE]: %s --- Invalid operation.", __PRETTY_FUNCTION__);
}


__be_int32
BMessageNode::CountItems() const
{
	return fItems.CountItems();
}


__be_int32
BMessageNode::IndexOf(BMessageItem *item) const
{
	return fItems.IndexOf(item);
}


BMessageItem*
BMessageNode::ItemAt(__be_int32 index) const
{
	return (BMessageItem*)fItems.ItemAt(index);
}


BMessageItem::BMessageItem(void *data, size_t nBytes, bool fixedSize)
	: fFixedSize(false), fBytes(0), fData(NULL)
{
	SetData(data, nBytes, fixedSize);
}


BMessageItem::~BMessageItem()
{
	if(fFixedSize && fData != NULL) BMemory::Free(fData);
}


void 
BMessageItem::SetData(void *data, size_t nBytes, bool fixedSize)
{
	if(fFixedSize && fData != NULL) BMemory::Free(fData);

	fFixedSize = fixedSize;
	fBytes = nBytes;
	fData = data;
}


void*
BMessageItem::Data() const
{
	return fData;
}


size_t
BMessageItem::Bytes() const
{
	return fBytes;
}


bool 
BMessageItem::IsFixedSize() const
{
	return fFixedSize;
}


BMessageBody::BMessageBody()
	: fNames(NULL), fTypes(NULL)
{
}


BMessageBody::~BMessageBody()
{
	// TODO
}


bool 
BMessageBody::AddItem(const char *name, type_code type, BMessageItem *item)
{
	// TODO
	BHAPI_WARNING("[PRIVATE]: %s --- TODO", __PRETTY_FUNCTION__);
	return false;
}


void 
BMessageBody::RemoveItem(BMessageItem *item)
{
	// TODO
	BHAPI_ERROR("[PRIVATE]: %s --- TODO", __PRETTY_FUNCTION__);
}


__be_int32
BMessageBody::CountNames() const
{
	return(fNames == NULL ? 0 : fNames->CountItems());
}


__be_int32
BMessageBody::CountTypes() const
{
	return(fTypes == NULL ? 0 : fTypes->CountItems());
}


size_t
BMessageBody::FlattenedSize() const
{
	// TODO
	BHAPI_WARNING("[PRIVATE]: %s --- TODO", __PRETTY_FUNCTION__);
	return 0;
}


bool 
BMessageBody::Flatten(char *buffer, size_t size) const
{
	// TODO
	BHAPI_WARNING("[PRIVATE]: %s --- TODO", __PRETTY_FUNCTION__);
	return false;
}


bool 
BMessageBody::Flatten(BDataIO *stream,  ssize_t *size) const
{
	// TODO
	BHAPI_WARNING("[PRIVATE]: %s --- TODO", __PRETTY_FUNCTION__);
	return false;
}


bool 
BMessageBody::Unflatten(const char *buffer, size_t size)
{
	// TODO
	BHAPI_WARNING("[PRIVATE]: %s --- TODO", __PRETTY_FUNCTION__);
	return false;
}


bool 
BMessageBody::Unflatten(BDataIO *stream, size_t size)
{
	// TODO
	BHAPI_WARNING("[PRIVATE]: %s --- TODO", __PRETTY_FUNCTION__);
	return false;
}


void 
BMessageBody::PrintToStream(BStreamIO &stream) const
{
	// TODO
	BHAPI_WARNING("[PRIVATE]: %s --- TODO", __PRETTY_FUNCTION__);
}

