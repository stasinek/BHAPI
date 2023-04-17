/*
 * Copyright 2001-2006, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Marc Flerackers (mflerackers@androme.be)
 */
#ifndef __TEXTGAPBUFFER_H
#define __TEXTGAPBUFFER_H


#include <Haiku.h>
#include <TextView.h>


class BFile;


namespace BPrivate {


class TextGapBuffer {
public:
								TextGapBuffer();
								~TextGapBuffer();

			void				InsertText(const char* inText, int32_t inNumItems,
									int32_t inAtIndex);
			void				InsertText(BFile* file, int32_t fileOffset,
									int32_t amount, int32_t atIndex);
			void				RemoveRange(int32_t start, int32_t end);

			bool				FindChar(char inChar, int32_t fromIndex,
									int32_t* ioDelta);

			const char*			Text();
			const char*			RealText();
			int32_t				Length() const;

			const char*			GetString(int32_t fromOffset, int32_t* numBytes);
			void				GetString(int32_t offset, int32_t length,
									char* buffer);

			char				RealCharAt(int32_t offset) const;

			bool				PasswordMode() const;
			void				SetPasswordMode(bool);

private:
			void				_MoveGapTo(int32_t toIndex);
			void				_EnlargeGapTo(int32_t inCount);
			void				_ShrinkGapTo(int32_t inCount);

			int32_t				fItemCount;			// logical count
			char*				fBuffer;			// allocated memory
			int32_t				fBufferCount;		// physical count
			int32_t				fGapIndex;			// gap position
			int32_t				fGapCount;			// gap count
			char*				fScratchBuffer;		// for GetString
			int32_t				fScratchSize;		// scratch size
			bool				fPasswordMode;
};


inline int32_t TextGapBuffer::Length() const
{
	return fItemCount;
}


inline char
TextGapBuffer::RealCharAt(int32_t index) const
{
	if (index < 0 || index >= fItemCount) {
		if (index != fItemCount)
			debugger("RealCharAt: invalid index supplied");
		return 0;
	}

	return index < fGapIndex ? fBuffer[index] : fBuffer[index + fGapCount];
}


} // namespace BPrivate


#endif //__TEXTGAPBUFFER_H
