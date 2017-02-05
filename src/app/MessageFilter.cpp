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
 * File: MessageFilter.cpp
 * Description: Filter message before ELooper::DispatchMessage
 *
 * --------------------------------------------------------------------------*/

#include "Looper.h"
#include "MessageFilter.h"


EMessageFilter::EMessageFilter(e_message_delivery delivery, e_message_source source, euint32 command, e_filter_hook filter)
	: fFiltersAny(false), fHandler(NULL)
{
	fDelivery = delivery;
	fSource = source;
	fCommand = command;
	fFilterHook = filter;
}


EMessageFilter::EMessageFilter(e_message_delivery delivery, e_message_source source, e_filter_hook filter)
	: fCommand(0), fFiltersAny(true), fHandler(NULL)
{
	fDelivery = delivery;
	fSource = source;
	fFilterHook = filter;
}


EMessageFilter::EMessageFilter(euint32 command, e_filter_hook filter)
	: fFiltersAny(false), fDelivery(E_ANY_DELIVERY), fSource(E_ANY_SOURCE), fHandler(NULL)
{
	fCommand = command;
	fFilterHook = filter;
}


EMessageFilter::EMessageFilter(const EMessageFilter &filter)
	: fHandler(NULL)
{
	fCommand = filter.fCommand;
	fFiltersAny = filter.fFiltersAny;
	fDelivery = filter.fDelivery;
	fSource = filter.fSource;
	fFilterHook = filter.fFilterHook;
}


EMessageFilter::EMessageFilter(const EMessageFilter *filter)
	: fHandler(NULL)
{
	fCommand = (filter ? 0 : filter->fCommand);
	fFiltersAny = (filter ? true : filter->fFiltersAny);
	fDelivery = (filter ? E_ANY_DELIVERY : filter->fDelivery);
	fSource = (filter ? E_ANY_SOURCE : filter->fSource);
	fFilterHook = (filter ? NULL : filter->fFilterHook);
}


EMessageFilter::~EMessageFilter()
{
	// TODO

	if(fHandler != NULL)
	{
		if(fHandler->EHandler::RemoveFilter(this) == false &&
		   fHandler->Looper() != NULL) fHandler->Looper()->ELooper::RemoveCommonFilter(this);
	}
}


EMessageFilter&
EMessageFilter::operator=(const EMessageFilter &filter)
{
	// TODO

	fCommand = filter.fCommand;
	fFiltersAny = filter.fFiltersAny;
	fDelivery = filter.fDelivery;
	fSource = filter.fSource;
	fFilterHook = filter.fFilterHook;

	return *this;
}


e_filter_result
EMessageFilter::Filter(EMessage *message, EHandler **target)
{
	return E_DISPATCH_MESSAGE;
}


e_filter_result
EMessageFilter::doFilter(EMessage *message, EHandler **target)
{
	if(message == NULL || target == NULL || fHandler == NULL) return E_SKIP_MESSAGE;

	e_filter_result retVal = E_DISPATCH_MESSAGE;

	// TODO: delivery & source
	if(fFiltersAny || message->what == fCommand)
	{
		if(fFilterHook != NULL) retVal = (*fFilterHook)(message, target, this);
		if(retVal == E_DISPATCH_MESSAGE) retVal = Filter(message, target);
	}

	return retVal;
}


e_message_delivery
EMessageFilter::MessageDelivery() const
{
	return fDelivery;
}


e_message_source
EMessageFilter::MessageSource() const
{
	return fSource;
}


euint32
EMessageFilter::Command() const
{
	return fCommand;
}


bool
EMessageFilter::FiltersAnyCommand() const
{
	return fFiltersAny;
}


ELooper*
EMessageFilter::Looper() const
{
	if(fHandler == NULL) return NULL;
	return fHandler->Looper();
}

