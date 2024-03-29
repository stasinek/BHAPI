/*
 * Copyright 2011-2012, Haiku Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef MAIL_PRIVATE_H
#define MAIL_PRIVATE_H


#include <Message.h>
#include <Path.h>


namespace BPrivate {


#define B_MAIL_DAEMON_SIGNATURE "application/x-vnd.Be-POST"


// mail daemon & protocol message constants
const uint32_t kMsgCheckAndSend = 'mbth';
const uint32_t kMsgCheckMessage = 'mnow';
const uint32_t kMsgSendMessages = 'msnd';
const uint32_t kMsgSettingsUpdated = 'mrrs';
const uint32_t kMsgAccountsChanged = 'macc';
const uint32_t kMsgCountNewMessages = 'mnum';
const uint32_t kMsgMarkMessageAsRead = 'mmar';
const uint32_t kMsgFetchBody = 'mfeb';
const uint32_t kMsgSyncMessages = '&SyM';


BPath default_mail_directory();
BPath default_mail_in_directory();
BPath default_mail_out_directory();


status_t WriteMessageFile(const BMessage& archive, const BPath& path,
	const char* name);


}	// namespace BPrivate


#endif	// MAIL_PRIVATE_H
