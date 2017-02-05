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
 * File: MessageBody.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../support/String.h"

#include "Memory.h"
#include "MessageBody.h"


class _LOCAL EMessageNode
{
public:
	EMessageNode(const char *name);
	EMessageNode(e_type_code type);
	~EMessageNode();

	const char	*Name() const;
	e_type_code	TypeCode() const;

	bool		AddItem(EMessageItem *item);
	void		RemoveItem(EMessageItem *item);

	eint32		CountItems() const;
	eint32		IndexOf(EMessageItem *item) const;
	EMessageItem	*ItemAt(eint32 index) const;

private:
	char *fName;
	e_type_code fType;
	EList fItems;
};


EMessageNode::EMessageNode(const char *name)
	: fName(EStrdup(name)), fType((e_type_code)0)
{
}


EMessageNode::EMessageNode(e_type_code type)
	: fName(NULL), fType(type)
{
}


EMessageNode::~EMessageNode()
{
	if(fName != NULL) delete[] fName;
}


const char*
EMessageNode::Name() const
{
	return fName;
}


e_type_code
EMessageNode::TypeCode() const
{
	return fType;
}


bool
EMessageNode::AddItem(EMessageItem *item)
{
	return fItems.AddItem(item);
}


void
EMessageNode::RemoveItem(EMessageItem *item)
{
	if(fItems.RemoveItem(fItems.IndexOf(item)) != item)
		ETK_ERROR("[PRIVATE]: %s --- Invalid operation.", __PRETTY_FUNCTION__);
}


eint32
EMessageNode::CountItems() const
{
	return fItems.CountItems();
}


eint32
EMessageNode::IndexOf(EMessageItem *item) const
{
	return fItems.IndexOf(item);
}


EMessageItem*
EMessageNode::ItemAt(eint32 index) const
{
	return (EMessageItem*)fItems.ItemAt(index);
}


EMessageItem::EMessageItem(void *data, size_t nBytes, bool fixedSize)
	: fFixedSize(false), fBytes(0), fData(NULL)
{
	SetData(data, nBytes, fixedSize);
}


EMessageItem::~EMessageItem()
{
	if(fFixedSize && fData != NULL) EMemory::Free(fData);
}


void
EMessageItem::SetData(void *data, size_t nBytes, bool fixedSize)
{
	if(fFixedSize && fData != NULL) EMemory::Free(fData);

	fFixedSize = fixedSize;
	fBytes = nBytes;
	fData = data;
}


void*
EMessageItem::Data() const
{
	return fData;
}


size_t
EMessageItem::Bytes() const
{
	return fBytes;
}


bool
EMessageItem::IsFixedSize() const
{
	return fFixedSize;
}


EMessageBody::EMessageBody()
	: fNames(NULL), fTypes(NULL)
{
}


EMessageBody::~EMessageBody()
{
	// TODO
}


bool
EMessageBody::AddItem(const char *name, e_type_code type, EMessageItem *item)
{
	// TODO
	ETK_WARNING("[PRIVATE]: %s --- TODO", __PRETTY_FUNCTION__);
	return false;
}


void
EMessageBody::RemoveItem(EMessageItem *item)
{
	// TODO
	ETK_ERROR("[PRIVATE]: %s --- TODO", __PRETTY_FUNCTION__);
}


eint32
EMessageBody::CountNames() const
{
	return(fNames == NULL ? 0 : fNames->CountItems());
}


eint32
EMessageBody::CountTypes() const
{
	return(fTypes == NULL ? 0 : fTypes->CountItems());
}


size_t
EMessageBody::FlattenedSize() const
{
	// TODO
	ETK_WARNING("[PRIVATE]: %s --- TODO", __PRETTY_FUNCTION__);
	return 0;
}


bool
EMessageBody::Flatten(char *buffer, size_t size) const
{
	// TODO
	ETK_WARNING("[PRIVATE]: %s --- TODO", __PRETTY_FUNCTION__);
	return false;
}


bool
EMessageBody::Flatten(EDataIO *stream, ssize_t *size) const
{
	// TODO
	ETK_WARNING("[PRIVATE]: %s --- TODO", __PRETTY_FUNCTION__);
	return false;
}


bool
EMessageBody::Unflatten(const char *buffer, size_t size)
{
	// TODO
	ETK_WARNING("[PRIVATE]: %s --- TODO", __PRETTY_FUNCTION__);
	return false;
}


bool
EMessageBody::Unflatten(EDataIO *stream, size_t size)
{
	// TODO
	ETK_WARNING("[PRIVATE]: %s --- TODO", __PRETTY_FUNCTION__);
	return false;
}


void
EMessageBody::PrintToStream(EStreamIO &stream) const
{
	// TODO
	ETK_WARNING("[PRIVATE]: %s --- TODO", __PRETTY_FUNCTION__);
}

