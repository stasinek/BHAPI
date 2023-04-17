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
 * File: Clipboard.h
 * Description: an interface to a clipboard
 * Warning: unfinished, just for system clipboard
 *
 * --------------------------------------------------------------------------*/
#ifndef BHAPI_CLIPBOARD_H
#define BHAPI_CLIPBOARD_H
//-----------------------------------------------------------------------------
#include <kits/app/Messenger.h>
#include <kits/support/Locker.h>
//-----------------------------------------------------------------------------
#include <Haiku.h>
//-----------------------------------------------------------------------------
#ifdef __cplusplus /* Just for C++ */
//-----------------------------------------------------------------------------
enum {
    B_CLIPBOARD_CHANGED = 'CLCH'
};

class BHAPI_IMPEXP BClipboard {
public:
                                BClipboard(const char* name,
                                    bool transient = false);
    virtual						~BClipboard();

            const char*			Name() const;

            uint32_t				LocalCount() const;
            uint32_t				SystemCount() const;
            status_t			StartWatching(BMessenger target);
            status_t			StopWatching(BMessenger target);

            bool				Lock();
            void				Unlock();
            bool				IsLocked() const;

            status_t			Clear();
            status_t			Commit();
            status_t			Commit(bool failIfChanged);
            status_t			Revert();

            BMessenger			DataSource() const;
            BMessage*			Data() const;

private:
                                BClipboard(const BClipboard&);
            BClipboard&			operator=(const BClipboard&);

    virtual	void				_ReservedClipboard1();
    virtual	void				_ReservedClipboard2();
    virtual	void				_ReservedClipboard3();

            bool				_AssertLocked() const;
            status_t			_DownloadFromSystem(bool force = false);
            status_t			_UploadToSystem();

            uint32_t				_reserved0;
            BMessage*			fData;
            BLocker				fLock;
            BMessenger			fClipHandler;
            BMessenger			fDataSource;
            uint32_t				fCount;
            char*				fName;
            uint32_t				_reserved[4];
public:
     int64		CountLocks() const;

private:
    BLocker fLocker;
};
//-----------------------------------------------------------------------------
namespace bhapi {
extern BHAPI_IMPEXP BClipboard* be_clipboard;
}
#endif /* __cplusplus */
//-----------------------------------------------------------------------------
#endif /* BHAPI_CLIPBOARD_H */
//-----------------------------------------------------------------------------

