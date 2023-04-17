/*
 * Copyright 2006-2010, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef	_SPLIT_VIEW_H
#define	_SPLIT_VIEW_H

#include <View.h>

class BSplitLayout;


class BSplitView : public BView {
public:
								BSplitView(
									orientation orientation = B_HORIZONTAL,
									float spacing = B_USE_DEFAULT_SPACING);
								BSplitView(BMessage* from);
	virtual						~BSplitView();

			void				SetInsets(float left, float top, float right,
									float bottom);
			void				SetInsets(float horizontal, float vertical);
			void				SetInsets(float insets);
			void				GetInsets(float* left, float* top,
									float* right, float* bottom) const;

			float				Spacing() const;
			void				SetSpacing(float spacing);

			orientation			Orientation() const;
			void				SetOrientation(orientation orientation);

			float				SplitterSize() const;
			void				SetSplitterSize(float size);

			int32_t				CountItems() const;
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
			void				SetItemCollapsed(int32_t index, bool collapsed);

//			void				AddChild(BView* child);
			void				AddChild(BView* child, BView* sibling = NULL);
			bool				AddChild(BView* child, float weight);
			bool				AddChild(int32_t index, BView* child,
									float weight);

			bool				AddChild(BLayoutItem* child);
			bool				AddChild(BLayoutItem* child, float weight);
			bool				AddChild(int32_t index, BLayoutItem* child,
									float weight);

	virtual	void				Draw(BRect updateRect);
	virtual	void				DrawAfterChildren(BRect updateRect);
	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32_t transit,
									const BMessage* message);

	virtual	void				MessageReceived(BMessage* message);


	virtual	void				SetLayout(BLayout* layout);
									// overridden to avoid use

	virtual status_t			Archive(BMessage* into, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* from);

	virtual	status_t			Perform(perform_code d, void* arg);

protected:
	virtual	status_t			AllArchived(BMessage* into) const;
	virtual status_t			AllUnarchived(const BMessage* from);

	virtual	void				DrawSplitter(BRect frame,
									const BRect& updateRect,
									orientation orientation,
									bool pressed);

private:

	// FBC padding
	virtual	void				_ReservedSplitView1();
	virtual	void				_ReservedSplitView2();
	virtual	void				_ReservedSplitView3();
	virtual	void				_ReservedSplitView4();
	virtual	void				_ReservedSplitView5();
	virtual	void				_ReservedSplitView6();
	virtual	void				_ReservedSplitView7();
	virtual	void				_ReservedSplitView8();
	virtual	void				_ReservedSplitView9();
	virtual	void				_ReservedSplitView10();

	// forbidden methods
								BSplitView(const BSplitView&);
			void				operator =(const BSplitView&);

	static	void				_DrawDefaultSplitter(BView* view, BRect frame,
									const BRect& updateRect,
									orientation orientation,
									bool pressed);

private:
			BSplitLayout*		fSplitLayout;
			uint32_t				_reserved[3];
};


#endif	// _SPLIT_VIEW_H
