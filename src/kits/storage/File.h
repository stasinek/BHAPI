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
 * File: File.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_FILE_H
#define BHAPI_FILE_H


#ifdef __cplusplus /* Just for C++ */
#include <Directory.h>
#include <Node.h>
#include <DataIO.h>
#include <StorageDefs.h>
#include <Haiku.h>
class BFile : public BNode, public BPositionIO {
    public:
        BFile();
        BFile(const BFile &file);
        BFile(const bhapi::entry_ref *ref, uint32_t openMode);
        BFile(const BEntry *entry, uint32_t openMode);
        BFile(const char *path, uint32_t openMode);
        BFile(const BDirectory *dir, const char *path, uint32_t openMode);
        virtual ~BFile();

        status_t SetTo(const bhapi::entry_ref *ref, uint32_t openMode);
        status_t SetTo(const BEntry *entry, uint32_t openMode);
        status_t SetTo(const char *path, uint32_t openMode);
        status_t SetTo(const BDirectory *dir, const char *path, uint32_t openMode);

        bool IsReadable() const;
        bool IsWritable() const;

        virtual ssize_t Read(void *buffer, size_t size);
        virtual ssize_t ReadAt(off_t location, void *buffer, size_t size);
        virtual ssize_t Write(const void *buffer, size_t size);
        virtual ssize_t WriteAt(off_t location, const void *buffer, size_t size);

        virtual off_t Seek(off_t offset, uint32_t seekMode);
        virtual off_t Position() const;

        virtual status_t SetSize(off_t size);
        virtual	status_t GetSize(off_t* size) const;

        BFile &operator=(const BFile &file);

    private:
        virtual void _PhiloFile1();
        virtual void _PhiloFile2();
        virtual void _PhiloFile3();
        virtual void _PhiloFile4();
        virtual void _PhiloFile5();
        virtual void _PhiloFile6();

        uint32_t _reservedData[8];

    private:
        int get_fd() const;
        virtual void close_fd();

    private:
        // The file's open mode.
        uint32_t fMode;
public:
    BFile(const char *path,  uint32_t open_mode,  uint32_t access_mode = bhapi::B_USER_READ | bhapi::B_USER_WRITE);
    BFile(const BEntry *entry,  uint32_t open_mode,  uint32_t access_mode = bhapi::B_USER_READ | bhapi::B_USER_WRITE);
    BFile(const BDirectory *dir, const char *leaf,  uint32_t open_mode,  uint32_t access_mode = bhapi::B_USER_READ | bhapi::B_USER_WRITE);

    status_t	InitCheck() const;
    status_t	SetTo(const char *path,  uint32_t open_mode,  uint32_t access_mode = bhapi::B_USER_READ | bhapi::B_USER_WRITE);
    status_t	SetTo(const BEntry *entry,  uint32_t open_mode,  uint32_t access_mode = bhapi::B_USER_READ | bhapi::B_USER_WRITE);
    status_t	SetTo(const BDirectory *dir, const char *leaf,  uint32_t open_mode,  uint32_t access_mode = bhapi::B_USER_READ | bhapi::B_USER_WRITE);
    void		Unset();

     ssize_t		ReadAt(int64 pos, void *buffer, size_t size);
     ssize_t		WriteAt(int64 pos, const void *buffer, size_t size);
     status_t	SetSize(int64 size);


private:
    void *fFD;
};
#endif /* __cplusplus */
#define BFILE_DEF
#endif /* BHAPI_FILE_H */

