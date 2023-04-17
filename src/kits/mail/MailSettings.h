/*
 * Copyright 2004-2012, Haiku Inc. All rights reserved.
 * Copyright 2001 Dr. Zoidberg Enterprises. All rights reserved.
 * Copyright 2011 Clemens Zeidler.
 *
 * Distributed under the terms of the MIT License.
 */
#ifndef MAIL_SETTINGS_H
#define MAIL_SETTINGS_H


#include <vector>

#include <Archivable.h>
#include <Entry.h>
#include <kits/support/List.h>
#include <Message.h>
#include <ObjectList.h>
#include <kits/support/String.h>


class BPath;


typedef enum {
	B_MAIL_SHOW_STATUS_WINDOW_NEVER         = 0,
	B_MAIL_SHOW_STATUS_WINDOW_WHEN_SENDING	= 1,
	B_MAIL_SHOW_STATUS_WINDOW_WHEN_ACTIVE	= 2,
	B_MAIL_SHOW_STATUS_WINDOW_ALWAYS        = 3
} b_mail_status_window_option;


class BMailSettings {
public:
								BMailSettings();
								~BMailSettings();

			status_t			Save();
			status_t			Reload();
			status_t			InitCheck() const;

			// Global settings
			int32_t				WindowFollowsCorner();
			void				SetWindowFollowsCorner(int32_t which_corner);

			uint32_t				ShowStatusWindow();
			void				SetShowStatusWindow(uint32_t mode);

			bool				DaemonAutoStarts();
			void				SetDaemonAutoStarts(bool does_it);

			void				SetConfigWindowFrame(BRect frame);
			BRect				ConfigWindowFrame();

			void				SetStatusWindowFrame(BRect frame);
			BRect				StatusWindowFrame();

			int32_t				StatusWindowWorkspaces();
			void				SetStatusWindowWorkspaces(int32_t workspaces);

			int32_t				StatusWindowLook();
			void				SetStatusWindowLook(int32_t look);

			bigtime_t			AutoCheckInterval();
			void				SetAutoCheckInterval(bigtime_t);

			bool				CheckOnlyIfPPPUp();
			void				SetCheckOnlyIfPPPUp(bool yes);

			bool				SendOnlyIfPPPUp();
			void				SetSendOnlyIfPPPUp(bool yes);

			int32_t				DefaultOutboundAccount();
			void				SetDefaultOutboundAccount(int32_t to);

private:
			BMessage			fData;
			uint32_t				_reserved[4];
};


class BMailAddOnSettings : public BMessage {
public:
								BMailAddOnSettings();
	virtual						~BMailAddOnSettings();

	virtual status_t			Load(const BMessage& message);
	virtual	status_t			Save(BMessage& message);

			void				SetAddOnRef(const entry_ref& ref);
			const entry_ref&	AddOnRef() const;

	virtual	bool				HasBeenModified() const;

private:
			const char*			_RelativizePath(const BPath& path) const;

private:
			BMessage			fOriginalSettings;
			entry_ref			fRef;
			entry_ref			fOriginalRef;
};


class BMailProtocolSettings : public BMailAddOnSettings {
public:
								BMailProtocolSettings();
	virtual						~BMailProtocolSettings();

	virtual	status_t			Load(const BMessage& message);
	virtual	status_t			Save(BMessage& message);

			int32_t				CountFilterSettings() const;
			int32_t				AddFilterSettings(const entry_ref* ref = NULL);
			void				RemoveFilterSettings(int32_t index);
			bool				MoveFilterSettings(int32_t from, int32_t to);
			BMailAddOnSettings*	FilterSettingsAt(int32_t index) const;

	virtual	bool				HasBeenModified() const;

private:
			BObjectList<BMailAddOnSettings> fFiltersSettings;
};


class BMailAccountSettings {
public:
								BMailAccountSettings();
								BMailAccountSettings(BEntry account);
								~BMailAccountSettings();

			status_t			InitCheck() { return fStatus; }

			void				SetAccountID(int32_t id);
			int32_t				AccountID() const;

			void				SetName(const char* name);
			const char*			Name() const;

			void				SetRealName(const char* realName);
			const char*			RealName() const;

			void				SetReturnAddress(const char* returnAddress);
			const char*			ReturnAddress() const;

			bool				SetInboundAddOn(const char* name);
			bool				SetOutboundAddOn(const char* name);
			const entry_ref&	InboundAddOnRef() const;
			const entry_ref&	OutboundAddOnRef() const;

			BMailProtocolSettings& InboundSettings();
			const BMailProtocolSettings& InboundSettings() const;
			BMailProtocolSettings& OutboundSettings();
			const BMailProtocolSettings& OutboundSettings() const;

			bool				HasInbound();
			bool				HasOutbound();

			void				SetInboundEnabled(bool enabled = true);
			bool				IsInboundEnabled() const;
			void				SetOutboundEnabled(bool enabled = true);
			bool				IsOutboundEnabled() const;

			status_t			Reload();
			status_t			Save();
			status_t			Delete();

			bool				HasBeenModified() const;

			const BEntry&		AccountFile() const;

private:
			status_t			_CreateAccountFilePath();
			status_t			_GetAddOnRef(const char* subPath,
									const char* name, entry_ref& ref);

private:
			status_t			fStatus;
			BEntry				fAccountFile;

			int32_t				fAccountID;

			BString				fAccountName;
			BString				fRealName;
			BString				fReturnAdress;

			BMailProtocolSettings fInboundSettings;
			BMailProtocolSettings fOutboundSettings;

			bool				fInboundEnabled;
			bool				fOutboundEnabled;

			bool				fModified;
};


class BMailAccounts {
public:
								BMailAccounts();
								~BMailAccounts();

	static	status_t			AccountsPath(BPath& path);

			int32_t				CountAccounts();
			BMailAccountSettings*	AccountAt(int32_t index);

			BMailAccountSettings*	AccountByID(int32_t id);
			BMailAccountSettings*	AccountByName(const char* name);
private:
			BObjectList<BMailAccountSettings>	fAccounts;
};


#endif	/* ZOIDBERG_MAIL_SETTINGS_H */
