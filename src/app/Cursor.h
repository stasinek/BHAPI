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
 * File: Cursor.h
 * Description: mouse cursor for application
 * 
 * --------------------------------------------------------------------------*/

#ifndef __ETK_CURSOR_H__
#define __ETK_CURSOR_H__

#include "./../support/Archivable.h"

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK ECursor : public EArchivable {
public:
	ECursor(const void *cursorData);
	ECursor(const ECursor &cursor);
	virtual ~ECursor();

	ECursor		&operator=(const ECursor &from);
	bool		operator==(const ECursor &other) const;
	bool		operator!=(const ECursor &other) const;

	const void 	*Data() const;
	euint32		DataLength() const;

	euint8		ColorDepth() const;
	euint8		Width() const;
	euint8		Height() const;

	euint16		Spot() const;
	euint8		SpotX() const;
	euint8		SpotY() const;

	const void	*Bits() const;
	const void	*Mask() const;

private:
	void *fData;
};


inline euint8 ECursor::SpotX() const
{
	return(Spot() >> 8);
}


inline euint8 ECursor::SpotY() const
{
	return(Spot() & 0xff);
}


extern _IMPEXP_ETK const ECursor *E_CURSOR_SYSTEM_DEFAULT;
extern _IMPEXP_ETK const ECursor *E_CURSOR_HAND;
extern _IMPEXP_ETK const ECursor *E_CURSOR_HAND_MOVE;
extern _IMPEXP_ETK const ECursor *E_CURSOR_I_BEAM;


#endif /* __cplusplus */

#endif /* __ETK_CURSOR_H__ */

