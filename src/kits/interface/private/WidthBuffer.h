/*
 * Copyright 2003-2010, Haiku, Inc.
 * Distributed under the terms of the MIT License.
 */
#ifndef __WIDTHBUFFER_H
#define __WIDTHBUFFER_H


#include <Locker.h>
#include <TextView.h>

#include <TextViewSupportBuffer.h>


class BFont;


namespace BPrivate {


class TextGapBuffer;


struct _width_table_ {
	BFont font;				// corresponding font
	int32_t hashCount;		// number of hashed items
	int32_t tableCount;		// size of table
	void* widths;			// width table
};


class WidthBuffer : public _BTextViewSupportBuffer_<_width_table_> {
public:
								WidthBuffer();
	virtual						~WidthBuffer();

			float				StringWidth(const char* inText,
									int32_t fromOffset, int32_t length,
									const BFont* inStyle);
			float				StringWidth(TextGapBuffer& gapBuffer,
									int32_t fromOffset, int32_t length,
									const BFont* inStyle);

private:
			bool				FindTable(const BFont* font, int32_t* outIndex);
			int32_t				InsertTable(const BFont* font);

			bool				GetEscapement(uint32_t value, int32_t index,
									float* escapement);
			float				HashEscapements(const char* chars,
									int32_t numChars, int32_t numBytes,
									int32_t tableIndex, const BFont* font);

	static	uint32_t				Hash(uint32_t);

private:
			BLocker				fLock;
};


extern WidthBuffer* gWidthBuffer;


} // namespace BPrivate


using BPrivate::WidthBuffer;


#if __GNUC__ < 3
//! NetPositive binary compatibility support

class _BWidthBuffer_ : public _BTextViewSupportBuffer_<BPrivate::_width_table_> {
	_BWidthBuffer_();
	virtual ~_BWidthBuffer_();
};

extern
_BWidthBuffer_* gCompatibilityWidthBuffer;

#endif // __GNUC__ < 3


#endif // __WIDTHBUFFER_H
