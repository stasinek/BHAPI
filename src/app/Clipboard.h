/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
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
 * File: Clipboard.h
 * Description: an interface to a clipboard
 * Warning: unfinished, just for system clipboard
 * 
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_CLIPBOARD__H
#define BHAPI_CLIPBOARD__H
#include "./../support/SupportDefs.h"
#include "./../app/Messenger.h"
#include "./../support/Locker.h"

#ifdef __cplusplus /* Just for C++ */

class IMPEXP_BHAPI BClipboard {
public:
	BClipboard(const char *name);
	virtual ~BClipboard();

	const char	*Name() const;

	b_uint32		LocalCount() const;
	b_uint32		SystemCount() const;
	b_status_t	StartWatching(const BMessenger &target);
	b_status_t	StopWatching(const BMessenger &target);

	bool		Lock();
	void		Unlock();
	b_int64		CountLocks() const;

	b_status_t	Clear();
	b_status_t	Commit();
	b_status_t	Revert();

	BMessenger	DataSource() const;
	BMessage	*Data() const;

private:
	char *fName;
	BLocker fLocker;
	BMessage *fData;
};

#endif /* __cplusplus */

#endif /* BHAPI_CLIPBOARD__H */

