/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
 *
 * ETK++ library is a freeware; it may be used and distributed according to
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

#include "./../kernel/OS.h>
#include "./../support/String.h>

#include "PrivateApplication.h"


EApplicationConnector *etk_app_connector = NULL;


EApplicationConnector::EApplicationConnector()
	: fLocker(true), fPort(NULL), fThread(NULL)
{
#if 0
	if(etk_get_current_team_id() == 0)
		ETK_ERROR("[PRIVATE]: %s --- Unsupported system.", __PRETTY_FUNCTION__);

	EString port_name;
	port_name << "e_app_" << etk_get_current_team_id();

	if((fPort = etk_create_port(10, port_name.String())) == NULL)
		ETK_ERROR("[PRIVATE]: %s --- Unable to create port.", __PRETTY_FUNCTION__);

	if((fThread = etk_create_thread(this->task, E_NORMAL_PRIORITY, reinterpret_cast<void*>(this), NULL)) == NULL)
		ETK_ERROR("[PRIVATE]: %s --- Unable to create thread.", __PRETTY_FUNCTION__);
#endif

	fHandlersDepot = new ETokensDepot(new ELocker(), true);

#if 0
	if(etk_resume_thread(fThread) != E_OK)
		ETK_ERROR("[PRIVATE]: %s --- Unable to resume thread.", __PRETTY_FUNCTION__);
#endif
}


EApplicationConnector::~EApplicationConnector()
{
	etk_close_port(fPort);

#if 0
	// FIXME: objects deleted when thread quiting, then it blocks !!!
	e_status_t err;
	etk_wait_for_thread(fThread, &err);
	etk_delete_thread(fThread);
#endif

	etk_delete_port(fPort);

	delete fHandlersDepot;
}


bool
EApplicationConnector::Lock()
{
	return fLocker.Lock();
}


void
EApplicationConnector::Unlock()
{
	fLocker.Unlock();
}


e_status_t
EApplicationConnector::task(void *data)
{
	EApplicationConnector *self = reinterpret_cast<EApplicationConnector*>(data);

	euint8 *buffer = (euint8*)malloc(ETK_MAX_PORT_BUFFER_SIZE + 1);
	if(buffer == NULL)
		ETK_ERROR("[PRIVATE]: %s --- Unable to allocate memory.", __PRETTY_FUNCTION__);

	eint32 code;
	e_status_t err;

	for(code = 0;
	    (err = etk_read_port_etc(self->fPort, &code, buffer, ETK_MAX_PORT_BUFFER_SIZE, E_TIMEOUT, 1000000)) != E_ERROR;
	    code = 0)
	{
		ETK_DEBUG("[PRIVATE]: %s --- Hey(%I64i:%I64i), running(%I32i) ...",
			  __PRETTY_FUNCTION__, etk_get_current_team_id(), etk_get_current_thread_id(), err - E_GENERAL_ERROR_BASE);

		/* do something */
	}

	free(buffer);

	ETK_DEBUG("[PRIVATE]: %s --- Hey(%I64i:%I64i), quited.",
		  __PRETTY_FUNCTION__, etk_get_current_team_id(), etk_get_current_thread_id());

	return E_OK;
}


ETokensDepot*
EApplicationConnector::HandlersDepot() const
{
	return fHandlersDepot;
}


void
EApplicationConnector::Init()
{
	etk_app_connector = new EApplicationConnector();
}


void
EApplicationConnector::Quit()
{
	delete etk_app_connector;
}


#ifndef _WIN32
class _LOCAL EApplicationInitializer {
public:
	EApplicationInitializer()
	{
		EApplicationConnector::Init();
	}

	~EApplicationInitializer()
	{
		EApplicationConnector::Quit();
	}
};

static EApplicationInitializer _etk_app_initializer;
#endif

