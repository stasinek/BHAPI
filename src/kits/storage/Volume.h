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
 * File: Volume.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_VOLUME_H
#define BHAPI_VOLUME_H

#ifdef __cplusplus /* Just for C++ */
#ifndef BSTRING_DEF
#define BSTRING_DEF
class BString;
#endif
#ifndef BDIRECTORY_DEF
#define BDIRECTORY_DEF
class BDirectory;
#endif
#include <Haiku.h>
class BHAPI_IMPEXP BVolume {
public:
    BVolume();
    BVolume(b_dev_t dev);
    BVolume(const BVolume &from);
    virtual ~BVolume();

    status_t	InitCheck() const;
    status_t	SetTo(b_dev_t dev);
    void		Unset();

    b_dev_t		Device() const;

    status_t	GetName(char *name, size_t nameSize) const;
    status_t	GetName(BString *name) const;
    status_t	SetName(const char *name);

    status_t	GetRootDirectory(BDirectory *dir) const;

    bool		operator==(const BVolume &vol) const;
    bool		operator!=(const BVolume &vol) const;
    BVolume&	operator=(const BVolume &vol);

private:
    b_dev_t fDevice;
    void *fData;
};
#define BVOLUME_DEF
#endif /* __cplusplus */
#endif /* BHAPI_VOLUME_H */

