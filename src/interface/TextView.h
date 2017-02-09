/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
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

#ifndef BHAPI_TEXT_VIEW__H
#define BHAPI_TEXT_VIEW__H

#include "./../support/StringMe.h"
#include "./../interface/View.h"


typedef struct b_text_run {
	b_int32			offset;		/* byte/character(utf8 mode) offset of first byte/character(utf8 mode) of run */
	b_font_desc		font;		/* font of run */
	b_rgb_color		color;		/* color of run */
	b_rgb_color		background;	/* background of run */
	bool			underline;	/* whether to draw underline */

#ifdef __cplusplus /* Just for C++ */
	inline b_text_run()
	{
		bzero(this, sizeof(struct b_text_run));
	}
#endif /* __cplusplus */
} b_text_run;


typedef struct b_text_run_array {
	b_int32			count;		/* number of text runs */
	b_text_run		runs[1];	/* array of count number of runs */
} b_text_run_array;


#ifdef __cplusplus /* Just for C++ */


class BFile;
class BClipboard;


class IMPEXP_BHAPI BTextView : public BView {
public:
	BTextView(BRect frame,
		  const char *name,
		  BRect textRect,
		  b_uint32 resizeMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		  b_uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS);
	BTextView(BRect frame,
		  const char *name,
		  BRect textRect,
		  const BFont *font, const b_rgb_color *color,
		  b_uint32 resizeMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		  b_uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS);
	virtual ~BTextView();

	const char	*Text() const;
	b_int32		TextLength() const; // ASCII
	b_int32		TextChars() const; // UTF-8

	char		ByteAt(b_int32 index) const; // ASCII
	const char	*CharAt(b_int32 index, b_uint8 *length = NULL) const; // UTF-8
	void		GetText(b_int32 offset, b_int32 length, char *buffer) const; // ASCII
	void		GetText(b_int32 offset, b_int32 length, char *buffer, b_int32 buffer_size_in_bytes, bool utf8) const;

	void		SetText(const char *text, const b_text_run_array *runs = NULL, bool utf8 = false);
	void		SetText(const char *text, b_int32 length, const b_text_run_array *runs = NULL, bool utf8 = false);
	void		SetText(BFile *file, b_int64 fileOffset, b_int32 length, const b_text_run_array *runs = NULL, bool utf8 = false);

	void		SetRunArray(b_int32 startPos, b_int32 endPos, const b_text_run_array *runs, bool utf8 = false); // exclude endPos

	// RunArray(): return value must free by "free"
	b_text_run_array *RunArray(b_int32 startPos, b_int32 endPos, b_int32 *length = NULL, bool utf8 = false) const; // exclude endPos

	void		Insert(const char *text, const b_text_run_array *runs = NULL, bool utf8 = false);
	void		Insert(const char *text, b_int32 length, const b_text_run_array *runs = NULL, bool utf8 = false);
	void		Insert(b_int32 offset, const char *text, b_int32 length, const b_text_run_array *runs = NULL, bool utf8 = false);
	void		Delete();
	void		Delete(b_int32 startPos, b_int32 endPos, bool utf8 = false); // exclude endPos

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

	void		SetAlignment(b_alignment alignment);
	b_alignment	Alignment() const;

	void		SetMaxBytes(b_int32 max);
	b_int32		MaxBytes() const;

	// HideTyping(): flag
	// 	0x00(false)	--- show everything
	// 	0x20 ~ 0x7e	--- replace the characters with this
	// 	others(true)	--- invisible
	void		HideTyping(b_uint8 flag = 0x2a);
	b_uint8		IsTypingHidden() const;

	void		SetTextRect(BRect textRect);
	BRect		TextRect() const;

	virtual void	Select(b_int32 startPos, b_int32 endPos, bool utf8 = false); // exclude endPos
	bool		GetSelection(b_int32 *startPos, b_int32 *endPos, bool utf8 = false) const; // exclude endPos
	void		SelectAll();
	void		Deselect();
	bool		IsSelected() const;

	b_int32		CountLines() const;
	b_int32		CurrentLine() const;
	void		GoToLine(b_int32 index);

	virtual bool	AcceptsPaste(BClipboard *clipboard);

	virtual void	Cut(BClipboard *clipboard);
	virtual void	Copy(BClipboard *clipboard) const;
	virtual void	Paste(BClipboard *clipboard);
	void		Clear();

	b_int32		LineAt(b_int32 offset, bool utf8 = false) const;
	b_int32		LineAt(BPoint pt, bool visible = true) const;
	BPoint		PointAt(b_int32 offset, float *height = NULL, bool max_height = false, bool utf8 = false) const;
	b_int32		OffsetAt(BPoint pt, bool visible = true, bool utf8 = false) const;
	b_int32		OffsetAt(b_int32 line, bool utf8 = false) const;

	float		LineWidth(b_int32 lineIndex = 0) const;
	float		LineHeight(b_int32 lineIndex = 0) const;
	float		TextHeight(b_int32 fromLineIndex, b_int32 toLineIndex) const;

	void		GetTextRegion(b_int32 startPos, b_int32 endPos, BRegion *region, bool utf8 = false) const; // exclude endPos
	virtual void	ScrollToOffset(b_int32 offset, bool utf8 = false);
	void		ScrollToSelection();

	void		SetTextBackground(b_rgb_color color);
	b_rgb_color	TextBackground() const;

	void		SetPosition(b_int32 pos, bool response = true, bool utf8 = false);
	b_int32		Position(bool utf8 = false, b_int32 *lineOffset = NULL) const;

	virtual void	Draw(BRect updateRect);
	virtual void	FrameResized(float new_width, float new_height);
	virtual void	MouseDown(BPoint where);
	virtual void	MouseUp(BPoint where);
	virtual void	MouseMoved(BPoint where, b_uint32 code, const BMessage *a_message);
	virtual void	KeyDown(const char *bytes, b_int32 numBytes);
	virtual void	KeyUp(const char *bytes, b_int32 numBytes);
	virtual void	WindowActivated(bool state);
	virtual void	MakeFocus(bool focusState = true);
	virtual void	MessageReceived(BMessage *msg);
	virtual void	GetPreferredSize(float *width, float *height);

protected:
	virtual void	InsertText(const char *text, b_int32 length, b_int32 offset, const b_text_run_array *runs, bool utf8);
	virtual void	DeleteText(b_int32 startPos, b_int32 endPos, bool utf8);

private:
	BRect fMargins;
    BString fText;
	b_text_run_array *fRunArray;

	bool fEditable;
	bool fSelectable;
	bool fStylable;
	b_alignment fAlignment;
	b_int32 fMaxBytes;
	float fTabWidth;
	bool fAutoindent;
	b_uint8 fTypingHidden;

	b_int32 fSelectTracking;
	b_int32 fSelectStart;
	b_int32 fSelectEnd;

	BList fLines;
	b_int32 fCurrentLine;
	b_int32 fCursor;

	b_rgb_color fTextBkColor;

	void ReScanRunArray(b_int32 fromLine, b_int32 toLine);
	void ReScanSize(b_int32 fromLine, b_int32 toLine);
	void ReScanLines();
	void FloorPosition(b_int32 *pos);
	void CeilPosition(b_int32 *pos);

	float _StringWidth(const BFont &font, const char *str, b_int32 length) const;
	void _DrawString(const BFont &font, const char *str, BPoint location, b_int32 length);
};


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
	b_int32 startPos = 0, endPos = 0;
	if(GetSelection(&startPos, &endPos, false)) Delete(startPos, endPos, false);
}


inline void BTextView::Clear()
{
	Delete();
}


#endif /* __cplusplus */

#endif /* BHAPI_TEXT_VIEW__H */

