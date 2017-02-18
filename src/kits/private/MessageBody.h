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
 * File: MessageBody.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_PRIVATE_MESSAGE_BODY__H
#define BHAPI_PRIVATE_MESSAGE_BODY__H
#include <kits/support/SupportDefs.h>
#include "../support/List.h"
#include "../support/StreamIO.h"

#ifdef __cplusplus /* Just for C++ */


class BMessageBody;
class BMessageNode;


class _LOCAL BMessageItem
{
public:
    BMessageItem(void *data, size_t nBytes, bool fixedSize = true);
    ~BMessageItem();

	void		SetData(void *data, size_t nBytes, bool fixedSize = true);
	void		*Data() const;
	size_t		Bytes() const;
	bool		IsFixedSize() const;

private:
    friend class BMessageBody;
    friend class BMessageNode;

	bool fFixedSize;
	size_t fBytes;
	void *fData;
};


class _LOCAL BMessageBody
{
public:
    BMessageBody();
    ~BMessageBody();

    bool		AddItem(const char *name, b_type_code type, BMessageItem *item);
    void		RemoveItem(BMessageItem *item);

    b_int32		CountNames() const;
    b_int32		CountTypes() const;

	size_t		FlattenedSize() const;
	bool		Flatten(char *buffer, size_t size) const;
	bool		Flatten(EDataIO *stream, b_size_t *size = NULL) const;
	bool		Unflatten(const char *buffer, size_t size);
	bool		Unflatten(EDataIO *stream, size_t size);

	void		PrintToStream(BStreamIO &stream) const;
	void		PrintToStream() const;

private:
	BList *fNames;
	BList *fTypes;
};


inline void
BMessageBody::PrintToStream() const
{
	PrintToStream(EOut);
}


#endif /* __cplusplus */

#endif /* BHAPI_PRIVATE_MESSAGE_BODY__H */

