/*
 * Copyright 2001-2011, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Erik Jaesler (erik@cgsoftware.com)
 *		Axel Dörfler, axeld@pinc-software.de
 */
#ifndef _TOKEN_SPACE_H
#define _TOKEN_SPACE_H


#include <kits/netmap>
#include <kits/netstack>

#include <kits/netLocker.h>
#include <Haiku.h>


// token types as specified in targets
#define B_PREFERRED_TOKEN	-2		/* A little bird told me about this one */
#define B_NULL_TOKEN		-1
#define B_ANY_TOKEN			0

// token types in the token list
#define B_HANDLER_TOKEN		1
#define B_SERVER_TOKEN		2


namespace BPrivate {


class BDirectMessageTarget;


class BTokenSpace : public BLocker {
public:
								BTokenSpace();
								~BTokenSpace();

			int32_t				NewToken(int16 type, void* object);
			bool				SetToken(int32_t token, int16 type, void* object);

			bool				RemoveToken(int32_t token);
			bool				CheckToken(int32_t token, int16 type) const;
			status_t			GetToken(int32_t token, int16 type,
									void** _object) const;

			status_t			SetHandlerTarget(int32_t token,
									BDirectMessageTarget* target);
			status_t			AcquireHandlerTarget(int32_t token,
									BDirectMessageTarget** _target);

			void				InitAfterFork();

private:
	struct token_info {
		int16	type;
		void*	object;
		BDirectMessageTarget* target;
	};
	typedef std::map<int32_t, token_info> TokenMap;

			TokenMap			fTokenMap;
			int32_t				fTokenCount;
};


extern BTokenSpace gDefaultTokens;


}	// namespace BPrivate


#endif	// _TOKEN_SPACE_H
