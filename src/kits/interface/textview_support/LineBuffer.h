/*
 * Copyright 2001-2006, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Marc Flerackers (mflerackers@androme.be)
 */
#ifndef __LINE_BUFFER_H
#define __LINE_BUFFER_H


#include <Haiku.h>
#include <TextView.h>

#include <TextViewSupportBuffer.h>

struct STELine {
	long		offset;		// offset of first character of line
	float		origin;		// pixel position of top of line
	float		ascent;		// maximum ascent for line
	float		width;		// cached width of line in pixels
};


class BTextView::LineBuffer : public _BTextViewSupportBuffer_<STELine> {

public:
								LineBuffer();
	virtual						~LineBuffer();

			void				InsertLine(STELine* inLine, int32_t index);
			void				RemoveLines(int32_t index, int32_t count = 1);
			void				RemoveLineRange(int32_t fromOffset,
									int32_t toOffset);

			int32_t				OffsetToLine(int32_t offset) const;
			int32_t				PixelToLine(float pixel) const;

			void				BumpOrigin(float delta, int32_t index);
			void				BumpOffset(int32_t delta, int32_t index);

			int32_t				NumLines() const;
			float				MaxWidth() const;
			STELine*			operator[](int32_t index) const;
};


inline int32_t BTextView::LineBuffer::NumLines() const
{
	return fItemCount - 1;
}


inline STELine *
BTextView::LineBuffer::operator[](int32_t index) const
{
	return &fBuffer[index];
}


#endif	// __LINE_BUFFER_H
