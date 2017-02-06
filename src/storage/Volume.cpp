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
 * File: Volume.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../support/supportDefs.h"

#ifdef _WIN32
#include <windows.h>
#undef HAVE_MNTENT_H
#endif // _WIN32

#ifdef HAVE_MNTENT_H
#include <stdio.h>
#include <mntent.h>
#endif

#ifdef __BEOS__
#include <be/storage/Entry.h>
#include <be/storage/Directory.h>
#include <be/storage/Path.h>
#include <be/storage/VolumeRoster.h>
#endif

#include "Volume.h"


typedef struct e_dev_data_t {
	char *name;
	char *root_dir;
} e_dev_data_t;


inline e_dev_data_t* etk_new_dev_data()
{
	e_dev_data_t* data = (e_dev_data_t*)malloc(sizeof(e_dev_data_t));
	if(data == NULL) return NULL;
	data->name = NULL;
	data->root_dir = NULL;
	return data;
}


inline e_status_t etk_set_dev_data(e_dev_data_t *data, const char *name, const char *root_dir)
{
	if(data == NULL) return E_BAD_VALUE;
	if(root_dir == NULL || *root_dir == 0) return E_BAD_VALUE;

#ifdef ETK_OS_UNIX
	if(*root_dir != '/') return E_BAD_VALUE;
#ifdef ETK_OS_LINUX
	if(strlen(root_dir) == 5 && strcmp(root_dir, "/proc") == 0) return E_BAD_VALUE;
	if(strlen(root_dir) == 4 && strcmp(root_dir, "/dev") == 0) return E_BAD_VALUE;
#endif // ETK_OS_LINUX
#else
#ifdef _WIN32
	if(strlen(root_dir) < 3 ||
	   !((*root_dir >= 'a' && *root_dir <= 'z') || (*root_dir >= 'A' && *root_dir <= 'Z')) ||
	   root_dir[1] != ':' || root_dir[2] != '/') return E_BAD_VALUE;
#endif // _WIN32
#endif // ETK_OS_UNIX

	char *fname = (name == NULL ? NULL : e_strdup(name));
	char *fdir = e_strdup(root_dir);

	if((fname == NULL && !(name == NULL || *name == 0)) || fdir == NULL)
	{
		if(fname) free(fname);
		if(fdir) free(fdir);
		return E_NO_MEMORY;
	}

	if(data->name) free(data->name);
	if(data->root_dir) free(data->root_dir);

	data->name = fname;
	data->root_dir = fdir;

	return E_OK;
}


inline void etk_delete_dev_data(e_dev_data_t *data)
{
	if(data == NULL) return;
	if(data->name) free(data->name);
	if(data->root_dir) free(data->root_dir);
	free(data);
}


EVolume::EVolume()
	: fDevice(0), fData(NULL)
{
}


EVolume::EVolume(e_dev_t dev)
	: fDevice(0), fData(NULL)
{
	SetTo(dev);
}


EVolume::EVolume(const EVolume &from)
	: fDevice(0), fData(NULL)
{
	SetTo(from.fDevice);
}


EVolume::~EVolume()
{
	Unset();
}


e_status_t
EVolume::InitCheck() const
{
	return(fDevice == 0 || fData == NULL ? E_NO_INIT : E_OK);
}


e_status_t
EVolume::SetTo(e_dev_t dev)
{
#ifdef HAVE_MNTENT_H
	if(dev <= 0)
	{
		Unset();
	}
	else if(fDevice != dev)
	{
		FILE *ent = setmntent("/etc/fstab", "r");
		if(ent == NULL)
		{
			ETK_DEBUG("[STORAGE]: %s --- Unable to open /etc/fstab", __PRETTY_FUNCTION__);
			return E_ENTRY_NOT_FOUND;
		}

		struct mntent *mnt = NULL;
		for(e_dev_t i = 0; i < dev; i++) {if((mnt = getmntent(ent)) == NULL) break;}

		if(mnt == NULL)
		{
			endmntent(ent);
			return E_ENTRY_NOT_FOUND;
		}

		if(fData == NULL)
		{
			if((fData = etk_new_dev_data()) == NULL)
			{
				endmntent(ent);
				return E_NO_MEMORY;
			}
		}

		e_status_t status = etk_set_dev_data((e_dev_data_t*)fData, mnt->mnt_fsname, mnt->mnt_dir);
		endmntent(ent);

		if(status != E_OK) return status;

		fDevice = dev;
	}

	return E_OK;
#else // !HAVE_MNTENT_H
#ifdef _WIN32
	if(dev <= 0)
	{
		Unset();
	}
	else if(fDevice != dev)
	{
		if(dev > 26) return E_ENTRY_NOT_FOUND;

		DWORD driveMask = GetLogicalDrives();
		if(driveMask == 0) return E_ENTRY_NOT_FOUND;
		if(!(driveMask & (1UL << (dev - 1)))) return E_BAD_VALUE;

		if(fData == NULL)
		{
			if((fData = etk_new_dev_data()) == NULL) return E_NO_MEMORY;
		}

		char dirname[4] = "A:\\";
		*dirname += (dev - 1);

		EString nameStr;
		char nameBuf[301];
		bzero(nameBuf, 301);
        if(!(GetVolumeInformationA(dirname, nameBuf, 300, NULL, NULL, NULL, NULL, 0) == 0 || nameBuf[0] == 0))
		{
			WCHAR wStr[301];
			bzero(wStr, sizeof(WCHAR) * 301);
			MultiByteToWideChar(CP_ACP, 0, nameBuf, -1, wStr, 300);
			char *utf8Name = e_unicode_convert_to_utf8((const eunichar*)wStr, -1);
			if(utf8Name != NULL)
			{
				nameStr.SetTo(utf8Name);
				free(utf8Name);
			}
		}
		if(nameStr.Length() <= 0) nameStr.SetTo(nameBuf);
		dirname[2] = '/';

		e_status_t status = etk_set_dev_data((e_dev_data_t*)fData, nameStr.String(), dirname);

		if(status != E_OK) return status;

		fDevice = dev;
	}

	return E_OK;
#else // !_WIN32
#ifdef __BEOS__
	if(dev <= 0)
	{
		Unset();
	}
	else if(fDevice != dev)
	{
		if(fData == NULL)
			if((fData = etk_new_dev_data()) == NULL) return E_NO_MEMORY;

		BVolume vol;
		BVolumeRoster volRoster;
		BDirectory beDir;
		BEntry beEntry;
		BPath bePath;
		char volName[B_FILE_NAME_LENGTH + 1];
		bzero(volName, B_FILE_NAME_LENGTH + 1);

		e_dev_t tmp = dev;
		while(tmp > 0)
		{
			if(volRoster.GetNextVolume(&vol) != B_OK) return E_ENTRY_NOT_FOUND;
			if(--tmp > 0) continue;
			if(vol.GetRootDirectory(&beDir) != B_OK ||
			   beDir.GetEntry(&beEntry) != B_OK ||
			   beEntry.GetPath(&bePath) != B_OK) return E_ENTRY_NOT_FOUND;
			vol.GetName(volName);
		}

		e_status_t status = etk_set_dev_data((e_dev_data_t*)fData, volName, bePath.Path());
		if(status != E_OK) return status;

		fDevice = dev;
	}

	return E_OK;
#else // !__BEOS__
	#warning "fixme: EVolume::SetTo"
	if(dev <= 0)
	{
		Unset();
		return E_OK;
	}
	else if(fDevice != dev && dev == 1)
	{
		if(fData == NULL)
			if((fData = etk_new_dev_data()) == NULL) return E_NO_MEMORY;

		e_status_t status = etk_set_dev_data((e_dev_data_t*)fData, "root", "/");
		if(status != E_OK) return status;

		fDevice = dev;
		return E_OK;
	}

	return E_ENTRY_NOT_FOUND;
#endif // __BEOS__
#endif // _WIN32
#endif // HAVE_MNTENT_H
}


void
EVolume::Unset()
{
	if(fData != NULL) etk_delete_dev_data((e_dev_data_t*)fData);

	fData = NULL;
	fDevice = 0;
}


e_dev_t
EVolume::Device() const
{
	return fDevice;
}


e_status_t
EVolume::GetName(EString *name) const
{
	if(name == NULL) return E_BAD_VALUE;
	if(fData == NULL) return E_NO_INIT;

	*name = ((e_dev_data_t*)fData)->name;
	return E_OK;
}


e_status_t EVolume::GetName(char *name, size_t nameSize) const
{
	EString str;

	e_status_t status = GetName(&str);
	if(status == E_OK) str.CopyInto(name, nameSize, 0, -1);

	return status;
}


e_status_t
EVolume::SetName(const char *name)
{
	// TODO
	return E_ERROR;
}


e_status_t
EVolume::GetRootDirectory(EDirectory *dir) const
{
	if(dir == NULL) return E_BAD_VALUE;

	dir->Unset();
	if(fData == NULL) return E_NO_INIT;
	return dir->SetTo(((e_dev_data_t*)fData)->root_dir);
}


bool
EVolume::operator==(const EVolume &vol) const
{
	return(fDevice == vol.fDevice);
}


bool
EVolume::operator!=(const EVolume &vol) const
{
	return(fDevice != vol.fDevice);
}


EVolume&
EVolume::operator=(const EVolume &vol)
{
	Unset();
	SetTo(vol.fDevice);

	return *this;
}

