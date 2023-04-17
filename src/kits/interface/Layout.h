/*
 * Copyright 2006-2010, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef	_LAYOUT_H
#define	_LAYOUT_H


#include <Alignment.h>
#include <Archivable.h>
#include <LayoutItem.h>
#include <kits/support/List.h>
#include <Size.h>


class BLayoutContext;
class BLayoutItem;
class BView;


class BLayout : public BLayoutItem {
public:
								BLayout();
								BLayout(BMessage* archive);
	virtual						~BLayout();

			BView*				Owner() const;
			BView*				TargetView() const;
	virtual	BView*				View(); // from BLayoutItem

	// methods dealing with items
	virtual	BLayoutItem*		AddView(BView* child);
	virtual	BLayoutItem*		AddView(int32_t index, BView* child);

	virtual	bool				AddItem(BLayoutItem* item);
	virtual	bool				AddItem(int32_t index, BLayoutItem* item);

	virtual	bool				RemoveView(BView* child);
	virtual	bool				RemoveItem(BLayoutItem* item);
	virtual	BLayoutItem*		RemoveItem(int32_t index);

			BLayoutItem*		ItemAt(int32_t index) const;
			int32_t				CountItems() const;
			int32_t				IndexOfItem(const BLayoutItem* item) const;
			int32_t				IndexOfView(BView* child) const;

			bool				AncestorsVisible() const;

	// Layouting related methods

	virtual	void				InvalidateLayout(bool children = false);
	virtual	void				Relayout(bool immediate = false);
									// from BLayoutItem
			void				RequireLayout();
			bool				IsValid();
			void				EnableLayoutInvalidation();
			void				DisableLayoutInvalidation();

			void				LayoutItems(bool force = false);
			BRect				LayoutArea();
			BLayoutContext*		LayoutContext() const;

	virtual status_t			Archive(BMessage* into, bool deep = true) const;

	virtual status_t			Perform(perform_code d, void* arg);

protected:
	// Archiving hook methods
	virtual	status_t			AllArchived(BMessage* archive) const;
	virtual	status_t			AllUnarchived(const BMessage* from);

	virtual status_t			ItemArchived(BMessage* into, BLayoutItem* item,
									int32_t index) const;
	virtual	status_t			ItemUnarchived(const BMessage* from,
									BLayoutItem* item, int32_t index);
	// BLayout hook methods
	virtual	bool				ItemAdded(BLayoutItem* item, int32_t atIndex);
	virtual	void				ItemRemoved(BLayoutItem* item, int32_t fromIndex);
	virtual	void				LayoutInvalidated(bool children);
	virtual	void				DoLayout() = 0;
	virtual	void				OwnerChanged(BView* was);

	// BLayoutItem hook methods
	virtual	void				AttachedToLayout();
	virtual void				DetachedFromLayout(BLayout* layout);
	virtual	void				AncestorVisibilityChanged(bool shown);

	// To be called by sub-classes in SetVisible().
			void				VisibilityChanged(bool show);
	// To be called when layout data is known to be good
			void				ResetLayoutInvalidation();

private:

	// FBC padding
	virtual	void				_ReservedLayout1();
	virtual	void				_ReservedLayout2();
	virtual	void				_ReservedLayout3();
	virtual	void				_ReservedLayout4();
	virtual	void				_ReservedLayout5();
	virtual	void				_ReservedLayout6();
	virtual	void				_ReservedLayout7();
	virtual	void				_ReservedLayout8();
	virtual	void				_ReservedLayout9();
	virtual	void				_ReservedLayout10();

	// forbidden methods
								BLayout(const BLayout&);
			void				operator =(const BLayout&);

			friend class BView;

			void				SetOwner(BView* owner);
			void				SetTarget(BView* target);

			void				_LayoutWithinContext(bool force,
									BLayoutContext* context);

			uint32_t				fState;
			bool				fAncestorsVisible;
			int32_t				fInvalidationDisabled;
			BLayoutContext*		fContext;
			BView*				fOwner;
			BView*				fTarget;
			BList				fItems;
			BList				fNestedLayouts;

			uint32_t				_reserved[10];
};


#endif	//	_LAYOUT_H
