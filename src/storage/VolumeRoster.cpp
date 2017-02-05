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
 * File: VolumeRoster.cpp
 *
 * --------------------------------------------------------------------------*/

#include "VolumeRoster.h"


EVolumeRoster::EVolumeRoster()
	: fPos(0)
{
}


EVolumeRoster::~EVolumeRoster()
{
}


e_status_t
EVolumeRoster::GetNextVolume(EVolume *vol)
{
	if(vol == NULL) return E_BAD_VALUE;

	EVolume aVol;
	while(true)
	{
		fPos++;

		e_status_t status = aVol.SetTo((e_dev_t)fPos);
		if(status == E_ENTRY_NOT_FOUND) return E_BAD_VALUE;
		if(status == E_BAD_VALUE) continue;
		if(status != E_OK) return status;

		status = vol->SetTo(aVol.Device());
		return status;
	}

	return E_NO_ERROR;
}


void
EVolumeRoster::Rewind()
{
	fPos = 0;
}


e_status_t
EVolumeRoster::GetBootVolume(EVolume *vol)
{
	if(vol == NULL) return E_BAD_VALUE;

#ifdef _WIN32
	if(vol->SetTo(3) == E_OK) return E_NO_ERROR;
	return E_ENTRY_NOT_FOUND;
#else
	e_dev_t dev = 0;
	EVolume aVol;
	while(true)
	{
		dev++;

		e_status_t status = aVol.SetTo(dev);
		if(status == E_ENTRY_NOT_FOUND) return E_BAD_VALUE;
		if(status == E_BAD_VALUE) continue;
		if(status != E_OK) return status;

		EDirectory dir;
		if(aVol.GetRootDirectory(&dir) != E_OK) continue;

		EEntry entry;
		if(dir.GetEntry(&entry) != E_OK) continue;

		EPath path;
		if(entry.GetPath(&path) != E_OK) continue;

#ifndef __BEOS__
		if(path != "/") continue;
#else
		if(path != "/boot") continue;
#endif

		if(vol->SetTo(dev) == E_OK) return E_NO_ERROR;
		break;
	}

	return E_ENTRY_NOT_FOUND;
#endif
}

