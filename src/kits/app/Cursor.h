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
 * File: Cursor.h
 * Description: mouse cursor for application
 *
 * --------------------------------------------------------------------------*/
#ifndef BHAPI_CURSOR_H
#define BHAPI_CURSOR_H
//-----------------------------------------------------------------------------
#include <kits/support/Archivable.h>
//-----------------------------------------------------------------------------
#include <Haiku.h>
//-----------------------------------------------------------------------------
#ifdef __cplusplus /* Just for C++ */
//-----------------------------------------------------------------------------
class BHAPI_IMPEXP BCursor : public BArchivable {
public:
    BCursor();
    BCursor(const void *cursorData);
    BCursor(const BCursor &cursor);
    virtual ~BCursor();

    BCursor		&operator=(const BCursor &from);
    bool		operator==(const BCursor &other) const;
    bool		operator!=(const BCursor &other) const;

    const void 	*Data() const;
     uint32_t		DataLength() const;

     uint8		ColorDepth() const;
     uint8		Width() const;
     uint8		Height() const;

     uint16		Spot() const;
     uint8		SpotX() const;
     uint8		SpotY() const;

    const void	*Bits() const;
    const void	*Mask() const;

private:
    void *fData;
};
//-----------------------------------------------------------------------------
#ifdef BHAPI_BUILD_LIBRARY
//-----------------------------------------------------------------------------
inline  uint8 BCursor::SpotX() const
{
    return(Spot() >> 8);
}
//-----------------------------------------------------------------------------
inline  uint8 BCursor::SpotY() const
{
    return(Spot() & 0xff);
}
#endif
//-----------------------------------------------------------------------------
extern BHAPI_IMPEXP const BCursor *B_CURSOR_SYSTEM_DEFAULT;
extern BHAPI_IMPEXP const BCursor *B_CURSOR_HAND;
extern BHAPI_IMPEXP const BCursor *B_CURSOR_HAND_MOVE;
extern BHAPI_IMPEXP const BCursor *B_CURSOR_I_BEAM;
//-----------------------------------------------------------------------------
#endif /* __cplusplus */
//-----------------------------------------------------------------------------
#endif /* BHAPI_CURSOR_H */
//-----------------------------------------------------------------------------
