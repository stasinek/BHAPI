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
 * File: TextView.h
 * Description: BTextView --- a multi-lines editable field
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_TEXT_VIEW_H
#define BHAPI_TEXT_VIEW_H

#ifdef __cplusplus /* Just for C++ */
#ifndef BFILE_DEF
#define BFILE_DEF
class BFile;
#endif
#ifndef BCLIPBOARD_DEF
#define BCLIPBOARD_DEF
class BClipboard;
#endif
#endif

#include <kits/interface/View.h>
#include <kits/support/String.h>
#include <Haiku.h>

#ifdef __cplusplus /* Just for C++ */
namespace bhapi {
#endif
typedef struct text_run {
     int32			offset;		/* byte/character(utf8 mode) offset of first byte/character(utf8 mode) of run */
    bhapi::font_desc		font;		/* font of run */
    bhapi::rgb_color		color;		/* color of run */
    bhapi::rgb_color		background;	/* background of run */
    bool			underline;	/* whether to draw underline */
#ifdef __cplusplus /* Just for C++ */
    inline text_run()
    {
        bzero(this, sizeof(*this));
    }
#endif /* __cplusplus */
} text_run;
typedef struct text_run_array {
     int32			count;		/* number of text runs */
    text_run		runs[1];	/* array of count number of runs */
} text_run_array;
#ifdef __cplusplus /* Just for C++ */
} /* namespace bhapi */
#endif

#ifdef __cplusplus /* Just for C++ */
class BHAPI_IMPEXP BTextView : public BView {
public:
    BTextView(BRect frame,
          const char *name,
          BRect textRect,
           uint32 resizeMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
           uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS);
    BTextView(BRect frame,
          const char *name,
          BRect textRect,
          const BFont *font, const bhapi::rgb_color *color,
           uint32 resizeMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
           uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS);
    virtual ~BTextView();

    const char	*Text() const;
     int32		TextLength() const; // ASCII
     int32		TextChars() const; // UTF-8

    char		ByteAt(int32 index) const; // ASCII
    const char	*CharAt(int32 index,  uint8 *length = NULL) const; // UTF-8
    void		GetText(int32 offset,  int32 length, char *buffer) const; // ASCII
    void		GetText(int32 offset,  int32 length, char *buffer,  int32 buffer_size_in_bytes, bool utf8) const;

    void		SetText(const char *text, const bhapi::text_run_array *runs = NULL, bool utf8 = false);
    void		SetText(const char *text,  int32 length, const bhapi::text_run_array *runs = NULL, bool utf8 = false);
    void		SetText(BFile *file,  int64 fileOffset,  int32 length, const bhapi::text_run_array *runs = NULL, bool utf8 = false);

    void		SetRunArray(int32 startPos,  int32 endPos, const bhapi::text_run_array *runs, bool utf8 = false); // exclude endPos

    // RunArray(): return value must free by "free"
    bhapi::text_run_array *RunArray(int32 startPos,  int32 endPos,  int32 *length = NULL, bool utf8 = false) const; // exclude endPos

    void		Insert(const char *text, const bhapi::text_run_array *runs = NULL, bool utf8 = false);
    void		Insert(const char *text,  int32 length, const bhapi::text_run_array *runs = NULL, bool utf8 = false);
    void		Insert(int32 offset, const char *text,  int32 length, const bhapi::text_run_array *runs = NULL, bool utf8 = false);
    void		Delete();
    void		Delete(int32 startPos,  int32 endPos, bool utf8 = false); // exclude endPos

    void		MakeEditable(bool editable);
    bool		IsEditable() const;

    void		MakeSelectable(bool selectable);
    bool		IsSelectable() const;

    void		SetStylable(bool stylable);
    bool		IsStylable() const;

    void		SetTabWidth(float width);
    float		TabWidth() const;

    void		SetAutoindent(bool flag);
    bool		DoesAutoindent() const;

    void		SetAlignment(bhapi::alignment alignment);
    bhapi::alignment	Alignment() const;

    void		SetMaxBytes(int32 max);
     int32		MaxBytes() const;

    // HideTyping(): flag
    // 	0x00(false)	--- show everything
    // 	0x20 ~ 0x7e	--- replace the characters with this
    // 	others(true)	--- invisible
    void		HideTyping(uint8 flag = 0x2a);
     uint8		IsTypingHidden() const;

    void		SetTextRect(BRect textRect);
    BRect		TextRect() const;

    virtual void	Select(int32 startPos,  int32 endPos, bool utf8 = false); // exclude endPos
    bool		GetSelection(int32 *startPos,  int32 *endPos, bool utf8 = false) const; // exclude endPos
    void		SelectAll();
    void		Deselect();
    bool		IsSelected() const;

     int32		CountLines() const;
     int32		CurrentLine() const;
    void		GoToLine(int32 index);

    virtual bool	AcceptsPaste(BClipboard *clipboard);

    virtual void	Cut(BClipboard *clipboard);
    virtual void	Copy(BClipboard *clipboard) const;
    virtual void	Paste(BClipboard *clipboard);
    void		Clear();

     int32		LineAt(int32 offset, bool utf8 = false) const;
     int32		LineAt(BPoint pt, bool visible = true) const;
    BPoint		PointAt(int32 offset, float *height = NULL, bool max_height = false, bool utf8 = false) const;
     int32		OffsetAt(BPoint pt, bool visible = true, bool utf8 = false) const;
     int32		OffsetAt(int32 line, bool utf8 = false) const;

    float		LineWidth(int32 lineIndex = 0) const;
    float		LineHeight(int32 lineIndex = 0) const;
    float		TextHeight(int32 fromLineIndex,  int32 toLineIndex) const;

    void		GetTextRegion(int32 startPos,  int32 endPos, BRegion *region, bool utf8 = false) const; // exclude endPos
    virtual void	ScrollToOffset(int32 offset, bool utf8 = false);
    void		ScrollToSelection();

    void		SetTextBackground(bhapi::rgb_color color);
    bhapi::rgb_color	TextBackground() const;

    void		SetPosition(int32 pos, bool response = true, bool utf8 = false);
     int32		Position(bool utf8 = false,  int32 *lineOffset = NULL) const;

    virtual void	Draw(BRect updateRect);
    virtual void	FrameResized(float new_width, float new_height);
    virtual void	MouseDown(BPoint where);
    virtual void	MouseUp(BPoint where);
    virtual void	MouseMoved(BPoint where,  uint32 code, const BMessage *a_message);
    virtual void	KeyDown(const char *bytes,  int32 numBytes);
    virtual void	KeyUp(const char *bytes,  int32 numBytes);
    virtual void	WindowActivated(bool state);
    virtual void	MakeFocus(bool focusState = true);
    virtual void	MessageReceived(BMessage *msg);
    virtual void	GetPreferredSize(float *width, float *height);

protected:
    virtual void	InsertText(const char *text,  int32 length,  int32 offset, const bhapi::text_run_array *runs, bool utf8);
    virtual void	DeleteText(int32 startPos,  int32 endPos, bool utf8);

private:
    BRect fMargins;
    BString fText;
    bhapi::text_run_array *fRunArray;

    bool fEditable;
    bool fSelectable;
    bool fStylable;
    bhapi::alignment fAlignment;
     int32 fMaxBytes;
    float fTabWidth;
    bool fAutoindent;
     uint8 fTypingHidden;

     int32 fSelectTracking;
     int32 fSelectStart;
     int32 fSelectEnd;

    BList fLines;
     int32 fCurrentLine;
     int32 fCursor;

    bhapi::rgb_color fTextBkColor;

    void ReScanRunArray(int32 fromLine,  int32 toLine);
    void ReScanSize(int32 fromLine,  int32 toLine);
    void ReScanLines();
    void FloorPosition(int32 *pos);
    void CeilPosition(int32 *pos);

    float _StringWidth(const BFont &font, const char *str,  int32 length) const;
    void _DrawString(const BFont &font, const char *str, BPoint location,  int32 length);
};
#ifdef BHAPI_BUILD_LIBRARY

inline void BTextView::SelectAll()
{
    Select(0, -1, false);
}

inline void BTextView::Deselect()
{
    Select(-1, 0, false);
}

inline bool BTextView::IsSelected() const
{
    return GetSelection(NULL, NULL, false);
}

inline void BTextView::Delete()
{
     int32 startPos = 0, endPos = 0;
    if(GetSelection(&startPos, &endPos, false)) Delete(startPos, endPos, false);
}

inline void BTextView::Clear()
{
    Delete();
}
#endif
#endif /* __cplusplus */

#endif /* BHAPI_TEXT_VIEW_H */

