/*
 * Copyright 2006-2010, Haiku Inc.
 * Distributed under the terms of the MIT License.
 */
#ifndef	_SPLIT_LAYOUT_H
#define	_SPLIT_LAYOUT_H


#include <AbstractLayout.h>
#include <Point.h>


namespace BPrivate {
namespace Layout {
	class Layouter;
	class LayoutInfo;
}
}

using BPrivate::Layout::Layouter;
using BPrivate::Layout::LayoutInfo;


class BSplitLayout : public BAbstractLayout {
public:
								BSplitLayout(orientation orientation,
									float spacing = 0.0f);
								BSplitLayout(BMessage* from);
	virtual						~BSplitLayout();

			void				SetInsets(float left, float top, float right,
									float bottom);
			void				GetInsets(float* left, float* top, float* right,
									float* bottom) const;

			float				Spacing() const;
			void				SetSpacing(float spacing);

			orientation			Orientation() const;
			void				SetOrientation(orientation orientation);

			float				SplitterSize() const;
			void				SetSplitterSize(float size);

	virtual	BLayoutItem*		AddView(BView* child);
	virtual	BLayoutItem*		AddView(int32_t index, BView* child);
	virtual	BLayoutItem*		AddView(BView* child, float weight);
	virtual	BLayoutItem*		AddView(int32_t index, BView* child,
									float weight);

	virtual	bool				AddItem(BLayoutItem* item);
	virtual	bool				AddItem(int32_t index, BLayoutItem* item);
	virtual	bool				AddItem(BLayoutItem* item, float weight);
	virtual	bool				AddItem(int32_t index, BLayoutItem* item,
									float weight);


			float				ItemWeight(int32_t index) const;
			float				ItemWeight(BLayoutItem* item) const;
			void				SetItemWeight(int32_t index, float weight,
									bool invalidateLayout);
			void				SetItemWeight(BLayoutItem* item, float weight);

			bool				IsCollapsible(int32_t index) const;
			void				SetCollapsible(bool collapsible);
			void				SetCollapsible(int32_t index, bool collapsible);
			void				SetCollapsible(int32_t first, int32_t last,
									bool collapsible);

			bool				IsItemCollapsed(int32_t index) const;
			void				SetItemCollapsed(int32_t index, bool visible);

	virtual	BSize				BaseMinSize();
	virtual	BSize				BaseMaxSize();
	virtual	BSize				BasePreferredSize();
	virtual	BAlignment			BaseAlignment();

	virtual	bool				HasHeightForWidth();
	virtual	void				GetHeightForWidth(float width, float* min,
									float* max, float* preferred);

	virtual	void				LayoutInvalidated(bool children);
	virtual	void				DoLayout();

	// interface for BSplitView
			BRect				SplitterItemFrame(int32_t index) const;
			bool				IsAboveSplitter(const BPoint& point) const;

			bool				StartDraggingSplitter(BPoint point);
			bool				DragSplitter(BPoint point);
			bool				StopDraggingSplitter();
			int32_t				DraggedSplitter() const;

	// archiving methods
	virtual status_t			Archive(BMessage* into, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* from);

	virtual status_t			ItemArchived(BMessage* into, BLayoutItem* item,
									int32_t index) const;
	virtual	status_t			ItemUnarchived(const BMessage* from,
									BLayoutItem* item, int32_t index);

protected:
	virtual	bool				ItemAdded(BLayoutItem* item, int32_t atIndex);
	virtual	void				ItemRemoved(BLayoutItem* item, int32_t fromIndex);

private:
			class ItemLayoutInfo;
			class ValueRange;
			class SplitterItem;

			void				_InvalidateLayout(bool invalidateView,
									bool children = false);
			void				_InvalidateCachedHeightForWidth();

			SplitterItem*		_SplitterItemAt(const BPoint& point,
									int32_t* index = NULL) const;
			SplitterItem*		_SplitterItemAt(int32_t index) const;

			void				_GetSplitterValueRange(int32_t index,
									ValueRange& range);
			int32_t				_SplitterValue(int32_t index) const;

			void				_LayoutItem(BLayoutItem* item, BRect frame,
									bool visible);
			void				_LayoutItem(BLayoutItem* item,
									ItemLayoutInfo* info);

			bool				_SetSplitterValue(int32_t index, int32_t value);

			ItemLayoutInfo*		_ItemLayoutInfo(BLayoutItem* item) const;


			void				_UpdateSplitterWeights();

			void				_ValidateMinMax();

			void				_InternalGetHeightForWidth(float width,
									bool realLayout, float* minHeight,
									float* maxHeight, float* preferredHeight);

			float				_SplitterSpace() const;

			BSize				_AddInsets(BSize size);
			void				_AddInsets(float* minHeight, float* maxHeight,
									float* preferredHeight);
			BSize				_SubtractInsets(BSize size);

private:
			orientation			fOrientation;
			float				fLeftInset;
			float				fRightInset;
			float				fTopInset;
			float				fBottomInset;
			float				fSplitterSize;
			float				fSpacing;

			BList				fSplitterItems;
			BList				fVisibleItems;

			BSize				fMin;
			BSize				fMax;
			BSize				fPreferred;

			Layouter*			fHorizontalLayouter;
			Layouter*			fVerticalLayouter;

			LayoutInfo*			fHorizontalLayoutInfo;
			LayoutInfo*			fVerticalLayoutInfo;

			BList				fHeightForWidthItems;
			// Incorporates the children's height for width constraints for a
			// concrete width. Cloned lazily from fVerticalLayout when needed.
			Layouter*			fHeightForWidthVerticalLayouter;
			LayoutInfo*			fHeightForWidthHorizontalLayoutInfo;
				// for computing height for width info

			bool				fLayoutValid;

			float				fCachedHeightForWidthWidth;
			float				fHeightForWidthVerticalLayouterWidth;
			float				fCachedMinHeightForWidth;
			float				fCachedMaxHeightForWidth;
			float				fCachedPreferredHeightForWidth;

			BPoint				fDraggingStartPoint;
			int32_t				fDraggingStartValue;
			int32_t				fDraggingCurrentValue;
			int32_t				fDraggingSplitterIndex;
};

#endif	// _SPLIT_LAYOUT_H
