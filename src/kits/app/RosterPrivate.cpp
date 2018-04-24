/*
 * Copyright 2001-2015, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ingo Weinhold (bonefish@users.sf.net)
 */


//! Access class for BRoster members


#include <kits/netRosterPrivate.h>

#include <kits/netRoster.h>

#include <kits/netlocks.h>


/*!	\class BRoster::Private
	\brief Class used to access private BRoster members.

	This way, the only friend BRoster needs is this class.
*/


/*!	\brief Initializes the roster.

	\param mainMessenger A BMessenger targeting the registrar application.
	\param mimeMessenger A BMessenger targeting the MIME manager.
*/
void BRoster::Private::SetTo(BMessenger mainMessenger, BMessenger mimeMessenger)
{
	if (fRoster != NULL) {
		fRoster->fMessenger = mainMessenger;
		fRoster->fMimeMessenger = mimeMessenger;
		fRoster->fMimeMessengerInitOnce = INIT_ONCE_INITIALIZED;
	}
}


/*!	\brief Sends a message to the registrar.

	\a mime specifies whether to send the message to the roster or to the
	MIME data base service.
	If \a reply is not \c NULL, the function waits for a reply.

	\param message The message to be sent.
	\param reply A pointer to a pre-allocated BMessage into which the reply
		   message will be copied. May be \c NULL.
	\param mime \c true, if the message should be sent to the MIME data base
		   service, \c false for the roster.
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: \c NULL \a message.
	- \c B_NO_INIT: the roster is \c NULL.
	- another error code
*/
status_t BRoster::Private::SendTo(BMessage *message, BMessage *reply, bool mime)
{
	if (message == NULL)
		return B_BAD_VALUE;
	if (fRoster == NULL)
		return B_NO_INIT;

	const BMessenger& messenger = mime
		? fRoster->_MimeMessenger() : fRoster->fMessenger;
	if (messenger.IsTargetLocal())
		return B_BAD_VALUE;

	return reply != NULL
		? messenger.SendMessage(message, reply)
		: messenger.SendMessage(message);
}


/*!	\brief Returns whether the roster's messengers are valid.

	\a mime specifies whether to check the roster messenger or the one of
	the MIME data base service.

	\param mime \c true, if the MIME data base service messenger should be
		   checked, \c false for the roster messenger.
	\return \true, if the selected messenger is valid, \c false otherwise.
*/
bool BRoster::Private::IsMessengerValid(bool mime) const
{
	return fRoster != NULL && (mime ? fRoster->_MimeMessenger().IsValid()
		: fRoster->fMessenger.IsValid());
}


/*!	\brief Initializes the global  __be_roster variable.

	Called before the global constructors are invoked.
*/
void BRoster::Private::InitBeRoster()
{
	__be_roster = new BRoster;
}


/*!	\brief Deletes the global  __be_roster.

	Called after the global destructors are invoked.
*/
void BRoster::Private::DeleteBeRoster()
{
	delete  __be_roster;
}
