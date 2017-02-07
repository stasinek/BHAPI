/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
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
 * File: NetBuffer.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __BHAPI_NET_BUFFER_H__
#define __BHAPI_NET_BUFFER_H__

#include "./../support/Archivable.h"

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_BHAPI BNetBuffer : public BArchivable {
public:
	BNetBuffer(size_t size = 0);
	BNetBuffer(const BNetBuffer &from);
	virtual ~BNetBuffer();

	// Archiving
	BNetBuffer(const BMessage *from);
	virtual b_status_t Archive(BMessage *into, bool deep = true) const;
	static BArchivable *Instantiate(const BMessage *from);

	b_status_t	InitCheck() const;

	BNetBuffer	&operator=(const BNetBuffer &buf);

	b_status_t	AppendData(const void *data, size_t len);
    b_status_t	AppendInt8(b_int8 value);
    b_status_t	AppendUint8(b_uint8 value);
    b_status_t	AppendInt16(b_int16 value);
    b_status_t	AppendUint16(b_uint16 value);
    b_status_t	AppendInt32(b_int32 value);
    b_status_t	AppendUint32(b_uint32 value);
    b_status_t	AppendInt64(b_int64 value);
    b_status_t	AppendUint64(b_uint64 value);
	b_status_t	AppendFloat(float value);
	b_status_t	AppendDouble(double value);
    b_status_t	AppenString(const char *string, b_int32 len = -1);
	b_status_t	AppendMessage(const BMessage &msg);

	b_status_t	RemoveData(void *data, size_t len);
    b_status_t	RemoveInt8(b_int8 &value);
    b_status_t	RemoveUint8(b_uint8 &value);
    b_status_t	RemoveInt16(b_int16 &value);
    b_status_t	RemoveUint16(b_uint16 &value);
    b_status_t	RemoveInt32(b_int32 &value);
    b_status_t	RemoveUint32(b_uint32 &value);
    b_status_t	RemoveInt64(b_int64 &value);
    b_status_t	RemoveUint64(b_uint64 &value);
	b_status_t	RemoveFloat(float &value);
	b_status_t	RemoveDouble(double &value);
    b_status_t	RemovString(char *string, size_t len);
	b_status_t	RemoveMessage(BMessage &msg);

	unsigned char	*Data() const;
	size_t		Size() const;
	size_t		BytesRemaining() const;

private:
	unsigned char *fData;
	size_t fSize;
	size_t fPos;
};

#endif /* __cplusplus */

#endif /* __BHAPI_NET_BUFFER_H__ */

