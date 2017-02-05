/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
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
 * File: Volume.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_VOLUME_H__
#define __ETK_VOLUME_H__

#include "./../storage/StorageDefs.h>
#include "./../storage/Directory.h>
#include "./../support/String.h>

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK EVolume {
public:
	EVolume();
	EVolume(e_dev_t dev);
	EVolume(const EVolume &from);
	virtual ~EVolume();

	e_status_t	InitCheck() const;
	e_status_t	SetTo(e_dev_t dev);
	void		Unset();

	e_dev_t		Device() const;

	e_status_t	GetName(char *name, size_t nameSize) const;
	e_status_t	GetName(EString *name) const;
	e_status_t	SetName(const char *name);

	e_status_t	GetRootDirectory(EDirectory *dir) const;

	bool		operator==(const EVolume &vol) const;
	bool		operator!=(const EVolume &vol) const;
	EVolume&	operator=(const EVolume &vol);

private:
	e_dev_t fDevice;
	void *fData;
};

#endif /* __cplusplus */

#endif /* __ETK_VOLUME_H__ */

