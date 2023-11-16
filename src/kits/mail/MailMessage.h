/*
 * Copyright 2007-2012, Haiku Inc. All Rights Reserved.
 * Copyright 2001, Dr. Zoidberg Enterprises. All rights reserved.
 *
 * Distributed under the terms of the MIT License.
 */
#ifndef _MAIL_MESSAGE_H_
#define _MAIL_MESSAGE_H_


//! The main general purpose mail message class


#include <MailContainer.h>


// add our additional attributes
#define B_MAIL_ATTR_ACCOUNT "MAIL:account"
#define B_MAIL_ATTR_THREAD "MAIL:thread"


class BDirectory;
class BEntry;


enum mail_reply_to_mode {
	B_MAIL_REPLY_TO = 0,
	B_MAIL_REPLY_TO_ALL,
	B_MAIL_REPLY_TO_SENDER
};


class BEmailMessage : public BMailContainer {
public:
								BEmailMessage(BPositionIO* stream = NULL,
									bool ownStream = false,
									uint32_t defaultCharSet
										= B_MAIL_NULL_CONVERSION);
								BEmailMessage(const entry_ref* ref,
									uint32_t defaultCharSet
										= B_MAIL_NULL_CONVERSION);
	virtual						~BEmailMessage();

			status_t			InitCheck() const;
			BPositionIO*		Data() const { return fData; }
				// is only set if the message owns the data

			BEmailMessage*		ReplyMessage(mail_reply_to_mode replyTo,
									bool accountFromMail,
									const char* quoteStyle = "> ");
			BEmailMessage*		ForwardMessage(bool accountFromMail,
									bool includeAttachments = false);
				// These return messages with the body quoted and
				// ready to send via the appropriate channel. ReplyMessage()
				// addresses the message appropriately, but ForwardMessage()
				// leaves it unaddressed.

			const char*			To() const;
			const char*			From() const;
			const char*			ReplyTo() const;
			const char*			CC() const;
			const char*			Subject() const;
			time_t				Date() const;
			int					Priority() const;

			void				SetSubject(const char* to,
									uint32_t charset = B_MAIL_NULL_CONVERSION,
									mail_encoding encoding = null_encoding);
			void				SetReplyTo(const char* to,
									uint32_t charset = B_MAIL_NULL_CONVERSION,
									mail_encoding encoding = null_encoding);
			void				SetFrom(const char* to,
									uint32_t charset = B_MAIL_NULL_CONVERSION,
									mail_encoding encoding = null_encoding);
			void				SetTo(const char* to,
									uint32_t charset = B_MAIL_NULL_CONVERSION,
									mail_encoding encoding = null_encoding);
			void				SetCC(const char* to,
									uint32_t charset = B_MAIL_NULL_CONVERSION,
									mail_encoding encoding = null_encoding);
			void				SetBCC(const char* to);
			void				SetPriority(int to);

			status_t			GetName(char* name, int32_t maxLength) const;
			status_t			GetName(BString* name) const;

			void				SendViaAccountFrom(BEmailMessage* message);
			void				SendViaAccount(const char* accountName);
			void				SendViaAccount(int32_t account);
			int32_t				Account() const;
			status_t			GetAccountName(BString& accountName) const;

	virtual	status_t			AddComponent(BMailComponent *component);
	virtual	status_t			RemoveComponent(BMailComponent *component);
	virtual	status_t			RemoveComponent(int32_t index);

	virtual	BMailComponent*		GetComponent(int32_t index,
									bool parseNow = false);
	virtual	int32_t				CountComponents() const;

			void				Attach(entry_ref* ref,
									bool includeAttributes = true);
			bool				IsComponentAttachment(int32_t index);

			void				SetBodyTextTo(const char* text);
			const char*			BodyText();

			status_t			SetBody(BTextMailComponent* body);
			BTextMailComponent*	Body();

	virtual	status_t			SetToRFC822(BPositionIO* data, size_t length,
									bool parseNow = false);
	virtual	status_t			RenderToRFC822(BPositionIO* renderTo);

			status_t			RenderTo(BDirectory* dir,
									BEntry* message = NULL);
				// Message will be set to the message file if not equal to NULL

			status_t			Send(bool sendNow);

private:
			BTextMailComponent*	_RetrieveTextBody(BMailComponent* component);

	virtual	void				_ReservedMessage1();
	virtual	void				_ReservedMessage2();
	virtual	void				_ReservedMessage3();

private:
			BPositionIO*		fData;

			status_t			fStatus;
			int32_t				fAccountID;
			char*				fBCC;

			int32_t				fComponentCount;
			BMailComponent*		fBody;
			BTextMailComponent*	fTextBody;

			uint32_t				_reserved[5];
};


#endif	// _MAIL_MESSAGE_H_
