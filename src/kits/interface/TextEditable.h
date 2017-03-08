/* --------------------------------------------------------------------------
 *
 * BHAPI++ ---BeOS/Haiku API previously ETK++, The Easy Toolkit for C++ programing
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
 * File: TextEditable.h
 * Description: BTextEditable --- a single-line editable field
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_TEXT_EDITABLE_H
#define BHAPI_TEXT_EDITABLE_H

#ifdef __cplusplus /* Just for C++ */
class BRect;
class BMessage;
#include "../interface/Control.h"
#include "../interface/InterfaceDefs.h"
#include "../support/SupportDefs.h"
class IMPEXPBHAPI BTextEditable : public BControl {
public:
    BTextEditable(BRect frame,
              const char *name,
              const char *initial_text,
              BMessage *message,
               __be_uint32 resizeMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
               __be_uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS);
    virtual ~BTextEditable();

    void		MakeEditable(bool editable);
    bool		IsEditable() const;

    void		SetPosition(__be_int32 pos); // UTF-8
     __be_int32		Position() const; // UTF-8

    virtual void	SetText(const char *text);
    void		SetText(const BString &text);
    const char	*Text() const;
    char		*DuplicateText(__be_int32 startPos,  __be_int32 endPos); // UTF-8, return value must free by "free"

    void		SetMaxChars(__be_int32 max);
     __be_int32		MaxChars() const;

    // HideTyping(): flag
    // 	0x00(false)	--- show everything
    // 	0x20 ~ 0x7e	--- replace the characters with this
    // 	others(true)	--- invisible
    void		HideTyping(__be_uint8 flag = 0x2a);
     __be_uint8		IsTypingHidden() const;

    void		InsertText(const char *text,  __be_int32 nChars = -1,  __be_int32 position = -1); // UTF-8
    void		RemoveText(__be_int32 startPos,  __be_int32 endPos); // UTF-8, include endPos
    void		MakeEmpty();

    void		MakeSelectable(bool selectable);
    bool		IsSelectable() const;

    virtual void	Select(__be_int32 startPos,  __be_int32 endPos); // UTF-8, include endPos
    bool		GetSelection(__be_int32 *startPos,  __be_int32 *endPos) const; // UTF-8, include endPos
    void		SelectAll();
    void		Deselect();
    bool		IsSelected() const;

    void		SetTextAlignment(bhapi::alignment alignment);
    bhapi::alignment	TextAlignment() const;

    virtual void	SetMargins(float left, float top, float right, float bottom);
    void		GetMargins(float *left, float *top, float *right, float *bottom) const;

    virtual void	Draw(BRect updateRect);
    virtual void	FrameResized(float new_width, float new_height);
    virtual void	MouseDown(BPoint where);
    virtual void	MouseUp(BPoint where);
    virtual void	MouseMoved(BPoint where,  __be_uint32 code, const BMessage *a_message);
    virtual void	WindowActivated(bool state);
    virtual void	KeyDown(const char *bytes,  __be_int32 numBytes);
    virtual void	KeyUp(const char *bytes,  __be_int32 numBytes);
    virtual void	SetFont(const BFont *font,  __be_uint8 mask = B_FONT_ALL);
    virtual void	GetPreferredSize(float *width, float *height);
    virtual void	MakeFocus(bool focusState = true);
    virtual void	MessageReceived(BMessage *msg);

private:
    char *fText;
    bool fEditable;
    bool fSelectable;
    bhapi::alignment fAlignment;
     __be_int32 fPosition;
     __be_int32 fSelectStart;
     __be_int32 fSelectEnd;
    BRect fMargins;
    float *fCharWidths;
     __be_int32 fCount;
    float locationOffset;
     __be_int32 fSelectTracking;
     __be_int32 fMaxChars;
     __be_uint8 fTypingHidden;

    void DrawSelectedBackground(BRect updateRect);
    void DrawCursor();

    bool GetCharLocation(__be_int32 pos, float *x, float *y, BFont *font = NULL);
    float _StringWidth(const BFont &font, const char *str) const;
    float *_CharWidths(const BFont &font, const char *str,  __be_int32 *count) const;
    void _DrawString(const char *str, BPoint location);
};
#ifdef BHAPI_BUILD_LIBRARY

inline void BTextEditable::MakeEmpty()
{
    SetText(NULL);
}


inline void BTextEditable::SelectAll()
{
    Select(0, -1);
}

inline void BTextEditable::Deselect()
{
    Select(-1, 0);
}


inline bool BTextEditable::IsSelected() const
{
    return GetSelection(NULL, NULL);
}

#endif // BHAPI_BUILD_LIBRARY
#endif /* __cplusplus */

#endif /* BHAPI_TEXT_EDITABLE_H */

