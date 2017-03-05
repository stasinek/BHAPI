﻿/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
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
 * File: Volume.cpp
 *
 * --------------------------------------------------------------------------*/

#include "Volume.h"


#include "../support/UTF8.h"
#include "../support/Errors.h"

#ifdef __BEOS__
#include <be/storage/Entry.h>
#include <be/storage/Directory.h>
#include <be/storage/Path.h>
#include <be/storage/VolumeRoster.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#undef HAVE_MNTENT_H
#endif // _WIN32

#ifdef HAVE_MNTENT_H
#include <stdio.h>
#include <mntent.h>
#endif
namespace bhapi {
typedef struct dev_data_t {
    char *name;
    char *root_dir;
} dev_data_t;
}
namespace bhapi {
inline bhapi::dev_data_t* new_dev_data()
{
    bhapi::dev_data_t* data = (bhapi::dev_data_t*)malloc(sizeof(bhapi::dev_data_t));
    if(data == NULL) return NULL;
    data->name = NULL;
    data->root_dir = NULL;
    return data;
}


inline b_status_t set_dev_data(bhapi::dev_data_t *data, const char *name, const char *root_dir)
{
    if(data == NULL) return B_BAD_VALUE;
    if(root_dir == NULL || *root_dir == 0) return B_BAD_VALUE;

#ifdef BHAPI_OS_UNIX
    if(*root_dir != '/') return B_BAD_VALUE;
#ifdef BHAPI_OS_LINUX
    if(strlen(root_dir) == 5 && strcmp(root_dir, "/proc") == 0) return B_BAD_VALUE;
    if(strlen(root_dir) == 4 && strcmp(root_dir, "/dev") == 0) return B_BAD_VALUE;
#endif // BHAPI_OS_LINUX
#else
#ifdef _WIN32
    if(strlen(root_dir) < 3 ||
       !((*root_dir >= 'a' && *root_dir <= 'z') || (*root_dir >= 'A' && *root_dir <= 'Z')) ||
       root_dir[1] != ':' || root_dir[2] != '/') return B_BAD_VALUE;
#endif // _WIN32
#endif // BHAPI_OS_UNIX

    char *fname = (name == NULL ? NULL : bhapi::strdup(name));
    char *fdir = bhapi::strdup(root_dir);

    if((fname == NULL && !(name == NULL || *name == 0)) || fdir == NULL)
    {
        if(fname) free(fname);
        if(fdir) free(fdir);
        return B_NO_MEMORY;
    }

    if(data->name) free(data->name);
    if(data->root_dir) free(data->root_dir);

    data->name = fname;
    data->root_dir = fdir;

    return B_OK;
}

inline void delete_dev_data(bhapi::dev_data_t *data)
{
    if(data == NULL) return;
    if(data->name) free(data->name);
    if(data->root_dir) free(data->root_dir);
    free(data);
}
} /* namespace */

BVolume::BVolume()
    : fDevice(0), fData(NULL)
{
}


BVolume::BVolume(b_dev_t dev)
    : fDevice(0), fData(NULL)
{
    SetTo(dev);
}


BVolume::BVolume(const BVolume &from)
    : fDevice(0), fData(NULL)
{
    SetTo(from.fDevice);
}


BVolume::~BVolume()
{
    Unset();
}


b_status_t
BVolume::InitCheck() const
{
    return(fDevice == 0 || fData == NULL ? B_NO_INIT : B_OK);
}


b_status_t
BVolume::SetTo(b_dev_t dev)
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
            BHAPI_DEBUG("[STORAGE]: %s --- Unable to open /etc/fstab", __PRETTY_FUNCTION__);
            return B_ENTRY_NOT_FOUND;
        }

        struct mntent *mnt = NULL;
        for(b_dev_t i = 0; i < dev; i++) {if((mnt = getmntent(ent)) == NULL) break;}

        if(mnt == NULL)
        {
            endmntent(ent);
            return B_ENTRY_NOT_FOUND;
        }

        if(fData == NULL)
        {
            if((fData = b_new_dev_data()) == NULL)
            {
                endmntent(ent);
                return B_NO_MEMORY;
            }
        }

        b_status_t status = b_set_dev_data((bhapi::dev_data_t*)fData, mnt->mnt_fsname, mnt->mnt_dir);
        endmntent(ent);

        if(status != B_OK) return status;

        fDevice = dev;
    }

    return B_OK;
#else // !HAVE_MNTENT_H
#ifdef _WIN32
    if(dev <= 0)
    {
        Unset();
    }
    else if(fDevice != dev)
    {
        if(dev > 26) return B_ENTRY_NOT_FOUND;

        DWORD driveMask = GetLogicalDrives();
        if(driveMask == 0) return B_ENTRY_NOT_FOUND;
        if(!(driveMask & (1UL << (dev - 1)))) return B_BAD_VALUE;

        if(fData == NULL)
        {
            if((fData = bhapi::new_dev_data()) == NULL) return B_NO_MEMORY;
        }

        char dirname[4] = "A:\\";
        *dirname += (dev - 1);

        BString nameStr;
        char nameBuf[301];
        bzero(nameBuf, 301);
        if(!(GetVolumeInformationA(dirname, nameBuf, 300, NULL, NULL, NULL, NULL, 0) == 0 || nameBuf[0] == 0))
        {
            WCHAR wStr[301];
            bzero(wStr, sizeof(WCHAR) * 301);
            MultiByteToWideChar(CP_ACP, 0, nameBuf, -1, wStr, 300);
            char *utf8Name = bhapi::unicode_convert_to_utf8((const b_unichar16*)wStr, -1);
            if(utf8Name != NULL)
            {
                nameStr.SetTo(utf8Name);
                free(utf8Name);
            }
        }
        if(nameStr.Length() <= 0) nameStr.SetTo(nameBuf);
        dirname[2] = '/';

        b_status_t status = bhapi::set_dev_data((bhapi::dev_data_t*)fData, nameStr.String(), dirname);

        if(status != B_OK) return status;

        fDevice = dev;
    }

    return B_OK;
#else // !_WIN32
#ifdef __BEOS__
    if(dev <= 0)
    {
        Unset();
    }
    else if(fDevice != dev)
    {
        if(fData == NULL)
            if((fData = b_new_dev_data()) == NULL) return B_NO_MEMORY;

        BVolume vol;
        BVolumeRoster volRoster;
        BDirectory beDir;
        BEntry beEntry;
        BPath bePath;
        char volName[B_FILE_NAME_LENGTH + 1];
        bzero(volName, B_FILE_NAME_LENGTH + 1);

        b_dev_t tmp = dev;
        while(tmp > 0)
        {
            if(volRoster.GetNextVolume(&vol) != B_OK) return B_ENTRY_NOT_FOUND;
            if(--tmp > 0) continue;
            if(vol.GetRootDirectory(&beDir) != B_OK ||
               beDir.GetEntry(&beEntry) != B_OK ||
               beEntry.GetPath(&bePath) != B_OK) return B_ENTRY_NOT_FOUND;
            vol.GetName(volName);
        }

        b_status_t status = b_set_dev_data((bhapi::dev_data_t*)fData, volName, bePath.Path());
        if(status != B_OK) return status;

        fDevice = dev;
    }

    return B_OK;
#else // !__BEOS__
    #warning "fixme: BVolume::SetTo"
    if(dev <= 0)
    {
        Unset();
        return B_OK;
    }
    else if(fDevice != dev && dev == 1)
    {
        if(fData == NULL)
            if((fData = b_new_dev_data()) == NULL) return B_NO_MEMORY;

        b_status_t status = b_set_dev_data((bhapi::dev_data_t*)fData, "root", "/");
        if(status != B_OK) return status;

        fDevice = dev;
        return B_OK;
    }

    return B_ENTRY_NOT_FOUND;
#endif // __BEOS__
#endif // _WIN32
#endif // HAVE_MNTENT_H
}


void
BVolume::Unset()
{
    if(fData != NULL) bhapi::delete_dev_data((bhapi::dev_data_t*)fData);

    fData = NULL;
    fDevice = 0;
}


b_dev_t
BVolume::Device() const
{
    return fDevice;
}


b_status_t
BVolume::GetName(BString *name) const
{
    if(name == NULL) return B_BAD_VALUE;
    if(fData == NULL) return B_NO_INIT;

    *name = ((bhapi::dev_data_t*)fData)->name;
    return B_OK;
}


b_status_t BVolume::GetName(char *name, size_t nameSize) const
{
    BString str;

    b_status_t status = GetName(&str);
    if(status == B_OK) str.CopyInto(name, nameSize, 0, -1);

    return status;
}


b_status_t
BVolume::SetName(const char *name)
{
    // TODO
    return B_ERROR;
}


b_status_t
BVolume::GetRootDirectory(BDirectory *dir) const
{
    if(dir == NULL) return B_BAD_VALUE;

    dir->Unset();
    if(fData == NULL) return B_NO_INIT;
    return dir->SetTo(((bhapi::dev_data_t*)fData)->root_dir);
}


bool
BVolume::operator==(const BVolume &vol) const
{
    return(fDevice == vol.fDevice);
}


bool
BVolume::operator!=(const BVolume &vol) const
{
    return(fDevice != vol.fDevice);
}


BVolume&
BVolume::operator=(const BVolume &vol)
{
    Unset();
    SetTo(vol.fDevice);

    return *this;
}

