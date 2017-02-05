/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
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
 * File: NetBuffer.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../support/ByteOrder.h"

#include "NetBuffer.h"


ENetBuffer::ENetBuffer(size_t size)
	: EArchivable(), fData(NULL), fSize(size), fPos(0)
{
	if(size > 0) fData = (unsigned char*)malloc(size);
}


ENetBuffer::ENetBuffer(const ENetBuffer &from)
	: EArchivable(), fData(NULL), fSize(0), fPos(0)
{
	ENetBuffer::operator=(from);
}


ENetBuffer::~ENetBuffer()
{
	if(fData) free(fData);
}


ENetBuffer::ENetBuffer(const EMessage *from)
	: EArchivable(from), fData(NULL), fSize(0), fPos(0)
{
	// TODO
}


e_status_t
ENetBuffer::Archive(EMessage *into, bool deep) const
{
	if(!into) return E_ERROR;

	EArchivable::Archive(into, deep);
	into->AddString("class", "ENetBuffer");

	// TODO

	return E_OK;
}


EArchivable*
ENetBuffer::Instantiate(const EMessage *from)
{
	if(e_validate_instantiation(from, "ENetBuffer"))
		return new ENetBuffer(from);
	return NULL;
}


e_status_t
ENetBuffer::InitCheck() const
{
	return((fData != NULL && fSize != 0 && fPos < fSize) ? E_OK : E_ERROR);
}


ENetBuffer&
ENetBuffer::operator=(const ENetBuffer &buf)
{
	if(fData) free(fData);

	if(!(buf.InitCheck() != E_OK ||
	     (fData = (unsigned char*)malloc(buf.fSize)) == NULL))
	{
		memcpy(fData, buf.fData, fSize);
		fSize = buf.fSize;
		fPos = buf.fPos;
	}
	else
	{
		fData = NULL;
		fSize = 0;
		fPos = 0;
	}

	return *this;
}


e_status_t
ENetBuffer::AppendData(const void *data, size_t len)
{
	if(data == NULL || len == 0 ||
	   fData == NULL || fPos + len > fSize) return E_ERROR;

	memcpy(fData + fPos, data, len);
	fPos += len;

	return E_OK;
}


e_status_t
ENetBuffer::AppendInt8(eint8 value)
{
	return AppendData(&value, 1);
}


e_status_t
ENetBuffer::AppendUint8(euint8 value)
{
	return AppendData(&value, 1);
}


e_status_t
ENetBuffer::AppendInt16(eint16 value)
{
	eint16 v = E_HOST_TO_BENDIAN_INT16(value);
	return AppendData(&v, 2);
}


e_status_t
ENetBuffer::AppendUint16(euint16 value)
{
	eint16 v = E_HOST_TO_BENDIAN_INT16(value);
	return AppendData(&v, 2);
}


e_status_t
ENetBuffer::AppendInt32(eint32 value)
{
	eint32 v = E_HOST_TO_BENDIAN_INT32(value);
	return AppendData(&v, 4);
}


e_status_t
ENetBuffer::AppendUint32(euint32 value)
{
	eint32 v = E_HOST_TO_BENDIAN_INT32(value);
	return AppendData(&v, 4);
}


e_status_t
ENetBuffer::AppendInt64(eint64 value)
{
	eint64 v = E_HOST_TO_BENDIAN_INT64(value);
	return AppendData(&v, 8);
}


e_status_t
ENetBuffer::AppendUint64(euint64 value)
{
	eint64 v = E_HOST_TO_BENDIAN_INT64(value);
	return AppendData(&v, 8);
}


e_status_t
ENetBuffer::AppendFloat(float value)
{
	return AppendData(&value, sizeof(float));
}


e_status_t
ENetBuffer::AppendDouble(double value)
{
	return AppendData(&value, sizeof(double));
}


e_status_t
ENetBuffer::AppendString(const char *string, eint32 len)
{
	size_t strLen = 0;

	if(fData == NULL) return E_ERROR;

	if(!(string == NULL || *string == 0)) strLen = strlen(string);
	if(!(len < 0 || (size_t)len >= strLen)) strLen = (size_t)len;

	if(fPos + strLen + 1 > fSize) return E_ERROR;

	if(strLen > 0)
	{
		memcpy(fData + fPos, string, strLen);
		fPos += strLen;
	}

	*(fData + (fPos++)) = 0;

	return E_OK;
}


e_status_t
ENetBuffer::AppendMessage(const EMessage &msg)
{
	if(fData == NULL) return E_ERROR;

	size_t msgSize = msg.FlattenedSize();
	if(msgSize == 0 || fPos + msgSize + 8 > fSize) return E_ERROR;

	char *buf = (char*)malloc(msgSize);
	if(buf == NULL) return E_NO_MEMORY;

	if(msg.Flatten(buf, msgSize) == false)
	{
		free(buf);
		return E_ERROR;
	}

	// data
	memcpy(fData + fPos, buf, msgSize);
	fPos += msgSize;
	free(buf);

	// size
	euint32 tmp = (euint32)msgSize;
#ifdef ETK_LITTLE_LENDIAN
	tmp = E_SWAP_INT32(tmp);
#endif
	memcpy(fData + fPos, &tmp, 4);
	fPos += 4;

	// magic
	*(fData + (fPos++)) = 0;
	*(fData + (fPos++)) = 'm';
	*(fData + (fPos++)) = 's';
	*(fData + (fPos++)) = 'g';

	return E_OK;
}


e_status_t
ENetBuffer::RemoveData(void *data, size_t len)
{
	if(data == NULL || len == 0 || fData == NULL || fPos < len) return E_ERROR;

	fPos -= len;
	memcpy(data, fData + fPos, len);

	return E_OK;
}


e_status_t
ENetBuffer::RemoveInt8(eint8 &value)
{
	return RemoveData(&value, 1);
}


e_status_t
ENetBuffer::RemoveUint8(euint8 &value)
{
	return RemoveData(&value, 1);
}


e_status_t
ENetBuffer::RemoveInt16(eint16 &value)
{
	if(RemoveData(&value, 2) != E_OK) return E_ERROR;

	value = E_BENDIAN_TO_HOST_INT16(value);
	return E_OK;
}


e_status_t
ENetBuffer::RemoveUint16(euint16 &value)
{
	if(RemoveData(&value, 2) != E_OK) return E_ERROR;

	value = E_BENDIAN_TO_HOST_INT16(value);
	return E_OK;
}


e_status_t
ENetBuffer::RemoveInt32(eint32 &value)
{
	if(RemoveData(&value, 4) != E_OK) return E_ERROR;

	value = E_BENDIAN_TO_HOST_INT32(value);
	return E_OK;
}


e_status_t
ENetBuffer::RemoveUint32(euint32 &value)
{
	if(RemoveData(&value, 4) != E_OK) return E_ERROR;

	value = E_BENDIAN_TO_HOST_INT32(value);
	return E_OK;
}


e_status_t
ENetBuffer::RemoveInt64(eint64 &value)
{
	if(RemoveData(&value, 8) != E_OK) return E_ERROR;

	value = E_BENDIAN_TO_HOST_INT64(value);
	return E_OK;
}


e_status_t
ENetBuffer::RemoveUint64(euint64 &value)
{
	if(RemoveData(&value, 8) != E_OK) return E_ERROR;

	value = E_BENDIAN_TO_HOST_INT64(value);
	return E_OK;
}


e_status_t
ENetBuffer::RemoveFloat(float &value)
{
	return RemoveData(&value, sizeof(float));
}


e_status_t
ENetBuffer::RemoveDouble(double &value)
{
	return RemoveData(&value, sizeof(double));
}


e_status_t
ENetBuffer::RemoveString(char *string, size_t len)
{
	if(string == NULL || len == 0 || fData == NULL || fPos == 0) return E_ERROR;

	if(*(fData + fPos - 1) != 0) return E_ERROR;
	if(len > fPos) len = fPos;

	fPos -= len;
	memcpy(string, fData + fPos, len);

	return E_OK;
}


e_status_t
ENetBuffer::RemoveMessage(EMessage &msg)
{
	if(fData == NULL || fPos <= 8) return E_ERROR;

	const unsigned char *magic = fData + fPos - 4;
	if(magic[0] != 0 || magic[1] != 'm' || magic[2] != 's' || magic[3] != 'g') return E_ERROR;

	euint32 msgSize = 0;
	memcpy(&msgSize, fData + fPos - 8, 4);
#ifdef ETK_LITTLE_LENDIAN
	msgSize = E_SWAP_INT32(msgSize);
#endif
	if(msgSize == 0 || (size_t)msgSize > fPos - 8) return E_ERROR;

	if(msg.Unflatten((const char*)(fData + fPos - (size_t)msgSize - 8),
			 (size_t)msgSize) == false) return E_ERROR;

	fPos -= (size_t)msgSize + 8;
	return E_OK;
}


unsigned char*
ENetBuffer::Data() const
{
	return fData;
}


size_t
ENetBuffer::Size() const
{
	return fSize;
}


size_t
ENetBuffer::BytesRemaining() const
{
	return(InitCheck() == E_OK ? fSize - fPos : 0);
}

