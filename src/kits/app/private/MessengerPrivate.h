/*
 * Copyright 2003-2005, Haiku.
 * Distributed under the terms of the MIT License.
 */
#ifndef MESSENGER_PRIVATE_H
#define MESSENGER_PRIVATE_H


#include <kits/app/Messenger.h>
#include <kits/netTokenSpace.h>


class BMessenger::Private {
	public:
		Private(BMessenger* messenger) : fMessenger(messenger) {}
		Private(BMessenger& messenger) : fMessenger(&messenger) {}

		port_id	Port()
			{ return fMessenger->fPort; }
		int32_t Token()
			{ return fMessenger->fHandlerToken; }
		team_id	Team()
			{ return fMessenger->fTeam; }
		bool IsPreferredTarget()
			{ return fMessenger->fHandlerToken == B_PREFERRED_TOKEN; }

		void SetTo(team_id team, port_id port, int32_t token)
			{ fMessenger->_SetTo(team, port, token); }

	private:
		BMessenger* fMessenger;
};

#endif	// MESSENGER_PRIVATE_H
