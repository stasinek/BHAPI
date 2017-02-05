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
 * File: TextView.h
 * Description: ETextView --- a multi-lines editable field
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_TEXT_VIEW_H__
#define __ETK_TEXT_VIEW_H__

#include "./../support/String.h"
#include "./../interface/View.h"


typedef struct e_text_run {
	eint32			offset;		/* byte/character(utf8 mode) offset of first byte/character(utf8 mode) of run */
	e_font_desc		font;		/* font of run */
	e_rgb_color		color;		/* color of run */
	e_rgb_color		background;	/* background of run */
	bool			underline;	/* whether to draw underline */

#ifdef __cplusplus /* Just for C++ */
	inline e_text_run()
	{
		bzero(this, sizeof(struct e_text_run));
	}
#endif /* __cplusplus */
} e_text_run;


typedef struct e_text_run_array {
	eint32			count;		/* number of text runs */
	e_text_run		runs[1];	/* array of count number of runs */
} e_text_run_array;


#ifdef __cplusplus /* Just for C++ */


class EFile;
class EClipboard;


class _IMPEXP_ETK ETextView : public EView {
public:
	ETextView(ERect frame,
		  const char *name,
		  ERect textRect,
		  euint32 resizeMode = E_FOLLOW_LEFT | E_FOLLOW_TOP,
		  euint32 flags = E_WILL_DRAW | E_FRAME_EVENTS);
	ETextView(ERect frame,
		  const char *name,
		  ERect textRect,
		  const EFont *font, const e_rgb_color *color,
		  euint32 resizeMode = E_FOLLOW_LEFT | E_FOLLOW_TOP,
		  euint32 flags = E_WILL_DRAW | E_FRAME_EVENTS);
	virtual ~ETextView();

	const char	*Text() const;
	eint32		TextLength() const; // ASCII
	eint32		TextChars() const; // UTF-8

	char		ByteAt(eint32 index) const; // ASCII
	const char	*CharAt(eint32 index, euint8 *length = NULL) const; // UTF-8
	void		GetText(eint32 offset, eint32 length, char *buffer) const; // ASCII
	void		GetText(eint32 offset, eint32 length, char *buffer, eint32 buffer_size_in_bytes, bool utf8) const;

	void		SetText(const char *text, const e_text_run_array *runs = NULL, bool utf8 = false);
	void		SetText(const char *text, eint32 length, const e_text_run_array *runs = NULL, bool utf8 = false);
	void		SetText(EFile *file, eint64 fileOffset, eint32 length, const e_text_run_array *runs = NULL, bool utf8 = false);

	void		SetRunArray(eint32 startPos, eint32 endPos, const e_text_run_array *runs, bool utf8 = false); // exclude endPos

	// RunArray(): return value must free by "free"
	e_text_run_array *RunArray(eint32 startPos, eint32 endPos, eint32 *length = NULL, bool utf8 = false) const; // exclude endPos

	void		Insert(const char *text, const e_text_run_array *runs = NULL, bool utf8 = false);
	void		Insert(const char *text, eint32 length, const e_text_run_array *runs = NULL, bool utf8 = false);
	void		Insert(eint32 offset, const char *text, eint32 length, const e_text_run_array *runs = NULL, bool utf8 = false);
	void		Delete();
	void		Delete(eint32 startPos, eint32 endPos, bool utf8 = false); // exclude endPos

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

	void		SetAlignment(e_alignment alignment);
	e_alignment	Alignment() const;

	void		SetMaxBytes(eint32 max);
	eint32		MaxBytes() const;

	// HideTyping(): flag
	// 	0x00(false)	--- show everything
	// 	0x20 ~ 0x7e	--- replace the characters with this
	// 	others(true)	--- invisible
	void		HideTyping(euint8 flag = 0x2a);
	euint8		IsTypingHidden() const;

	void		SetTextRect(ERect textRect);
	ERect		TextRect() const;

	virtual void	Select(eint32 startPos, eint32 endPos, bool utf8 = false); // exclude endPos
	bool		GetSelection(eint32 *startPos, eint32 *endPos, bool utf8 = false) const; // exclude endPos
	void		SelectAll();
	void		Deselect();
	bool		IsSelected() const;

	eint32		CountLines() const;
	eint32		CurrentLine() const;
	void		GoToLine(eint32 index);

	virtual bool	AcceptsPaste(EClipboard *clipboard);

	virtual void	Cut(EClipboard *clipboard);
	virtual void	Copy(EClipboard *clipboard) const;
	virtual void	Paste(EClipboard *clipboard);
	void		Clear();

	eint32		LineAt(eint32 offset, bool utf8 = false) const;
	eint32		LineAt(EPoint pt, bool visible = true) const;
	EPoint		PointAt(eint32 offset, float *height = NULL, bool max_height = false, bool utf8 = false) const;
	eint32		OffsetAt(EPoint pt, bool visible = true, bool utf8 = false) const;
	eint32		OffsetAt(eint32 line, bool utf8 = false) const;

	float		LineWidth(eint32 lineIndex = 0) const;
	float		LineHeight(eint32 lineIndex = 0) const;
	float		TextHeight(eint32 fromLineIndex, eint32 toLineIndex) const;

	void		GetTextRegion(eint32 startPos, eint32 endPos, ERegion *region, bool utf8 = false) const; // exclude endPos
	virtual void	ScrollToOffset(eint32 offset, bool utf8 = false);
	void		ScrollToSelection();

	void		SetTextBackground(e_rgb_color color);
	e_rgb_color	TextBackground() const;

	void		SetPosition(eint32 pos, bool response = true, bool utf8 = false);
	eint32		Position(bool utf8 = false, eint32 *lineOffset = NULL) const;

	virtual void	Draw(ERect updateRect);
	virtual void	FrameResized(float new_width, float new_height);
	virtual void	MouseDown(EPoint where);
	virtual void	MouseUp(EPoint where);
	virtual void	MouseMoved(EPoint where, euint32 code, const EMessage *a_message);
	virtual void	KeyDown(const char *bytes, eint32 numBytes);
	virtual void	KeyUp(const char *bytes, eint32 numBytes);
	virtual void	WindowActivated(bool state);
	virtual void	MakeFocus(bool focusState = true);
	virtual void	MessageReceived(EMessage *msg);
	virtual void	GetPreferredSize(float *width, float *height);

protected:
	virtual void	InsertText(const char *text, eint32 length, eint32 offset, const e_text_run_array *runs, bool utf8);
	virtual void	DeleteText(eint32 startPos, eint32 endPos, bool utf8);

private:
	ERect fMargins;
	EString fText;
	e_text_run_array *fRunArray;

	bool fEditable;
	bool fSelectable;
	bool fStylable;
	e_alignment fAlignment;
	eint32 fMaxBytes;
	float fTabWidth;
	bool fAutoindent;
	euint8 fTypingHidden;

	eint32 fSelectTracking;
	eint32 fSelectStart;
	eint32 fSelectEnd;

	EList fLines;
	eint32 fCurrentLine;
	eint32 fCursor;

	e_rgb_color fTextBkColor;

	void ReScanRunArray(eint32 fromLine, eint32 toLine);
	void ReScanSize(eint32 fromLine, eint32 toLine);
	void ReScanLines();
	void FloorPosition(eint32 *pos);
	void CeilPosition(eint32 *pos);

	float _StringWidth(const EFont &font, const char *str, eint32 length) const;
	void _DrawString(const EFont &font, const char *str, EPoint location, eint32 length);
};


inline void ETextView::SelectAll()
{
	Select(0, -1, false);
}


inline void ETextView::Deselect()
{
	Select(-1, 0, false);
}


inline bool ETextView::IsSelected() const
{
	return GetSelection(NULL, NULL, false);
}


inline void ETextView::Delete()
{
	eint32 startPos = 0, endPos = 0;
	if(GetSelection(&startPos, &endPos, false)) Delete(startPos, endPos, false);
}


inline void ETextView::Clear()
{
	Delete();
}


#endif /* __cplusplus */

#endif /* __ETK_TEXT_VIEW_H__ */

