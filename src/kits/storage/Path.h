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
 * File: Path.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_PATH_H
#define BHAPI_PATH_H

#include <Flattenable.h>
#include <Message.h>

// Forward declarations
class BDirectory;
class BEntry;
namespace bhapi {
struct entry_ref;
}

#include <Haiku.h>
#define B_MAXPATH 255

#ifdef __cplusplus /* Just for C++ */

class BHAPI_IMPEXP BPath {
public:
                            BPath();
                            BPath(const BPath& path);
                            BPath(const bhapi::entry_ref* ref);
                            BPath(const BEntry* entry);
                            BPath(const char* dir, const char* leaf = NULL,
                                bool normalize = false);
                            BPath(const BDirectory* dir,
                                const char* leaf = NULL,
                                bool normalize = false);

    virtual					~BPath();

            status_t		InitCheck() const;

            status_t		SetTo(const bhapi::entry_ref* ref);
            status_t		SetTo(const BEntry* entry);
            status_t		SetTo(const char* path, const char* leaf = NULL,
                                bool normalize = false);
            status_t		SetTo(const BDirectory* dir,
                                const char* leaf = NULL,
                                bool normalize = false);
            void			Unset();

            status_t		Append(const char* path, bool normalize = false);

            const char*		Path() const;
            const char*		Leaf() const;
            status_t		GetParent(BPath* path) const;
            bool			IsAbsolute() const;

            bool			operator==(const BPath& item) const;
            bool			operator==(const char* path) const;
            bool			operator!=(const BPath& item) const;
            bool			operator!=(const char* path) const;
            BPath&			operator=(const BPath& item);
            BPath&			operator=(const char* path);

    // BFlattenable protocol
    virtual	bool			IsFixedSize() const;
    virtual	type_code		TypeCode() const;
    virtual	ssize_t			FlattenedSize() const;
    virtual	status_t		Flatten(void* buffer, ssize_t size) const;
    virtual	bool			AllowsTypeCode(type_code code) const;
    virtual	status_t		Unflatten(type_code code, const void* buffer,
                                ssize_t size);

private:
    virtual void			_WarPath1();
    virtual void			_WarPath2();
    virtual void			_WarPath3();

            status_t		_SetPath(const char* path);
    static	bool			_MustNormalize(const char* path, status_t* _error);

            uint32			_reserved[4];

            char*			fName;
                                // Pointer to the path string of the object.
            status_t		fCStatus;
                                // The initialization status of the object.
private:
    char *fPath;
};
#endif /* __cplusplus */
#endif /* BHAPI_PATH_H */

