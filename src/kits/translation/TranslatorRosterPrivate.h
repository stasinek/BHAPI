/*
 * Copyright 2006-2015, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Author:
 *		Axel Dörfler, axeld@pinc-software.de
 */
#ifndef TRANSLATOR_ROSTER_PRIVATE_H
#define TRANSLATOR_ROSTER_PRIVATE_H

#include <map>
#include <set>
#include <vector>

#include <Entry.h>
#include <Handler.h>
#include <Locker.h>
#include <Messenger.h>
#include <TranslatorRoster.h>


struct translator_data;


struct translator_item {
	BTranslator*	translator;
	entry_ref		ref;
	ino_t			node;
	image_id		image;
};

typedef std::map<translator_id, translator_item> TranslatorMap;
typedef std::vector<BMessenger> MessengerList;
typedef std::vector<node_ref> NodeRefList;
typedef std::set<entry_ref> EntryRefSet;
typedef std::map<image_id, int32_t> ImageMap;
typedef std::map<BTranslator*, image_id> TranslatorImageMap;


class BTranslatorRoster::Private : public BHandler, public BLocker {
public:
								Private();
	virtual						~Private();

	virtual	void				MessageReceived(BMessage* message);

			void				AddDefaultPaths();
			status_t			AddPaths(const char* paths);
			status_t			AddPath(const char* path, int32_t* _added = NULL);
			status_t			AddTranslator(BTranslator* translator,
									image_id image = -1,
									const entry_ref* ref = NULL,
									ino_t node = 0);

			void				RemoveTranslators(entry_ref& ref);

			BTranslator*		FindTranslator(translator_id id);

			status_t			StoreTranslators(BMessage& archive);
			status_t			Identify(BPositionIO* source,
									BMessage* ioExtension, uint32_t hintType,
									const char* hintMIME, uint32_t wantType,
									translator_info* _info);

			status_t			GetTranslators(BPositionIO* source,
									BMessage* ioExtension, uint32_t hintType,
									const char* hintMIME, uint32_t wantType,
									translator_info** _info, int32_t* _numInfo);
			status_t			GetAllTranslators(translator_id** _ids,
									int32_t* _count);
			status_t			GetRefFor(translator_id id, entry_ref& ref);

			bool				IsActive() const { return Looper(); }

			status_t			CreateTranslators(const entry_ref& ref,
									int32_t& count, BMessage* update = NULL);
			status_t			GetTranslatorData(image_id image,
									translator_data& data);

			status_t			StartWatching(BMessenger target);
			status_t			StopWatching(BMessenger target);

private:
	static	int					_CompareSupport(const void* _a, const void* _b);

			void				_RescanChanged();

			const translation_format* _CheckHints(
									const translation_format* formats,
									int32_t formatsCount, uint32_t hintType,
									const char* hintMIME);

			const translator_item* _FindTranslator(translator_id id) const;
			const translator_item* _FindTranslator(const char* name) const;
			const translator_item* _FindTranslator(entry_ref& ref) const;
			translator_item*	_FindTranslator(node_ref& nodeRef);

			int32_t				_CompareTranslatorDirectoryPriority(
									const entry_ref& a,
									const entry_ref& b) const;
			bool				_IsKnownDirectory(const node_ref& nodeRef)
									const;

			void				_RemoveTranslators(const node_ref* nodeRef,
									const entry_ref* ref = NULL);
			void				_EntryAdded(const node_ref& nodeRef,
									const char* name);
			void				_EntryAdded(const entry_ref& ref);
			void				_NotifyListeners(BMessage& update) const;
			void				_TranslatorDeleted(translator_id id,
									BTranslator *self);

			NodeRefList			fDirectories;
			TranslatorMap		fTranslators;
			MessengerList		fMessengers;
			EntryRefSet			fRescanEntries;
			ImageMap			fKnownImages;
			TranslatorImageMap	fImageOrigins;
			const char*			fABISubDirectory;
			int32_t				fNextID;
			bool				fLazyScanning;
			bool				fSafeMode;
};


#endif	// TRANSLATOR_ROSTER_PRIVATE_H
