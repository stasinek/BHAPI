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
     int32_t			offset;		/* byte/character(utf8 mode) offset of first byte/character(utf8 mode) of run */
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
     int32_t			count;		/* number of text runs */
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
           uint32_t resizeMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
           uint32_t flags = B_WILL_DRAW | B_FRAME_EVENTS);
    BTextView(BRect frame,
          const char *name,
          BRect textRect,
          const BFont *font, const bhapi::rgb_color *color,
           uint32_t resizeMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
           uint32_t flags = B_WILL_DRAW | B_FRAME_EVENTS);
    virtual ~BTextView();

    const char	*Text() const;
     int32_t		TextLength() const; // ASCII
     int32_t		TextChars() const; // UTF-8

    char		ByteAt(int32_t index) const; // ASCII
    const char	*CharAt(int32_t index,  uint8 *length = NULL) const; // UTF-8
    void		GetText(int32_t offset,  int32_t length, char *buffer) const; // ASCII
    void		GetText(int32_t offset,  int32_t length, char *buffer,  int32_t buffer_size_in_bytes, bool utf8) const;

    void		SetText(const char *text, const bhapi::text_run_array *runs = NULL, bool utf8 = false);
    void		SetText(const char *text,  int32_t length, const bhapi::text_run_array *runs = NULL, bool utf8 = false);
    void		SetText(BFile *file,  int64 fileOffset,  int32_t length, const bhapi::text_run_array *runs = NULL, bool utf8 = false);

    void		SetRunArray(int32_t startPos,  int32_t endPos, const bhapi::text_run_array *runs, bool utf8 = false); // exclude endPos

    // RunArray(): return value must free by "free"
    bhapi::text_run_array *RunArray(int32_t startPos,  int32_t endPos,  int32_t *length = NULL, bool utf8 = false) const; // exclude endPos

    void		Insert(const char *text, const bhapi::text_run_array *runs = NULL, bool utf8 = false);
    void		Insert(const char *text,  int32_t length, const bhapi::text_run_array *runs = NULL, bool utf8 = false);
    void		Insert(int32_t offset, const char *text,  int32_t length, const bhapi::text_run_array *runs = NULL, bool utf8 = false);
    void		Delete();
    void		Delete(int32_t startPos,  int32_t endPos, bool utf8 = false); // exclude endPos

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

    void		SetMaxBytes(int32_t max);
     int32_t		MaxBytes() const;

    // HideTyping(): flag
    // 	0x00(false)	--- show everything
    // 	0x20 ~ 0x7e	--- replace the characters with this
    // 	others(true)	--- invisible
    void		HideTyping(uint8 flag = 0x2a);
     uint8		IsTypingHidden() const;

    void		SetTextRect(BRect textRect);
    BRect		TextRect() const;

    virtual void	Select(int32_t startPos,  int32_t endPos, bool utf8 = false); // exclude endPos
    bool		GetSelection(int32_t *startPos,  int32_t *endPos, bool utf8 = false) const; // exclude endPos
    void		SelectAll();
    void		Deselect();
    bool		IsSelected() const;

     int32_t		CountLines() const;
     int32_t		CurrentLine() const;
    void		GoToLine(int32_t index);

    virtual bool	AcceptsPaste(BClipboard *clipboard);

    virtual void	Cut(BClipboard *clipboard);
    virtual void	Copy(BClipboard *clipboard) const;
    virtual void	Paste(BClipboard *clipboard);
    void		Clear();

     int32_t		LineAt(int32_t offset, bool utf8 = false) const;
     int32_t		LineAt(BPoint pt, bool visible = true) const;
    BPoint		PointAt(int32_t offset, float *height = NULL, bool max_height = false, bool utf8 = false) const;
     int32_t		OffsetAt(BPoint pt, bool visible = true, bool utf8 = false) const;
     int32_t		OffsetAt(int32_t line, bool utf8 = false) const;

    float		LineWidth(int32_t lineIndex = 0) const;
    float		LineHeight(int32_t lineIndex = 0) const;
    float		TextHeight(int32_t fromLineIndex,  int32_t toLineIndex) const;

    void		GetTextRegion(int32_t startPos,  int32_t endPos, BRegion *region, bool utf8 = false) const; // exclude endPos
    virtual void	ScrollToOffset(int32_t offset, bool utf8 = false);
    void		ScrollToSelection();

    void		SetTextBackground(bhapi::rgb_color color);
    bhapi::rgb_color	TextBackground() const;

    void		SetPosition(int32_t pos, bool response = true, bool utf8 = false);
     int32_t		Position(bool utf8 = false,  int32_t *lineOffset = NULL) const;

    virtual void	Draw(BRect updateRect);
    virtual void	FrameResized(float new_width, float new_height);
    virtual void	MouseDown(BPoint where);
    virtual void	MouseUp(BPoint where);
    virtual void	MouseMoved(BPoint where,  uint32_t code, const BMessage *a_message);
    virtual void	KeyDown(const char *bytes,  int32_t numBytes);
    virtual void	KeyUp(const char *bytes,  int32_t numBytes);
    virtual void	WindowActivated(bool state);
    virtual void	MakeFocus(bool focusState = true);
    virtual void	MessageReceived(BMessage *msg);
    virtual void	GetPreferredSize(float *width, float *height);

protected:
    virtual void	InsertText(const char *text,  int32_t length,  int32_t offset, const bhapi::text_run_array *runs, bool utf8);
    virtual void	DeleteText(int32_t startPos,  int32_t endPos, bool utf8);

private:
    BRect fMargins;
    BString fText;
    bhapi::text_run_array *fRunArray;

    bool fEditable;
    bool fSelectable;
    bool fStylable;
    bhapi::alignment fAlignment;
     int32_t fMaxBytes;
    float fTabWidth;
    bool fAutoindent;
     uint8 fTypingHidden;

     int32_t fSelectTracking;
     int32_t fSelectStart;
     int32_t fSelectEnd;

    BList fLines;
     int32_t fCurrentLine;
     int32_t fCursor;

    bhapi::rgb_color fTextBkColor;

    void ReScanRunArray(int32_t fromLine,  int32_t toLine);
    void ReScanSize(int32_t fromLine,  int32_t toLine);
    void ReScanLines();
    void FloorPosition(int32_t *pos);
    void CeilPosition(int32_t *pos);

    float _StringWidth(const BFont &font, const char *str,  int32_t length) const;
    void _DrawString(const BFont &font, const char *str, BPoint location,  int32_t length);
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
     int32_t startPos = 0, endPos = 0;
    if(GetSelection(&startPos, &endPos, false)) Delete(startPos, endPos, false);
}

inline void BTextView::Clear()
{
    Delete();
}
#endif
#endif /* __cplusplus */

#endif /* BHAPI_TEXT_VIEW_H */

