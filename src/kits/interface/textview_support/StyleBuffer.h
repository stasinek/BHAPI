/*
 * Copyright 2001-2006 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Marc Flerackers, mflerackers@androme.be
 *		Stefano Ceccherini, burton666@libero.it
 */


#include <Font.h>
#include <InterfaceDefs.h>
#include <Haiku.h>
#include <TextView.h>

#include <TextViewSupportBuffer.h>


struct STEStyle {
	BFont		font;		// font
	rgb_color	color;		// pen color
};


struct STEStyleRun {
	long		offset;		// byte offset of first character of run
	STEStyle	style;		// style info
};


struct STEStyleRange {
	long		count;		// number of style runs
	STEStyleRun	runs[1];	// array of count number of runs
};


struct STEStyleRecord {
	long		refs;		// reference count for this style
	float		ascent;		// ascent for this style
	float		descent;	// descent for this style
	STEStyle	style;		// style info
};


struct STEStyleRunDesc {
	long		offset;		// byte offset of first character of run
	long		index;		// index of corresponding style record
};


// _BStyleRunDescBuffer_ class -------------------------------------------------
class _BStyleRunDescBuffer_ : public _BTextViewSupportBuffer_<STEStyleRunDesc> {
public:
								_BStyleRunDescBuffer_();

			void				InsertDesc(STEStyleRunDesc* inDesc,
								int32_t index);
			void				RemoveDescs(int32_t index, int32_t count = 1);

			int32_t				OffsetToRun(int32_t offset) const;
			void				BumpOffset(int32_t delta, int32_t index);

			STEStyleRunDesc*	operator[](int32_t index) const;
};


inline STEStyleRunDesc*
_BStyleRunDescBuffer_::operator[](int32_t index) const
{
	return &fBuffer[index];
}


// _BStyleRecordBuffer_ class --------------------------------------------------
class _BStyleRecordBuffer_ : public _BTextViewSupportBuffer_<STEStyleRecord> {
public:
								_BStyleRecordBuffer_();

			int32_t				InsertRecord(const BFont* inFont,
									const rgb_color* inColor);
			void				CommitRecord(int32_t index);
			void				RemoveRecord(int32_t index);

			bool				MatchRecord(const BFont* inFont,
									const rgb_color* inColor,
									int32_t* outIndex);

			STEStyleRecord*		operator[](int32_t index) const;
};


inline STEStyleRecord*
_BStyleRecordBuffer_::operator[](int32_t index) const
{
	return &fBuffer[index];
}


// StyleBuffer class --------------------------------------------------------
class BTextView::StyleBuffer {
public:
								StyleBuffer(const BFont* inFont,
									const rgb_color* inColor);

			void				InvalidateNullStyle();
			bool				IsValidNullStyle() const;

			void				SyncNullStyle(int32_t offset);
			void				SetNullStyle(uint32_t inMode,
									const BFont* inFont,
									const rgb_color* inColor,
									int32_t offset = 0);
			void				GetNullStyle(const BFont** font,
									const rgb_color** color) const;

			void				GetStyle(int32_t inOffset, BFont* outFont,
									rgb_color* outColor) const;
			void				ContinuousGetStyle(BFont*, uint32_t*,
									rgb_color*, bool*, int32_t, int32_t) const;

			STEStyleRange*		AllocateStyleRange(
									const int32_t numStyles) const;
			void				SetStyleRange(int32_t fromOffset,
									int32_t toOffset, int32_t textLen,
									uint32_t inMode, const BFont* inFont,
									const rgb_color* inColor);
			STEStyleRange*		GetStyleRange(int32_t startOffset,
									int32_t endOffset) const;

			void				RemoveStyleRange(int32_t fromOffset,
									int32_t toOffset);
			void				RemoveStyles(int32_t index, int32_t count = 1);

			int32_t				Iterate(int32_t fromOffset, int32_t length,
									InlineInput* input,
									const BFont** outFont = NULL,
									const rgb_color** outColor = NULL,
									float* outAscent = NULL,
									float* outDescen = NULL,
									uint32_t* = NULL) const;

			int32_t				OffsetToRun(int32_t offset) const;
			void				BumpOffset(int32_t delta, int32_t index);

			STEStyleRun			operator[](int32_t index) const;
			int32_t				NumRuns() const;

	const	_BStyleRunDescBuffer_&	RunBuffer() const;
	const	_BStyleRecordBuffer_&	RecordBuffer() const;

private:
			_BStyleRunDescBuffer_	fStyleRunDesc;
			_BStyleRecordBuffer_	fStyleRecord;
			bool				fValidNullStyle;
			STEStyle			fNullStyle;
};


inline int32_t BTextView::StyleBuffer::NumRuns() const
{
	return fStyleRunDesc.ItemCount();
}


inline const _BStyleRunDescBuffer_&
BTextView::StyleBuffer::RunBuffer() const
{
	return fStyleRunDesc;
}


inline const _BStyleRecordBuffer_&
BTextView::StyleBuffer::RecordBuffer() const
{
	return fStyleRecord;
}
