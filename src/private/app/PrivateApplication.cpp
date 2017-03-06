/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
 *
 * BHAPI++ library is a freeware; it may be used and distributed according to
 * the terms of The MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File: PrivateApplication.cpp
 *
 * --------------------------------------------------------------------------*/

#include "PrivateApplication.h"

#include "../kernel/OS.h"
#include "../support/StringClass.h"
#include "../kernel/Debug.h"
#include "../support/Errors.h"

BApplicationConnector *bhapi::app_connector = NULL;

BApplicationConnector::BApplicationConnector()
	: fLocker(true), fPort(NULL), fThread(NULL)
{
#if 0
    if(bhapi::get_current_team_id() == 0)
		BHAPI_ERROR("[PRIVATE]: %s --- Unsupported system.", __PRETTY_FUNCTION__);

	BString port_name;
    port_name << "e_app_" << bhapi::get_current_team_id();

    if((fPort = bhapi::create_port(10, port_name.String())) == NULL)
		BHAPI_ERROR("[PRIVATE]: %s --- Unable to create port.", __PRETTY_FUNCTION__);

    if((fThread = bhapi::create_thread(this->task, B_NORMAL_PRIORITY, reinterpret_cast<void*>(this), NULL)) == NULL)
		BHAPI_ERROR("[PRIVATE]: %s --- Unable to create thread.", __PRETTY_FUNCTION__);
#endif

	fHandlersDepot = new BTokensDepot(new BLocker(), true);

#if 0
    if(bhapi::resume_thread(fThread) != B_OK)
		BHAPI_ERROR("[PRIVATE]: %s --- Unable to resume thread.", __PRETTY_FUNCTION__);
#endif
}


BApplicationConnector::~BApplicationConnector()
{
    bhapi::close_port(fPort);

#if 0
	// FIXME: objects deleted when thread quiting, then it blocks !!!
	b_status_t err;
    bhapi::wait_for_thread(fThread, &err);
    bhapi::delete_thread(fThread);
#endif

    bhapi::delete_port(fPort);

	delete fHandlersDepot;
}


bool
BApplicationConnector::Lock()
{
	return fLocker.Lock();
}


void
BApplicationConnector::Unlock()
{
	fLocker.Unlock();
}


b_status_t
BApplicationConnector::task(void *data)
{
	BApplicationConnector *self = reinterpret_cast<BApplicationConnector*>(data);

	b_uint8 *buffer = (b_uint8*)malloc(BHAPI_MAX_PORT_BUFFER_SIZE + 1);
	if(buffer == NULL)
		BHAPI_ERROR("[PRIVATE]: %s --- Unable to allocate memory.", __PRETTY_FUNCTION__);

	b_int32 code;
	b_status_t err;

	for(code = 0;
        (err = bhapi::read_port_etc(self->fPort, &code, buffer, BHAPI_MAX_PORT_BUFFER_SIZE, B_TIMEOUT, 1000000)) != B_ERROR;
	    code = 0)
	{
		BHAPI_DEBUG("[PRIVATE]: %s --- Hey(%I64i:%I64i), running(%I32i) ...",
              __PRETTY_FUNCTION__, bhapi::get_current_team_id(), bhapi::get_current_thread_id(), err - B_GENERAL_ERROR_BASE);

		/* do something */
	}

	free(buffer);

	BHAPI_DEBUG("[PRIVATE]: %s --- Hey(%I64i:%I64i), quited.",
          __PRETTY_FUNCTION__, bhapi::get_current_team_id(), bhapi::get_current_thread_id());

	return B_OK;
}


BTokensDepot*
BApplicationConnector::HandlersDepot() const
{
	return fHandlersDepot;
}


void
BApplicationConnector::Init()
{
    bhapi::app_connector = new BApplicationConnector();
}


void
BApplicationConnector::Quit()
{
    delete bhapi::app_connector;
}


#ifndef _WIN32
class LOCALBHAPI BApplicationInitializer {
public:
	BApplicationInitializer()
	{
		BApplicationConnector::Init();
	}

	~BApplicationInitializer()
	{
		BApplicationConnector::Quit();
	}
};

namespace bhapi {
static BApplicationInitializer app_initializer;
}
#endif

