/*
 * Copyright 2002-2006, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Tyler Dauwalder
 */
#ifndef _MIME_DATABASE_H
#define _MIME_DATABASE_H

#include <map>
#include <set>
#include <kits/support/String>

#include <kits/support/List.h>
#include <Locker.h>
#include <Mime.h>
#include <Messenger.h>
#include <StorageDefs.h>

#include <mime/AssociatedTypes.h>
#include <mime/InstalledTypes.h>
#include <mime/SnifferRules.h>
#include <mime/SupportingApps.h>


class BNode;
class BBitmap;
class BMessage;
class BString;

struct entry_ref;

namespace BPrivate {
namespace Storage {
namespace Mime {


class DatabaseLocation;
class MimeSniffer;


// types of mime update functions that may be run asynchronously
typedef enum {
	B_REG_UPDATE_MIME_INFO,
	B_REG_CREATE_APP_META_MIME,
} mime_update_function;

class Database {
	public:
		class NotificationListener;

	public:
		Database(DatabaseLocation* databaseLocation, MimeSniffer* mimeSniffer,
			NotificationListener* notificationListener);
		~Database();

		status_t InitCheck() const;

		DatabaseLocation* Location() const	{ return fLocation; }

		// Type management
		status_t Install(const char *type);
		status_t Delete(const char *type);

		// Set()
		status_t SetAppHint(const char *type, const entry_ref *ref);
		status_t SetAttrInfo(const char *type, const BMessage *info);
		status_t SetShortDescription(const char *type, const char *description);	
		status_t SetLongDescription(const char *type, const char *description);
		status_t SetFileExtensions(const char *type, const BMessage *extensions);
		status_t SetIcon(const char* type, const BBitmap* icon,
					icon_size which);
		status_t SetIcon(const char *type, const void *data, size_t dataSize,
					icon_size which);
		status_t SetIcon(const char *type, const void *data, size_t dataSize);
		status_t SetIconForType(const char* type, const char* fileType,
					const BBitmap* icon, icon_size which);
		status_t SetIconForType(const char *type, const char *fileType,
					const void *data, size_t dataSize, icon_size which);
		status_t SetIconForType(const char *type, const char *fileType,
					const void *data, size_t dataSize);
		status_t SetPreferredApp(const char *type, const char *signature,
					app_verb verb = B_OPEN);
		status_t SetSnifferRule(const char *type, const char *rule);
		status_t SetSupportedTypes(const char *type, const BMessage *types, bool fullSync);

		// Non-atomic Get()
		status_t GetInstalledSupertypes(BMessage *super_types);
		status_t GetInstalledTypes(BMessage *types);
		status_t GetInstalledTypes(const char *super_type,
					BMessage *subtypes);
		status_t GetSupportingApps(const char *type, BMessage *signatures);
		status_t GetAssociatedTypes(const char *extension, BMessage *types);

		// Sniffer
		status_t GuessMimeType(const entry_ref *file, BString *result);
		status_t GuessMimeType(const void *buffer, int32_t length, BString *result);
		status_t GuessMimeType(const char *filename, BString *result);

		// Monitor
		status_t StartWatching(BMessenger target);
		status_t StopWatching(BMessenger target);

		// Delete()
		status_t DeleteAppHint(const char *type);
		status_t DeleteAttrInfo(const char *type);
		status_t DeleteShortDescription(const char *type);
		status_t DeleteLongDescription(const char *type);
		status_t DeleteFileExtensions(const char *type);
		status_t DeleteIcon(const char *type, icon_size size);
		status_t DeleteIcon(const char *type);
		status_t DeleteIconForType(const char *type, const char *fileType,
					icon_size which);
		status_t DeleteIconForType(const char *type, const char *fileType);
		status_t DeletePreferredApp(const char *type, app_verb verb = B_OPEN);
		status_t DeleteSnifferRule(const char *type);
		status_t DeleteSupportedTypes(const char *type, bool fullSync);

		// deferred notifications
		void	DeferInstallNotification(const char* type);
		void	UndeferInstallNotification(const char* type);

	private:
		struct DeferredInstallNotification {
			char	type[B_MIME_TYPE_LENGTH];
			bool	notify;
		};

		status_t _SetStringValue(const char *type, int32_t what,
					const char* attribute, type_code attributeType,
					size_t maxLength, const char *value);

		// Functions to send monitor notifications
		status_t _SendInstallNotification(const char *type);
		status_t _SendDeleteNotification(const char *type);	
		status_t _SendMonitorUpdate(int32_t which, const char *type,
					const char *extraType, bool largeIcon, int32_t action);
		status_t _SendMonitorUpdate(int32_t which, const char *type,
					const char *extraType, int32_t action);
		status_t _SendMonitorUpdate(int32_t which, const char *type,
					bool largeIcon, int32_t action);
		status_t _SendMonitorUpdate(int32_t which, const char *type,
					int32_t action);
		status_t _SendMonitorUpdate(BMessage &msg);

		DeferredInstallNotification* _FindDeferredInstallNotification(
			const char* type, bool remove = false);
		bool _CheckDeferredInstallNotification(int32_t which, const char* type);

	private:
		status_t fStatus;
		DatabaseLocation* fLocation;
		NotificationListener* fNotificationListener;
		std::set<BMessenger> fMonitorMessengers;
		AssociatedTypes fAssociatedTypes;
		InstalledTypes fInstalledTypes;
		SnifferRules fSnifferRules;
		SupportingApps fSupportingApps;

		BLocker	fDeferredInstallNotificationsLocker;
		BList	fDeferredInstallNotifications;
};

class InstallNotificationDeferrer {
	public:
		InstallNotificationDeferrer(Database* database, const char* type)
			:
			fDatabase(database),
			fType(type)
		{
			if (fDatabase != NULL && fType != NULL)
				fDatabase->DeferInstallNotification(fType);
		}

		~InstallNotificationDeferrer()
		{
			if (fDatabase != NULL && fType != NULL)
				fDatabase->UndeferInstallNotification(fType);
		}

	private:
		Database*	fDatabase;
		const char*	fType;
};


class Database::NotificationListener {
public:
	virtual						~NotificationListener();

	virtual	status_t			Notify(BMessage* message,
									const BMessenger& target) = 0;
};


} // namespace Mime
} // namespace Storage
} // namespace BPrivate

#endif	// _MIME_DATABASE_H
