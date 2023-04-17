/*
 * Copyright 2001-2009, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _SHELF_H
#define _SHELF_H


#include <Dragger.h>
#include <Handler.h>
#include <kits/support/List.h>
#include <Locker.h>


class BDataIO;
class BPoint;
class BView;
class BEntry;
class _BZombieReplicantView_;
struct entry_ref;

namespace BPrivate {
	class replicant_data;
	class ShelfContainerViewFilter;
};


class BShelf : public BHandler {
public:
								BShelf(BView* view, bool allowDrags = true,
									const char* shelfType = NULL);
								BShelf(const entry_ref* ref, BView* view,
									bool allowDrags = true,
									const char* shelfType = NULL);
								BShelf(BDataIO* stream, BView* view,
									bool allowDrags = true,
									const char* shelfType = NULL);
								BShelf(BMessage* archive);
	virtual						~BShelf();

	static BArchivable*			Instantiate(BMessage* archive);
	virtual	status_t			Archive(BMessage* archive,
									bool deep = true) const;

	virtual	void				MessageReceived(BMessage* message);
			status_t			Save();
	virtual	void				SetDirty(bool state);
			bool				IsDirty() const;

	virtual	BHandler*			ResolveSpecifier(BMessage* message,
									int32_t index, BMessage* specifier,
									int32_t form, const char* property);
	virtual	status_t			GetSupportedSuites(BMessage* data);

	virtual	status_t			Perform(perform_code code, void* data);

			bool				AllowsDragging() const;
			void				SetAllowsDragging(bool state);
			bool				AllowsZombies() const;
			void				SetAllowsZombies(bool state);
			bool				DisplaysZombies() const;
			void				SetDisplaysZombies(bool state);
			bool				IsTypeEnforced() const;
			void				SetTypeEnforced(bool state);

			status_t			SetSaveLocation(BDataIO* stream);
			status_t			SetSaveLocation(const entry_ref* ref);
			BDataIO*			SaveLocation(entry_ref* ref) const;

			status_t			AddReplicant(BMessage* archive,
									BPoint location);
			status_t			DeleteReplicant(BView* replicant);
			status_t			DeleteReplicant(BMessage* archive);
			status_t			DeleteReplicant(int32_t index);
			int32_t				CountReplicants() const;
			BMessage*			ReplicantAt(int32_t index, BView** view = NULL,
									uint32_t* uid = NULL,
									status_t* perr = NULL) const;
			int32_t				IndexOf(const BView* replicantView) const;
			int32_t				IndexOf(const BMessage* archive) const;
			int32_t				IndexOf(uint32_t id) const;

protected:
	virtual	bool				CanAcceptReplicantMessage(
									BMessage* archive) const;
	virtual	bool				CanAcceptReplicantView(BRect,
									BView*, BMessage*) const;
	virtual	BPoint				AdjustReplicantBy(BRect, BMessage*) const;

	virtual	void				ReplicantDeleted(int32_t index,
									const BMessage* archive,
									const BView *replicant);

private:
	// FBC padding and forbidden methods
	virtual	void				_ReservedShelf2();
	virtual	void				_ReservedShelf3();
	virtual	void				_ReservedShelf4();
	virtual	void				_ReservedShelf5();
	virtual	void				_ReservedShelf6();
	virtual	void				_ReservedShelf7();
	virtual	void				_ReservedShelf8();

								BShelf(const BShelf& other);
			BShelf&				operator=(const BShelf& other);

private:
	friend class BPrivate::ShelfContainerViewFilter;

			status_t			_Archive(BMessage* data) const;
			void				_InitData(BEntry* entry, BDataIO* stream,
									BView* view, bool allowDrags);
			status_t			_DeleteReplicant(
									BPrivate::replicant_data* replicant);
			status_t			_AddReplicant(BMessage* data,
									BPoint* location, uint32_t uniqueID);
			BView*				_GetReplicant(BMessage* data, BView* view,
									const BPoint& point, BDragger*& dragger,
									BDragger::relation& relation);		
			_BZombieReplicantView_* _CreateZombie(BMessage *data,
									BDragger *&dragger);
			
			status_t			_GetProperty(BMessage* message,
									BMessage* reply);
	static	void				_GetReplicantData(BMessage* message,
									BView* view, BView*& replicant,
									BDragger*& dragger,
									BDragger::relation& relation);
	static	BArchivable*		_InstantiateObject(BMessage* archive,
									image_id* image);

private:
			BView*				fContainerView;
			BDataIO*			fStream;
			BEntry*				fEntry;
			BList				fReplicants;
			BPrivate::ShelfContainerViewFilter* fFilter;
			uint32_t				fGenCount;
			bool				fAllowDragging;
			bool				fDirty;
			bool				fDisplayZombies;
			bool				fAllowZombies;
			bool				fTypeEnforced;
	
			uint32_t				_reserved[8];
};

#endif	/* _SHELF_H */
