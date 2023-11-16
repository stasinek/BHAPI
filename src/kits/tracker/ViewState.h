/*
Open Tracker License

Terms and Conditions

Copyright (c) 1991-2000, Be Incorporated. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice applies to all licensees
and shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF TITLE, MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
BE INCORPORATED BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Be Incorporated shall not be
used in advertising or otherwise to promote the sale, use or other dealings in
this Software without prior written authorization from Be Incorporated.

Tracker(TM), Be(R), BeOS(R), and BeIA(TM) are trademarks or registered trademarks
of Be Incorporated in the United States and other countries. Other brand product
names are registered trademarks or trademarks of their respective holders.
All rights reserved.
*/
#ifndef _VIEW_STATE_H
#define _VIEW_STATE_H


#include <DataIO.h>
#include <kits/support/String.h>


namespace BPrivate {

const int32_t kColumnStateArchiveVersion = 22;
	// bump version when layout or size changes

class BColumn {
public:
	BColumn(const char* title, float offset, float width,
		alignment align, const char* attributeName, uint32_t attrType,
		const char* displayAs, bool statField, bool editable);
	BColumn(const char* title, float offset, float width,
		alignment align, const char* attributeName, uint32_t attrType,
		bool statField, bool editable);
	~BColumn();

	BColumn(BMallocIO* stream, int32_t version, bool endianSwap = false);
	BColumn(const BMessage &, int32_t index = 0);
	static BColumn* InstantiateFromStream(BMallocIO* stream,
		bool endianSwap = false);
	static BColumn* InstantiateFromMessage(const BMessage &archive,
		int32_t index = 0);
	void ArchiveToStream(BMallocIO* stream) const;
	void ArchiveToMessage(BMessage &) const;

	const char* Title() const;
	float Offset() const;
	float Width() const;
	alignment Alignment() const;
	const char* AttrName() const;
	uint32_t AttrType() const;
	const char* DisplayAs() const;
	uint32_t AttrHash() const;
	bool StatField() const;
	bool Editable() const;

	void SetOffset(float);
	void SetWidth(float);

private:
	void _Init(const char* title, float offset, float width,
		alignment align, const char* attributeName, uint32_t attrType,
		const char* displayAs, bool statField, bool editable);
	static BColumn* _Sanitize(BColumn* column);

	BString fTitle;
	float fOffset;
	float fWidth;
	alignment fAlignment;
	BString fAttrName;
	BString fDisplayAs;
	uint32_t fAttrHash;
	uint32_t fAttrType;
	bool fStatField;
	bool fEditable;
};


const int32_t kViewStateArchiveVersion = 11;
	// bump version when layout or size changes

class BViewState {
public:
	BViewState();

	BViewState(BMallocIO* stream, bool endianSwap = false);
	BViewState(const BMessage &message);
	static BViewState* InstantiateFromStream(BMallocIO* stream,
		bool endianSwap = false);
	static BViewState* InstantiateFromMessage(const BMessage &message);
	void ArchiveToStream(BMallocIO* stream) const;
	void ArchiveToMessage(BMessage &message) const;

	uint32_t ViewMode() const;
	uint32_t LastIconMode() const;
	uint32_t IconSize() const;
	uint32_t LastIconSize() const;
	BPoint ListOrigin() const;
	BPoint IconOrigin() const;
	uint32_t PrimarySort() const;
	uint32_t SecondarySort() const;
	uint32_t PrimarySortType() const;
	uint32_t SecondarySortType() const;
	bool ReverseSort() const;

	void SetViewMode(uint32_t);
	void SetLastIconMode(uint32_t);
	void SetIconSize(uint32_t);
	void SetLastIconSize(uint32_t);
	void SetListOrigin(BPoint);
	void SetIconOrigin(BPoint);
	void SetPrimarySort(uint32_t);
	void SetSecondarySort(uint32_t);
	void SetPrimarySortType(uint32_t);
	void SetSecondarySortType(uint32_t);
	void SetReverseSort(bool);

	bool StateNeedsSaving();

private:
	static BViewState* _Sanitize(BViewState* state, bool fixOnly = false);

	uint32_t fViewMode;
	uint32_t fLastIconMode;
	uint32_t fIconSize;
	uint32_t fLastIconSize;
	BPoint fListOrigin;
	BPoint fIconOrigin;
	uint32_t fPrimarySortAttr;
	uint32_t fSecondarySortAttr;
	uint32_t fPrimarySortType;
	uint32_t fSecondarySortType;
	bool fReverseSort;

	void _Init();
	void _StorePreviousState();

	uint32_t fPreviousViewMode;
	uint32_t fPreviousLastIconMode;
	uint32_t fPreviousIconSize;
	uint32_t fPreviousLastIconSize;
	BPoint fPreviousListOrigin;
	BPoint fPreviousIconOrigin;
	uint32_t fPreviousPrimarySortAttr;
	uint32_t fPreviousSecondarySortAttr;
	uint32_t fPreviousPrimarySortType;
	uint32_t fPreviousSecondarySortType;
	bool fPreviousReverseSort;
};


inline const char*  BColumn::Title() const
{
	return fTitle.String();
}


inline float
BColumn::Offset() const
{
	return fOffset;
}


inline float
BColumn::Width() const
{
	return fWidth;
}


inline alignment
BColumn::Alignment() const
{
	return fAlignment;
}


inline const char*  BColumn::AttrName() const
{
	return fAttrName.String();
}


inline uint32_t BColumn::AttrHash() const
{
	return fAttrHash;
}


inline uint32_t BColumn::AttrType() const
{
	return fAttrType;
}


inline const char*  BColumn::DisplayAs() const
{
	return fDisplayAs.String();
}


inline bool BColumn::StatField() const
{
	return fStatField;
}


inline bool BColumn::Editable() const
{
	return fEditable;
}


inline void BColumn::SetWidth(float w)
{
	fWidth = w;
}


inline void BColumn::SetOffset(float o)
{
	fOffset = o;
}


inline uint32_t BViewState::ViewMode() const
{
	return fViewMode;
}


inline uint32_t BViewState::LastIconMode() const
{
	return fLastIconMode;
}


inline uint32_t BViewState::IconSize() const
{
	return fIconSize;
}


inline uint32_t BViewState::LastIconSize() const
{
	return fLastIconSize;
}


inline BPoint
BViewState::ListOrigin() const
{
	return fListOrigin;
}


inline BPoint
BViewState::IconOrigin() const
{
	return fIconOrigin;
}


inline uint32_t BViewState::PrimarySort() const
{
	return fPrimarySortAttr;
}


inline uint32_t BViewState::SecondarySort() const
{
	return fSecondarySortAttr;
}


inline uint32_t BViewState::PrimarySortType() const
{
	return fPrimarySortType;
}

inline uint32_t BViewState::SecondarySortType() const
{
	return fSecondarySortType;
}

inline bool BViewState::ReverseSort() const
{
	return fReverseSort;
}


inline void BViewState::SetViewMode(uint32_t mode)
{
	fViewMode = mode;
}


inline void BViewState::SetLastIconMode(uint32_t mode)
{
	fLastIconMode = mode;
}


inline void BViewState::SetIconSize(uint32_t size)
{
	fIconSize = size;
}


inline void BViewState::SetLastIconSize(uint32_t size)
{
	fLastIconSize = size;
}


inline void BViewState::SetListOrigin(BPoint newOrigin)
{
	fListOrigin = newOrigin;
}

inline void BViewState::SetIconOrigin(BPoint newOrigin)
{
	fIconOrigin = newOrigin;
}

inline void BViewState::SetPrimarySort(uint32_t attr)
{
	fPrimarySortAttr = attr;
}


inline void BViewState::SetSecondarySort(uint32_t attr)
{
	fSecondarySortAttr = attr;
}


inline void BViewState::SetPrimarySortType(uint32_t type)
{
	fPrimarySortType = type;
}


inline void BViewState::SetSecondarySortType(uint32_t type)
{
	fSecondarySortType = type;
}


inline void BViewState::SetReverseSort(bool on)
{
	fReverseSort = on;
}


inline bool BViewState::StateNeedsSaving()
{
	return (fPreviousViewMode != fViewMode)
		|| (fPreviousLastIconMode != fLastIconMode)
		|| (fPreviousIconSize != fIconSize)
		|| (fPreviousLastIconSize != fLastIconSize)
		|| (fPreviousListOrigin != fListOrigin)
		|| (fPreviousIconOrigin != fIconOrigin)
		|| (fPreviousPrimarySortAttr != fPrimarySortAttr)
		|| (fPreviousSecondarySortAttr != fSecondarySortAttr)
		|| (fPreviousPrimarySortType != fPrimarySortType)
		|| (fPreviousSecondarySortType != fSecondarySortType)
		|| (fPreviousReverseSort != fReverseSort);
}

} // namespace BPrivate

using namespace BPrivate;


#endif	// _VIEW_STATE_H
