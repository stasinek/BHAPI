/*
 * Copyright 2012, Haiku Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _MAIL_DAEMON_H
#define _MAIL_DAEMON_H


#include <E-mail.h>
#include <Messenger.h>


enum {
	B_MAIL_BODY_FETCHED = '_Mbf'
};


class BMailDaemon {
public:
								BMailDaemon();
	virtual						~BMailDaemon();

			bool				IsRunning();

			status_t			CheckMail(int32_t accountID = -1);
			status_t			CheckAndSendQueuedMail(int32_t accountID = -1);

			status_t			SendQueuedMail();

			int32_t				CountNewMessages(
									bool waitForFetchCompletion = false);
			status_t			MarkAsRead(int32_t account, const entry_ref& ref,
									read_flags flag = B_READ);
			status_t			FetchBody(const entry_ref& ref,
									BMessenger* listener = NULL);

			status_t			Quit();
			status_t			Launch();

private:
			BMessenger			fDaemon;
};


#endif	// _MAIL_DAEMON_H
