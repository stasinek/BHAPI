/* --------------------------------------------------------------------------
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
 * File: Entry.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_ENTRY_H
#define BHAPI_ENTRY_H

#include <Path.h>
#include <Statable.h>
#include <Haiku.h>

#ifdef __cplusplus /* Just for C++ */
namespace bhapi {
#endif

struct entry_ref {
                                entry_ref();
                                entry_ref(dev_t dev, ino_t dir,
                                    const char* name);
                                entry_ref(const entry_ref& ref);
                                ~entry_ref();

            status_t			set_name(const char* name);

            bool				operator==(const entry_ref& ref) const;
            bool				operator!=(const entry_ref& ref) const;
            entry_ref&			operator=(const entry_ref& ref);

            dev_t				device;
            ino_t				directory;
            char*				name;
};

#ifdef __cplusplus /* Just for C++ */
}
#endif

#ifdef __cplusplus /* Just for C++ */

class AllocationInfo;
class BDirectory;
class BPath;
class EntryIterator;
class Node;

class BEntry : public BStatable {
public:
                                BEntry();
                                BEntry(const BDirectory* dir, const char* path,
                                    bool traverse = false);
                                BEntry(const bhapi::entry_ref* ref,
                                    bool traverse = false);
                                BEntry(const char* path, bool traverse = false);
                                BEntry(const BEntry& entry);
    virtual						~BEntry();

            status_t			InitCheck() const;
            bool				Exists() const;

            const char*			Name() const;

    virtual status_t			GetStat(struct stat* stat) const;

            status_t			SetTo(const BDirectory* dir, const char* path,
                                   bool traverse = false);
            status_t			SetTo(const bhapi::entry_ref* ref,
                                    bool traverse = false);
            status_t			SetTo(const char* path, bool traverse = false);
            void				Unset();

            status_t			GetRef(bhapi::entry_ref* ref) const;
            status_t			GetPath(BPath* path) const;
            status_t			GetParent(BEntry* entry) const;
            status_t			GetParent(BDirectory* dir) const;
            status_t			GetName(char* buffer) const;

            status_t			Rename(const char* path, bool clobber = false);
            status_t			MoveTo(BDirectory* dir, const char* path = NULL,
                                    bool clobber = false);
            status_t			Remove();

            bool				operator==(const BEntry& item) const;
            bool				operator!=(const BEntry& item) const;

            BEntry&				operator=(const BEntry& item);

private:
            friend class BDirectory;
            friend class BFile;
            friend class BNode;
            friend class BSymLink;

    virtual	void				_PennyEntry1();
    virtual	void				_PennyEntry2();
    virtual	void				_PennyEntry3();
    virtual	void				_PennyEntry4();
    virtual	void				_PennyEntry5();
    virtual	void				_PennyEntry6();

    // BEntry implementation of BStatable::set_stat()
    virtual	status_t			set_stat(struct stat& stat, uint32_t what);
            status_t			_SetTo(int dir, const char* path,
                                    bool traverse);
            status_t			_SetName(const char* name);

            status_t			_Rename(BEntry& target, bool clobber);

            void				_Dump(const char* name = NULL);

            status_t			_GetStat(struct stat* stat) const;
    virtual status_t			_GetStat(struct stat_beos* stat) const;

private:
            int					fDirFd;
            char*				fName;
            status_t			fCStatus;

            uint32_t				_reserved[4];
public:
    status_t	SetTo(const char *dir, const char *leaf, bool traverse = false);
    bool		IsHidden() const;
    bool		IsFile() const;
    bool		IsDirectory() const;
    bool		IsSymLink() const;

    status_t	GetSize(int64 *file_size) const;
    status_t	GetModificationTime(bigtime_t *time) const;
    status_t	GetCreationTime(bigtime_t *time) const;
    status_t	GetAccessTime(bigtime_t *time) const;

    status_t	GetName(char *buffer, size_t bufferSize) const;
    const char* Path() const;

    status_t	GetParent(BPath *path) const;
};

status_t get_ref_for_path(const char* path, bhapi::entry_ref* ref);
bool operator<(const bhapi::entry_ref& a, const bhapi::entry_ref& b);

#endif /* __cplusplus */
#endif /* BHAPI_ENTRY_H */

