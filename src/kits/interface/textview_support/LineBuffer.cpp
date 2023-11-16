/*
 * Copyright 2001-2006, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Marc Flerackers (mflerackers@androme.be)
 */


#include <LineBuffer.h>


BTextView::LineBuffer::LineBuffer()
	:	_BTextViewSupportBuffer_<STELine>(20, 2)
{
}


BTextView::LineBuffer::~LineBuffer()
{
}


void BTextView::LineBuffer::InsertLine(STELine* inLine, int32_t index)
{
	InsertItemsAt(1, index, inLine);
}


void BTextView::LineBuffer::RemoveLines(int32_t index, int32_t count)
{
	RemoveItemsAt(count, index);
}


void BTextView::LineBuffer::RemoveLineRange(int32_t fromOffset, int32_t toOffset)
{
	int32_t fromLine = OffsetToLine(fromOffset);
	int32_t toLine = OffsetToLine(toOffset);

	int32_t count = toLine - fromLine;
	if (count > 0)
		RemoveLines(fromLine + 1, count);

	BumpOffset(fromOffset - toOffset, fromLine + 1);
}


int32_t BTextView::LineBuffer::OffsetToLine(int32_t offset) const
{
	int32_t minIndex = 0;
	int32_t maxIndex = fItemCount - 1;
	int32_t index = 0;

	while (minIndex < maxIndex) {
		index = (minIndex + maxIndex) >> 1;
		if (offset >= fBuffer[index].offset) {
			if (offset < fBuffer[index + 1].offset)
				break;
			else
				minIndex = index + 1;
		} else
			maxIndex = index;
	}

	return index;
}


int32_t BTextView::LineBuffer::PixelToLine(float pixel) const
{
	int32_t minIndex = 0;
	int32_t maxIndex = fItemCount - 1;
	int32_t index = 0;

	while (minIndex < maxIndex) {
		index = (minIndex + maxIndex) >> 1;
		if (pixel >= fBuffer[index].origin) {
			if (pixel < fBuffer[index + 1].origin)
				break;
			else
				minIndex = index + 1;
		} else
			maxIndex = index;
	}

	return index;
}


void BTextView::LineBuffer::BumpOrigin(float delta, int32_t index)
{
	for (long i = index; i < fItemCount; i++)
		fBuffer[i].origin += delta;
}


void BTextView::LineBuffer::BumpOffset(int32_t delta, int32_t index)
{
	for (long i = index; i < fItemCount; i++)
		fBuffer[i].offset += delta;
}


float
BTextView::LineBuffer::MaxWidth() const
{
	if (fItemCount == 0)
		return 0;

	float maxWidth = 0;
	STELine* line = &fBuffer[0];
	for (int32_t i = 0; i < fItemCount; i++) {
		if (maxWidth < line->width)
			maxWidth = line->width;
		line++;
	}
	return maxWidth;
}
