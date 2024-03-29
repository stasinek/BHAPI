﻿/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
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
 * File: Token.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_PRIVATE_TOKEN_H
#define BHAPI_PRIVATE_TOKEN_H
#include <kits/support/SupportDefs.h>
#include <kits/support/Locker.h>

#ifdef __cplusplus /* Just for C++ */
class BTokensDepot;
class BHAPI_LOCAL BToken {
public:
    BToken();
    ~BToken();

    bool		IsValid() const;
     uint64		Token() const;
    bigtime_t	TimeStamp() const;

    BToken		&operator+=(uint64 vitalities);
    BToken		&operator-=(uint64 vitalities);
    BToken		&operator++();
    BToken		&operator--();
     uint64		Vitalities() const;

    void		*Data() const;
    void		SetData(void *data);

    BTokensDepot	*Depot() const;
    void		MakeEmpty();

private:
    friend class BTokensDepot;

    bool fOriginal;
     uint64 fToken;
    bigtime_t fTimeStamp;

    BTokensDepot *fDepot;
};


class BHAPI_LOCAL BTokensDepot {
public:
    BTokensDepot(BLocker *locker = NULL,
             bool deconstruct_locker = false);
    virtual ~BTokensDepot();

    // CreateToken : return a new allocated object
    BToken		*CreateToken(void *data = NULL);
    // OpenToken : return an allocated object associated with "token" when "fetch_token = NULL"
    BToken		*OpenToken(uint64 token, BToken *fetch_token = NULL);
    // FetchToken : return the static object associated with "token", it should be called within Lock()/Unlock()
    BToken		*FetchToken(uint64 token);

    void		SetLocker(BLocker *locker, bool deconstruct_locker);
    BLocker		*Locker() const;

    bool		Lock();
    void		Unlock();

private:
    friend class BToken;

    BLocker *fLocker;
    bool fDeconstructLocker;
    BToken fToken;

    void *fData;
};


#endif /* __cplusplus */

#endif /* BHAPI_PRIVATE_TOKEN_H */

