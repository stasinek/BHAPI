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
 * File: MessageBody.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_PRIVATE_MESSAGE_BODY_H__
#define __ETK_PRIVATE_MESSAGE_BODY_H__

#include "./../support/List.h"
#include "./../support/StreamIO.h"

#ifdef __cplusplus /* Just for C++ */


class EMessageBody;
class EMessageNode;


class _LOCAL EMessageItem
{
public:
	EMessageItem(void *data, size_t nBytes, bool fixedSize = true);
	~EMessageItem();

	void		SetData(void *data, size_t nBytes, bool fixedSize = true);
	void		*Data() const;
	size_t		Bytes() const;
	bool		IsFixedSize() const;

private:
	friend class EMessageBody;
	friend class EMessageNode;

	bool fFixedSize;
	size_t fBytes;
	void *fData;
};


class _LOCAL EMessageBody
{
public:
	EMessageBody();
	~EMessageBody();

	bool		AddItem(const char *name, e_type_code type, EMessageItem *item);
	void		RemoveItem(EMessageItem *item);

	eint32		CountNames() const;
	eint32		CountTypes() const;

	size_t		FlattenedSize() const;
	bool		Flatten(char *buffer, size_t size) const;
	bool		Flatten(EDataIO *stream, ssize_t *size = NULL) const;
	bool		Unflatten(const char *buffer, size_t size);
	bool		Unflatten(EDataIO *stream, size_t size);

	void		PrintToStream(EStreamIO &stream) const;
	void		PrintToStream() const;

private:
	EList *fNames;
	EList *fTypes;
};


inline void
EMessageBody::PrintToStream() const
{
	PrintToStream(EOut);
}


#endif /* __cplusplus */

#endif /* __ETK_PRIVATE_MESSAGE_BODY_H__ */

