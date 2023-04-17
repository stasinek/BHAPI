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
 * File: Entry.cpp
 *
 * --------------------------------------------------------------------------*/

#include <Entry.h>
#include <Directory.h>

#include <kits/support/String.h>
#include <kits/debug/Errors.h>
#include <os/debug.h>

#ifndef _WIN32
#define __USE_LARGEFILE64
#define __USE_FILE_OFFSET64
#include <unistd.h>
#else
#include <io.h>
#include <winsock2.h>
#include <windows.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

// implement in "Path.cpp"
extern status_t b_path_expound(BString &path, const char *dir, const char *leaf, bool *normalize);
extern status_t b_path_get_parent(BString &parent, const char *path);


BEntry::BEntry()
    : fName(NULL)
{
}


/*BEntry::BEntry(const char *dir, const char *leaf, bool traverse)
    : fName(NULL)
{
    SetTo(dir, leaf, traverse);
}

*/

BEntry::BEntry(const BDirectory *dir, const char *leaf, bool traverse)
    : fName(NULL)
{
    SetTo(dir, leaf, traverse);
}


BEntry::BEntry(const char *path, bool traverse)
    : fName(NULL)
{
    SetTo(path, traverse);
}


BEntry::BEntry(const BEntry &entry)
    : fName(NULL)
{
    BEntry::operator=(entry);
}


BEntry::~BEntry()
{
    if(fName != NULL) delete[] fName;
}


status_t
BEntry::SetTo(const char *path, bool traverse)
{
    return SetTo(path, NULL, traverse);
}


status_t
BEntry::SetTo(const char *dir, const char *leaf, bool traverse)
{
    if(dir == NULL) return B_BAD_VALUE;

    BString str;
    if(b_path_expound(str, dir, leaf, NULL) != B_OK) return B_BAD_VALUE;

    BString parent;
    status_t status = b_path_get_parent(parent, str.String());
    if(status == B_ENTRY_NOT_FOUND) parent = str;
    else if(status != B_OK) return B_BAD_VALUE;

#ifdef _WIN32
    parent.ReplaceAll("/", "\\");
#endif

    bool parentExists;
#ifndef _WIN32
    struct stat st;
    parentExists = (stat(parent.String(), &st) == 0);
#else
    struct _stat st;
    parentExists = (_stat(parent.String(), &st) == 0);
#endif
    if(!parentExists) return B_ENTRY_NOT_FOUND;

    // TODO: traverse

    char *name = bhapi::strdup(str.String());
    if(name == NULL) return B_NO_MEMORY;

    if(fName != NULL) delete[] fName;
    fName = name;

    return B_OK;
}


status_t
BEntry::SetTo(const BDirectory *dir, const char *leaf, bool traverse)
{
    if(dir == NULL || dir->InitCheck() != B_OK) return B_BAD_VALUE;
    return SetTo(dir->fName, leaf, traverse);
}


void
BEntry::Unset()
{
    if(fName != NULL) delete[] fName;
    fName = NULL;
}


status_t
BEntry::InitCheck() const
{
    if(fName == NULL) return B_NO_INIT;
    return B_OK;
}


bool
BEntry::Exists() const
{
    if(fName == NULL) return false;

    const char *filename = (const char*)fName;

#ifdef _WIN32
    BString str(fName);
    str.ReplaceAll("/", "\\");
    filename = str.String();
#endif

#ifndef _WIN32
    struct stat st;
    return(stat(filename, &st) == 0);
#else
    struct _stat st;
    return(_stat(filename, &st) == 0);
#endif
}


bool
BEntry::IsHidden() const
{
    bool retVal = false;

    BPath aPath(fName);
    const char *leaf = aPath.Leaf();
    if(!(leaf == NULL || *leaf != '.')) retVal = true;

#ifdef _WIN32
    BString str(fName);
    str.ReplaceAll("/", "\\");

    if(GetFileAttributesA(str.String()) & FILE_ATTRIBUTE_HIDDEN) retVal = true;
#endif

    return retVal;
}


bool
BEntry::IsFile() const
{
    if(fName == NULL) return false;

    const char *filename = (const char*)fName;

#ifdef _WIN32
    BString str(fName);
    str.ReplaceAll("/", "\\");
    filename = str.String();

    struct _stat st;
    if(_stat(filename, &st) != 0) return false;
    return((st.st_mode & _S_IFREG) ? true : false);
#else
    struct stat st;
    if(stat(filename, &st) != 0) return false;
    return S_ISREG(st.st_mode);
#endif
}


bool
BEntry::IsDirectory() const
{
    if(fName == NULL) return false;

    const char *filename = (const char*)fName;

#ifdef _WIN32
    BString str(fName);
    str.ReplaceAll("/", "\\");
    filename = str.String();

    struct _stat st;
    if(_stat(filename, &st) != 0) return false;
    return((st.st_mode & _S_IFDIR) ? true : false);
#else
    struct stat st;
    if(stat(filename, &st) != 0) return false;
    return S_ISDIR(st.st_mode);
#endif
}


bool
BEntry::IsSymLink() const
{
#ifdef S_ISLNK
    struct stat st;
    if(fName == NULL || lstat(fName, &st) != 0) return false;
    return S_ISLNK(st.st_mode);
#else
    return false;
#endif
}


status_t
BEntry::GetSize(int64 *file_size) const
{
    if(fName == NULL || file_size == NULL) return B_ERROR;

    const char *filename = (const char*)fName;

#ifdef _WIN32
    BString str(fName);
    str.ReplaceAll("/", "\\");
    filename = str.String();

    struct _stati64 stat;
    if(_stati64(filename, &stat) != 0) return B_ERROR;
    *file_size = (int64)stat.st_size;
#elif defined(HAVE_STAT64)
    struct stat64 stat;
    if(stat64(filename, &stat) != 0) return B_ERROR;
    *file_size = (int64)stat.st_size;
#else
    struct stat st;
    if(stat(filename, &st) != 0) return B_ERROR;
    *file_size = (int64)st.st_size;
#endif

    return B_OK;
}


status_t
BEntry::GetModificationTime(bigtime_t *time) const
{
    if(fName == NULL || time == NULL) return B_ERROR;

    const char *filename = (const char*)fName;

#ifdef _WIN32
    BString str(fName);
    str.ReplaceAll("/", "\\");
    filename = str.String();

    struct _stat stat;
    if(_stat(filename, &stat) != 0) return B_ERROR;
    *time = B_INT64_CONSTANT(1000000) * (bigtime_t)stat.st_mtime;
#else
    struct stat st;
    if(stat(filename, &st) != 0) return B_ERROR;
    *time = B_INT64_CONSTANT(1000000) * (bigtime_t)st.st_mtime;
#endif

    return B_OK;
}


status_t
BEntry::GetCreationTime(bigtime_t *time) const
{
    if(fName == NULL || time == NULL) return B_ERROR;

    const char *filename = (const char*)fName;

#ifdef _WIN32
    BString str(fName);
    str.ReplaceAll("/", "\\");
    filename = str.String();

    struct _stat stat;
    if(_stat(filename, &stat) != 0) return B_ERROR;
    *time = B_INT64_CONSTANT(1000000) * (bigtime_t)stat.st_ctime;
#else
    struct stat st;
    if(stat(filename, &st) != 0) return B_ERROR;
    *time = B_INT64_CONSTANT(1000000) * (bigtime_t)st.st_ctime;
#endif

    return B_OK;
}


status_t
BEntry::GetAccessTime(bigtime_t *time) const
{
    if(fName == NULL || time == NULL) return B_ERROR;

    const char *filename = (const char*)fName;

#ifdef _WIN32
    BString str(fName);
    str.ReplaceAll("/", "\\");
    filename = str.String();

    struct _stat stat;
    if(_stat(filename, &stat) != 0) return B_ERROR;
    *time = B_INT64_CONSTANT(1000000) * (bigtime_t)stat.st_atime;
#else
    struct stat st;
    if(stat(filename, &st) != 0) return B_ERROR;
    *time = B_INT64_CONSTANT(1000000) * (bigtime_t)st.st_atime;
#endif

    return B_OK;
}


const char*
BEntry::Name() const
{
    if(fName == NULL) return NULL;

    size_t nameLen = strlen(fName);

#ifdef _WIN32
    if(nameLen <= 3) return NULL;
#else
    if(nameLen == 1 && *fName == '/') return NULL;
#endif

    const char *tmp = fName + nameLen - 1;
    for(; nameLen > 0; nameLen--, tmp--)
    {
        if(*tmp == '/') return(tmp + 1);
    }

    return NULL;
}


status_t
BEntry::GetName(char *buffer, size_t bufferSize) const
{
    const char *name = Name();
    if(name == NULL) return B_ERROR;

    if(buffer == NULL && bufferSize == 0) return B_BAD_VALUE;
    if(bufferSize < strlen(name) + 1) return B_NAME_TOO_LONG;
    memcpy(buffer, name, strlen(name) + 1);
    return B_OK;
}


const char*
BEntry::Path() const
{
    return fName;
}


status_t
BEntry::GetPath(BPath *path) const
{
    if(path == NULL) return B_BAD_VALUE;
    if(fName == NULL) {path->Unset(); return B_NO_INIT;}

    return path->SetTo(fName, NULL, false);
}


status_t
BEntry::GetParent(BEntry *entry) const
{
    if(entry == NULL) return B_BAD_VALUE;
    if(fName == NULL) return B_NO_INIT;

    BString str;
    status_t status = b_path_get_parent(str, fName);
    if(status != B_OK) return status;

    return entry->SetTo(str.String(), false);
}


status_t
BEntry::GetParent(BPath *path) const
{
    if(path == NULL) return B_BAD_VALUE;
    if(fName == NULL) return B_NO_INIT;

    BString str;
    status_t status = b_path_get_parent(str, fName);
    if(status != B_OK) return status;

    return path->SetTo(str.String(), NULL, false);
}


status_t
BEntry::GetParent(BDirectory *dir) const
{
    if(dir == NULL) return B_BAD_VALUE;
    if(fName == NULL) return B_NO_INIT;

    BString str;
    status_t status = b_path_get_parent(str, fName);
    if(status != B_OK) return status;

    return dir->SetTo(str.String());
}


bool
BEntry::operator==(const BEntry &entry) const
{
    if(fName == NULL && entry.fName == NULL) return true;
    if(fName == NULL || entry.fName == NULL) return false;
    return(!(strlen(fName) != strlen(entry.fName) || strcmp(fName, entry.fName) != 0));
}


bool
BEntry::operator!=(const BEntry &entry) const
{
    if(fName == NULL && entry.fName == NULL) return false;
    if(fName == NULL || entry.fName == NULL) return true;
    if(strlen(fName) != strlen(entry.fName)) return true;
    return(strcmp(fName, entry.fName) != 0);
}


BEntry&
BEntry::operator=(const BEntry &entry)
{
    if(fName != NULL) delete[] fName;
    fName = bhapi::strdup(entry.fName);
    return *this;
}

/*
 * Copyright 2002-2012, Haiku Inc.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Tyler Dauwalder
 *		Ingo Weinhold, bonefish@users.sf.net
 */


#include <Entry.h>

#include <fcntl.h>
#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <kits/support/String.h>
#include <unistd.h>

//#include <compat/sys/stat.h>

#include <Directory.h>
#include <Path.h>
#include <SymLink.h>

#include <syscalls.h>

#include <storage_support.h>


using namespace std;


//	#pragma mark - struct entry_ref


entry_ref::entry_ref()
    :
    device((dev_t)-1),
    directory((ino_t)-1),
    name(NULL)
{
}


entry_ref::entry_ref(dev_t dev, ino_t dir, const char* name)
    :
    device(dev),
    directory(dir),
    name(NULL)
{
    set_name(name);
}


entry_ref::entry_ref(const entry_ref& ref)
    :
    device(ref.device),
    directory(ref.directory),
    name(NULL)
{
    set_name(ref.name);
}


entry_ref::~entry_ref()
{
    free(name);
}


status_t
entry_ref::set_name(const char* name)
{
    free(this->name);

    if (name == NULL) {
        this->name = NULL;
    } else {
        this->name = strdup(name);
        if (!this->name)
            return B_NO_MEMORY;
    }

    return B_OK;
}


bool
entry_ref::operator==(const entry_ref& ref) const
{
    return (device == ref.device
        && directory == ref.directory
        && (name == ref.name
            || (name != NULL && ref.name != NULL
                && strcmp(name, ref.name) == 0)));
}


bool
entry_ref::operator!=(const entry_ref& ref) const
{
    return !(*this == ref);
}


entry_ref&
entry_ref::operator=(const entry_ref& ref)
{
    if (this == &ref)
        return *this;

    device = ref.device;
    directory = ref.directory;
    set_name(ref.name);
    return *this;
}


//	#pragma mark - BEntry


/*BEntry::BEntry()
    :
    fDirFd(-1),
    fName(NULL),
    fCStatus(B_NO_INIT)
{
}


BEntry::BEntry(const BDirectory* dir, const char* path, bool traverse)
    :
    fDirFd(-1),
    fName(NULL),
    fCStatus(B_NO_INIT)
{
    SetTo(dir, path, traverse);
}

*/

BEntry::BEntry(const entry_ref* ref, bool traverse)
    :
    fDirFd(-1),
    fName(NULL),
    fCStatus(B_NO_INIT)
{
    SetTo(ref, traverse);
}


/*BEntry::BEntry(const char* path, bool traverse)
    :
    fDirFd(-1),
    fName(NULL),
    fCStatus(B_NO_INIT)
{
    SetTo(path, traverse);
}


BEntry::BEntry(const BEntry& entry)
    :
    fDirFd(-1),
    fName(NULL),
    fCStatus(B_NO_INIT)
{
    *this = entry;
}


BEntry::~BEntry()
{
    Unset();
}


status_t
BEntry::InitCheck() const
{
    return fCStatus;
}


bool
BEntry::Exists() const
{
    // just stat the beast
    struct stat st;
    return GetStat(&st) == B_OK;
}


const char*
BEntry::Name() const
{
    if (fCStatus != B_OK)
        return NULL;

    return fName;
}


status_t
BEntry::SetTo(const BDirectory* dir, const char* path, bool traverse)
{
    // check params
    if (!dir)
        return (fCStatus = B_BAD_VALUE);
    if (path && path[0] == '\0')	// R5 behaviour
        path = NULL;

    // if path is absolute, let the path-only SetTo() do the job
    if (BPrivate::Storage::is_absolute_path(path))
        return SetTo(path, traverse);

    Unset();

    if (dir->InitCheck() != B_OK)
        fCStatus = B_BAD_VALUE;

    // dup() the dir's FD and let set() do the rest
    int dirFD = _kern_dup(dir->get_fd());
    if (dirFD < 0)
        return (fCStatus = dirFD);
    return (fCStatus = _SetTo(dirFD, path, traverse));
}

*/

status_t
BEntry::SetTo(const entry_ref* ref, bool traverse)
{
    Unset();
    if (ref == NULL)
        return (fCStatus = B_BAD_VALUE);

    // if ref-name is absolute, let the path-only SetTo() do the job
    if (BPrivate::Storage::is_absolute_path(ref->name))
        return SetTo(ref->name, traverse);

    // open the directory and let set() do the rest
    int dirFD = _kern_open_dir_entry_ref(ref->device, ref->directory, NULL);
    if (dirFD < 0)
        return (fCStatus = dirFD);
    return (fCStatus = _SetTo(dirFD, ref->name, traverse));
}

/*
status_t
BEntry::SetTo(const char* path, bool traverse)
{
    Unset();
    // check the argument
    if (!path)
        return (fCStatus = B_BAD_VALUE);
    return (fCStatus = _SetTo(-1, path, traverse));
}


void
BEntry::Unset()
{
    // Close the directory
    if (fDirFd >= 0)
        _kern_close(fDirFd);

    // Free our leaf name
    free(fName);

    fDirFd = -1;
    fName = NULL;
    fCStatus = B_NO_INIT;
}
*/

status_t
BEntry::GetRef(entry_ref* ref) const
{
    if (fCStatus != B_OK)
        return B_NO_INIT;

    if (ref == NULL)
        return B_BAD_VALUE;

    struct stat st;
    status_t error = _kern_read_stat(fDirFd, NULL, false, &st,
        sizeof(struct stat));
    if (error == B_OK) {
        ref->device = st.st_dev;
        ref->directory = st.st_ino;
        error = ref->set_name(fName);
    }
    return error;
}


status_t
BEntry::GetPath(BPath* path) const
{
    if (fCStatus != B_OK)
        return B_NO_INIT;

    if (path == NULL)
        return B_BAD_VALUE;

    return path->SetTo(this);
}


/*
 * status_t BEntry::GetParent(BEntry* entry) const
{
    // check parameter and initialization
    if (fCStatus != B_OK)
        return B_NO_INIT;
    if (entry == NULL)
        return B_BAD_VALUE;

    // check whether we are the root directory
    // It is sufficient to check whether our leaf name is ".".
    if (strcmp(fName, ".") == 0)
        return B_ENTRY_NOT_FOUND;

    // open the parent directory
    char leafName[B_FILE_NAME_LENGTH];
    int parentFD = _kern_open_parent_dir(fDirFd, leafName, B_FILE_NAME_LENGTH);
    if (parentFD < 0)
        return parentFD;

    // set close on exec flag on dir FD
    fcntl(parentFD, F_SETFD, FD_CLOEXEC);

    // init the entry
    entry->Unset();
    entry->fDirFd = parentFD;
    entry->fCStatus = entry->_SetName(leafName);
    if (entry->fCStatus != B_OK)
        entry->Unset();
    return entry->fCStatus;
}


status_t
BEntry::GetParent(BDirectory* dir) const
{
    // check initialization and parameter
    if (fCStatus != B_OK)
        return B_NO_INIT;
    if (dir == NULL)
        return B_BAD_VALUE;
    // check whether we are the root directory
    // It is sufficient to check whether our leaf name is ".".
    if (strcmp(fName, ".") == 0)
        return B_ENTRY_NOT_FOUND;
    // get a node ref for the directory and init it
    struct stat st;
    status_t error = _kern_read_stat(fDirFd, NULL, false, &st,
        sizeof(struct stat));
    if (error != B_OK)
        return error;
    node_ref ref;
    ref.device = st.st_dev;
    ref.node = st.st_ino;
    return dir->SetTo(&ref);
    // TODO: This can be optimized: We already have a FD for the directory,
    // so we could dup() it and set it on the directory. We just need a private
    // API for being able to do that.
}
*/

status_t
BEntry::GetName(char* buffer) const
{
    if (fCStatus != B_OK)
        return B_NO_INIT;
    if (buffer == NULL)
        return B_BAD_VALUE;

    strcpy(buffer, fName);
    return B_OK;
}


status_t
BEntry::Rename(const char* path, bool clobber)
{
    // check parameter and initialization
    if (path == NULL)
        return B_BAD_VALUE;
    if (fCStatus != B_OK)
        return B_NO_INIT;
    // get an entry representing the target location
    BEntry target;
    status_t error;
    if (BPrivate::Storage::is_absolute_path(path)) {
        error = target.SetTo(path);
    } else {
        int dirFD = _kern_dup(fDirFd);
        if (dirFD < 0)
            return dirFD;
        // init the entry
        error = target.fCStatus = target._SetTo(dirFD, path, false);
    }
    if (error != B_OK)
        return error;
    return _Rename(target, clobber);
}


status_t
BEntry::MoveTo(BDirectory* dir, const char* path, bool clobber)
{
    // check parameters and initialization
    if (fCStatus != B_OK)
        return B_NO_INIT;
    if (dir == NULL)
        return B_BAD_VALUE;
    if (dir->InitCheck() != B_OK)
        return B_BAD_VALUE;
    // NULL path simply means move without renaming
    if (path == NULL)
        path = fName;
    // get an entry representing the target location
    BEntry target;
    status_t error = target.SetTo(dir, path);
    if (error != B_OK)
        return error;
    return _Rename(target, clobber);
}


status_t
BEntry::Remove()
{
    if (fCStatus != B_OK)
        return B_NO_INIT;

    if (IsDirectory())
        return _kern_remove_dir(fDirFd, fName);

    return _kern_unlink(fDirFd, fName);
}


bool
BEntry::operator==(const BEntry& item) const
{
    // First check statuses
    if (this->InitCheck() != B_OK && item.InitCheck() != B_OK) {
        return true;
    } else if (this->InitCheck() == B_OK && item.InitCheck() == B_OK) {

        // Directories don't compare well directly, so we'll
        // compare entry_refs instead
        entry_ref ref1, ref2;
        if (this->GetRef(&ref1) != B_OK)
            return false;
        if (item.GetRef(&ref2) != B_OK)
            return false;
        return (ref1 == ref2);

    } else {
        return false;
    }

}

/*
bool
BEntry::operator!=(const BEntry& item) const
{
    return !(*this == item);
}


BEntry&
BEntry::operator=(const BEntry& item)
{
    if (this == &item)
        return *this;

    Unset();
    if (item.fCStatus == B_OK) {
        fDirFd = _kern_dup(item.fDirFd);
        if (fDirFd >= 0)
            fCStatus = _SetName(item.fName);
        else
            fCStatus = fDirFd;

        if (fCStatus != B_OK)
            Unset();
    }

    return *this;
}
*/

void BEntry::_PennyEntry1(){}
void BEntry::_PennyEntry2(){}
void BEntry::_PennyEntry3(){}
void BEntry::_PennyEntry4(){}
void BEntry::_PennyEntry5(){}
void BEntry::_PennyEntry6(){}


/*!	Updates the BEntry with the data from the stat structure according
    to the \a what mask.

    \param st The stat structure to set.
    \param what A mask

    \returns A status code.
    \retval B_OK Everything went fine.
    \retval B_FILE_ERROR There was an error writing to the BEntry object.
*/
status_t
BEntry::set_stat(struct stat& st, uint32_t what)
{
    if (fCStatus != B_OK)
        return B_FILE_ERROR;

    return _kern_write_stat(fDirFd, fName, false, &st, sizeof(struct stat),
        what);
}


/*!	Sets the entry to point to the entry specified by the path \a path
    relative to the given directory.

    If \a traverse is \c true and the given entry is a symbolic link, the
    object is recursively set to point to the entry pointed to by the symlink.

    If \a path is an absolute path, \a dirFD is ignored.

    If \a dirFD is -1, \a path is considered relative to the current directory
    (unless it is an absolute path).

    The ownership of the file descriptor \a dirFD is transferred to the
    method, regardless of whether it succeeds or fails. The caller must not
    close the FD afterwards.

    \param dirFD File descriptor of a directory relative to which path is to
        be considered. May be -1 if the current directory shall be considered.
    \param path Pointer to a path relative to the given directory.
    \param traverse If \c true and the given entry is a symbolic link, the
        object is recursively set to point to the entry linked to by the
        symbolic link.

    \returns \c B_OK on success, or an error code on failure.
*/
status_t
BEntry::_SetTo(int dirFD, const char* path, bool traverse)
{
    bool requireConcrete = false;
    FDCloser fdCloser(dirFD);
    char tmpPath[B_PATH_NAME_LENGTH];
    char leafName[B_FILE_NAME_LENGTH];
    int32_t linkLimit = B_MAX_SYMLINKS;
    while (true) {
        if (!path || strcmp(path, ".") == 0) {
            // "."
            // if no dir FD is supplied, we need to open the current directory
            // first
            if (dirFD < 0) {
                dirFD = _kern_open_dir(-1, ".");
                if (dirFD < 0)
                    return dirFD;
                fdCloser.SetTo(dirFD);
            }
            // get the parent directory
            int parentFD = _kern_open_parent_dir(dirFD, leafName,
                B_FILE_NAME_LENGTH);
            if (parentFD < 0)
                return parentFD;
            dirFD = parentFD;
            fdCloser.SetTo(dirFD);
            break;
        } else if (strcmp(path, "..") == 0) {
            // ".."
            // open the parent directory
            int parentFD = _kern_open_dir(dirFD, "..");
            if (parentFD < 0)
                return parentFD;
            dirFD = parentFD;
            fdCloser.SetTo(dirFD);
            // get the parent's parent directory
            parentFD = _kern_open_parent_dir(dirFD, leafName,
                B_FILE_NAME_LENGTH);
            if (parentFD < 0)
                return parentFD;
            dirFD = parentFD;
            fdCloser.SetTo(dirFD);
            break;
        } else {
            // an ordinary path; analyze it
            char dirPath[B_PATH_NAME_LENGTH];
            status_t error = BPrivate::Storage::parse_path(path, dirPath,
                leafName);
            if (error != B_OK)
                return error;
            // special case: root directory ("/")
            if (leafName[0] == '\0' && dirPath[0] == '/')
                strcpy(leafName, ".");
            if (leafName[0] == '\0') {
                // the supplied path is already a leaf
                error = BPrivate::Storage::check_entry_name(dirPath);
                if (error != B_OK)
                    return error;
                strcpy(leafName, dirPath);
                // if no directory was given, we need to open the current dir
                // now
                if (dirFD < 0) {
                    char* cwd = getcwd(tmpPath, B_PATH_NAME_LENGTH);
                    if (!cwd)
                        return B_ERROR;
                    dirFD = _kern_open_dir(-1, cwd);
                    if (dirFD < 0)
                        return dirFD;
                    fdCloser.SetTo(dirFD);
                }
            } else if (strcmp(leafName, ".") == 0
                    || strcmp(leafName, "..") == 0) {
                // We have to resolve this to get the entry name. Just open
                // the dir and let the next iteration deal with it.
                dirFD = _kern_open_dir(-1, path);
                if (dirFD < 0)
                    return dirFD;
                fdCloser.SetTo(dirFD);
                path = NULL;
                continue;
            } else {
                int parentFD = _kern_open_dir(dirFD, dirPath);
                if (parentFD < 0)
                    return parentFD;
                dirFD = parentFD;
                fdCloser.SetTo(dirFD);
            }
            // traverse symlinks, if desired
            if (!traverse)
                break;
            struct stat st;
            error = _kern_read_stat(dirFD, leafName, false, &st,
                sizeof(struct stat));
            if (error == B_ENTRY_NOT_FOUND && !requireConcrete) {
                // that's fine -- the entry is abstract and was not target of
                // a symlink we resolved
                break;
            }
            if (error != B_OK)
                return error;
            // the entry is concrete
            if (!S_ISLNK(st.st_mode))
                break;
            requireConcrete = true;
            // we need to traverse the symlink
            if (--linkLimit < 0)
                return B_LINK_LIMIT;
            size_t bufferSize = B_PATH_NAME_LENGTH - 1;
            error = _kern_read_link(dirFD, leafName, tmpPath, &bufferSize);
            if (error < 0)
                return error;
            tmpPath[bufferSize] = '\0';
            path = tmpPath;
            // next round...
        }
    }

    // set close on exec flag on dir FD
    fcntl(dirFD, F_SETFD, FD_CLOEXEC);

    // set the result
    status_t error = _SetName(leafName);
    if (error != B_OK)
        return error;
    fdCloser.Detach();
    fDirFd = dirFD;
    return B_OK;
}


/*!	Handles string allocation, deallocation, and copying for the
    leaf name of the entry.

    \param name The leaf \a name of the entry.

    \returns A status code.
    \retval B_OK Everything went fine.
    \retval B_BAD_VALUE \a name is \c NULL.
    \retval B_NO_MEMORY Ran out of memory trying to allocate \a name.
*/
status_t
BEntry::_SetName(const char* name)
{
    if (name == NULL)
        return B_BAD_VALUE;

    free(fName);

    fName = strdup(name);
    if (fName == NULL)
        return B_NO_MEMORY;

    return B_OK;
}


/*!	Renames the entry referred to by this object to the location
    specified by \a target.

    If an entry exists at the target location, the method fails, unless
    \a clobber is \c true, in which case that entry is overwritten (doesn't
    work for non-empty directories, though).

    If the operation was successful, this entry is made a clone of the
    supplied one and the supplied one is uninitialized.

    \param target The entry specifying the target location.
    \param clobber If \c true, the an entry existing at the target location
           will be overwritten.

    \return \c B_OK, if everything went fine, another error code otherwise.
*/
status_t
BEntry::_Rename(BEntry& target, bool clobber)
{
    // check, if there's an entry in the way
    if (!clobber && target.Exists())
        return B_FILE_EXISTS;
    // rename
    status_t error = _kern_rename(fDirFd, fName, target.fDirFd, target.fName);
    if (error == B_OK) {
        Unset();
        fCStatus = target.fCStatus;
        fDirFd = target.fDirFd;
        fName = target.fName;
        target.fCStatus = B_NO_INIT;
        target.fDirFd = -1;
        target.fName = NULL;
    }
    return error;
}


/*!	Debugging function, dumps the given entry to stdout.

    \param name A pointer to a string to be printed along with the dump for
           identification purposes.
*/
void
BEntry::_Dump(const char* name)
{
    if (name != NULL) {
        printf("------------------------------------------------------------\n");
        printf("%s\n", name);
        printf("------------------------------------------------------------\n");
    }

    printf("fCStatus == %" B_PRId32 "\n", fCStatus);

    struct stat st;
    if (fDirFd != -1
        && _kern_read_stat(fDirFd, NULL, false, &st,
                sizeof(struct stat)) == B_OK) {
        printf("dir.device == %" B_PRIdDEV "\n", st.st_dev);
        printf("dir.inode  == %" B_PRIdINO "\n", st.st_ino);
    } else {
        printf("dir == NullFd\n");
    }

    printf("leaf == '%s'\n", fName);
    printf("\n");

}


status_t
BEntry::_GetStat(struct stat* st) const
{
    if (fCStatus != B_OK)
        return B_NO_INIT;

    return _kern_read_stat(fDirFd, fName, false, st, sizeof(struct stat));
}


status_t BEntry::_GetStat(struct stat_beos* st) const
{
    struct stat newStat;
    status_t error = _GetStat(&newStat);
    if (error != B_OK)
        return error;

    convert_to_stat_beos(&newStat, st);
    return B_OK;
}


// #pragma mark -


status_t get_ref_for_path(const char* path, entry_ref* ref)
{
    status_t error = path && ref ? B_OK : B_BAD_VALUE;
    if (error == B_OK) {
        BEntry entry(path);
        error = entry.InitCheck();
        if (error == B_OK)
            error = entry.GetRef(ref);
    }
    return error;
}

bool operator<(const entry_ref& a, const entry_ref& b)
{
    return (a.device < b.device
        || (a.device == b.device
            && (a.directory < b.directory
            || (a.directory == b.directory
                && ((a.name == NULL && b.name != NULL)
                || (a.name != NULL && b.name != NULL
                    && strcmp(a.name, b.name) < 0))))));
}

// #pragma mark - symbol versions

#ifdef HAIKU_TARGET_PLATFORM_LIBBE_TEST
#	if __GNUC__ == 2	// gcc 2
    B_DEFINE_SYMBOL_VERSION("_GetStat__C6BEntryP4stat",
        "GetStat__C6BEntryP4stat@@LIBBE_TEST");
#	else	// gcc 4
    // Haiku GetStat()
    B_DEFINE_SYMBOL_VERSION("_ZNK6BEntry8_GetStatEP4stat",
        "_ZNK6BEntry7GetStatEP4stat@@LIBBE_TEST");

#	endif	// gcc 4
#else	// !HAIKU_TARGET_PLATFORM_LIBBE_TEST
#	if __GNUC__ == 2	// gcc 2
    // BeOS compatible GetStat()
    B_DEFINE_SYMBOL_VERSION("_GetStat__C6BEntryP9stat_beos",
        "GetStat__C6BEntryP4stat@LIBBE_BASE");
    // Haiku GetStat()
    B_DEFINE_SYMBOL_VERSION("_GetStat__C6BEntryP4stat",
        "GetStat__C6BEntryP4stat@@LIBBE_1_ALPHA1");

#	else	// gcc 4
    // BeOS compatible GetStat()
    B_DEFINE_SYMBOL_VERSION("_ZNK6BEntry8_GetStatEP9stat_beos",
        "_ZNK6BEntry7GetStatEP4stat@LIBBE_BASE");
    // Haiku GetStat()
    B_DEFINE_SYMBOL_VERSION("_ZNK6BEntry8_GetStatEP4stat",
        "_ZNK6BEntry7GetStatEP4stat@@LIBBE_1_ALPHA1");
#	endif	// gcc 4
#endif	// !HAIKU_TARGET_PLATFORM_LIBBE_TEST
