/*
 * Copyright 2001-2011, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ingo Weinhold (bonefish@users.sf.net)
 */

//!	Global library initialization/termination routines.


#include <kits/netstdio.h>
#include <kits/netstdlib.h>

#include <kits/netAppMisc.h>
#include <kits/netLooperList.h>
#include <kits/netMessagePrivate.h>
#include <kits/netRosterPrivate.h>
#include <kits/netTokenSpace.h>


extern void __initialize_locale_kit();


// debugging
//#define DBG(x) x
#define DBG(x)
#define OUT	printf


static void initialize_forked_child()
{
	DBG(OUT("initialize_forked_child()\n"));

	BMessage::Private::StaticReInitForkedChild();
	BPrivate::gLooperList.InitAfterFork();
	BPrivate::gDefaultTokens.InitAfterFork();
	BPrivate::init_team_after_fork();

	DBG(OUT("initialize_forked_child() done\n"));
}


extern "C" void initialize_before()
{
	DBG(OUT("initialize_before()\n"));

	BMessage::Private::StaticInit();
	BRoster::Private::InitBeRoster();

	atfork(initialize_forked_child);

	DBG(OUT("initialize_before() done\n"));
}


extern "C" void initialize_after()
{
	DBG(OUT("initialize_after()\n"));

	__initialize_locale_kit();

	DBG(OUT("initialize_after() done\n"));
}


extern "C" void terminate_after()
{
	DBG(OUT("terminate_after()\n"));

	BRoster::Private::DeleteBeRoster();
	BMessage::Private::StaticCleanup();
	BMessage::Private::StaticCacheCleanup();

	DBG(OUT("terminate_after() done\n"));
}

