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
 * File: TextEditable.cpp
 * Description: ETextEditable --- a single-line editable field
 * 
 * --------------------------------------------------------------------------*/

#include "./../support/String.h>
#include "./../app/Application.h>

#include "Window.h"
#include "TextEditable.h"


ETextEditable::ETextEditable(ERect frame,
			     const char *name,
       			     const char *initial_text,
       			     EMessage *message,
       			     euint32 resizeMode,
       			     euint32 flags)
	: EControl(frame, name, NULL, message, resizeMode, flags),
	  fText(NULL), fEditable(true), fSelectable(true), fAlignment(E_ALIGN_LEFT), fPosition(0), fSelectStart(-1), fSelectEnd(-1),
	  fCharWidths(NULL), fCount(0), locationOffset(0), fSelectTracking(-1), fMaxChars(E_MAXINT32), fTypingHidden(0)
{
	fMargins = ERect(0, 0, 0, 0);
	if(initial_text)
	{
		fText = EStrdup(initial_text);
		if(fText)
		{
			EFont font;
			GetFont(&font);
			fCharWidths = _CharWidths(font, fText, &fCount);
		}
	}
}


ETextEditable::~ETextEditable()
{
	if(fText) delete[] fText;
	if(fCharWidths) delete[] fCharWidths;
}


void
ETextEditable::MakeEditable(bool editable)
{
	if(fEditable != editable)
	{
		fEditable = editable;
		if(!fEditable && !fSelectable)
		{
			fPosition = 0;
			locationOffset = 0;
		}
		Invalidate();
	}
}


bool
ETextEditable::IsEditable() const
{
	return fEditable;
}


void
ETextEditable::MakeSelectable(bool selectable)
{
	if(fSelectable != selectable)
	{
		fSelectable = selectable;
		fSelectTracking = -1;
		if(!fSelectable && !fEditable)
		{
			fPosition = 0;
			locationOffset = 0;
		}
		Invalidate();
	}
}


bool
ETextEditable::IsSelectable() const
{
	return fSelectable;
}


void
ETextEditable::SetTextAlignment(e_alignment alignment)
{
	if(fAlignment != alignment)
	{
		fAlignment = alignment;
		locationOffset = 0;
		Invalidate();
	}
}


e_alignment
ETextEditable::TextAlignment() const
{
	return fAlignment;
}


void
ETextEditable::SetPosition(eint32 pos)
{
	if(pos < 0 || pos > fCount) pos = fCount;

	fSelectStart = fSelectEnd = -1;
	fSelectTracking = -1;

	// call virtual function
	Select(fSelectStart, fSelectEnd);

	if(fPosition != pos)
	{
		fPosition = pos;
		Invalidate();
	}
}


eint32
ETextEditable::Position() const
{
	return fPosition;
}


void
ETextEditable::SetText(const char *str)
{
	if(fText) delete[] fText;

	const char *end = e_utf8_at(str, min_c(e_utf8_strlen(str), fMaxChars), NULL);
	fText = (str ? EStrdup(str, end == NULL ? -1 : (end - str)) : NULL);

	if(fCharWidths) delete[] fCharWidths;
	fCount = 0; fCharWidths = NULL;
	if(fText)
	{
		EFont font;
		GetFont(&font);
		fCharWidths = _CharWidths(font, fText, &fCount);
	}

	if(fCount <= 0 || fText == NULL) locationOffset = 0;
	if(fPosition > fCount) fPosition = fCount;

	fSelectStart = fSelectEnd = -1;
	fSelectTracking = -1;

	// call virtual function
	Select(fSelectStart, fSelectEnd);

	Invalidate();
}


void
ETextEditable::SetText(const EString &text)
{
	SetText(text.String());
}


const char*
ETextEditable::Text() const
{
	return fText;
}


char*
ETextEditable::DuplicateText(eint32 startPos, eint32 endPos)
{
	if(!fText || fCount <= 0 || startPos < 0) return NULL;
	if(endPos < 0 || endPos >= fCount) endPos = fCount - 1;
	if(endPos < startPos) return NULL;

	const char* start = e_utf8_at(fText, startPos, NULL);
	euint8 endLen = 0;
	const char* end = e_utf8_at(fText, endPos, &endLen);

	if(start == NULL || (end == NULL || endLen == 0)) return NULL;

	return e_strndup(start, end - start + (eint32)endLen);
}



void
ETextEditable::InsertText(const char *text, eint32 nChars, eint32 position)
{
	if(text == NULL || *text == 0 || nChars == 0) return;
	if(position < 0) position = fCount;

	eint32 length = 0;
	euint8 chLen = 0;
	const char* str = NULL;
	if(!(nChars < 0 || (str = e_utf8_at(text, nChars - 1, &chLen)) == NULL || chLen == 0)) length = (eint32)chLen + (str - text);
	else length = (eint32)strlen(text);

	if(length <= 0) return;

	if(fText == NULL)
	{
		EString astr(text, length);
		SetText(astr);
	}
	else
	{
		eint32 pos = -1;
		if(position < fCount)
		{
			euint8 len = 0;
			str = e_utf8_at(fText, position, &len);
			if(!(str == NULL || len == 0)) pos = (str - fText);
		}

		EString astr(fText);
		if(pos < 0 || pos >= (eint32)strlen(fText))
			astr.Append(text, length);
		else
			astr.Insert(text, length, pos);
		SetText(astr);
	}
}


void
ETextEditable::RemoveText(eint32 startPos, eint32 endPos)
{
	if(!fText || fCount <= 0 || startPos < 0) return;
	if(endPos < 0 || endPos >= fCount) endPos = fCount - 1;
	if(endPos < startPos) return;

	const char* start = e_utf8_at(fText, startPos, NULL);
	euint8 endLen = 0;
	const char* end = e_utf8_at(fText, endPos, &endLen);

	if(start == NULL || (end == NULL || endLen == 0)) return;

	EString astr(fText);
	astr.Remove(start - fText, end - start + (eint32)endLen);
	SetText(astr);
}


void
ETextEditable::Select(eint32 startPos, eint32 endPos)
{
	if((startPos == fSelectStart && endPos == fSelectEnd) || fText == NULL || fCount <= 0) return;
	if(endPos < 0 || endPos >= fCount) endPos = fCount - 1;
	if(endPos < startPos) return;

	if(startPos < 0)
	{
		if(fSelectStart >= 0 || fSelectEnd >= 0)
		{
			fSelectStart = fSelectEnd = -1;
			Invalidate();
		}
	}
	else if(fSelectStart != startPos || fSelectEnd != endPos)
	{
		fSelectStart = startPos;
		fSelectEnd = endPos;
		Invalidate();
	}
}


bool
ETextEditable::GetSelection(eint32 *startPos, eint32 *endPos) const
{
	if(fSelectStart < 0 || fSelectEnd < 0 || fSelectEnd < fSelectStart || fSelectEnd >= fCount) return false;
	if(!startPos && !endPos) return true;

	if(startPos) *startPos = fSelectStart;
	if(endPos) *endPos = fSelectEnd;

	return true;
}


void
ETextEditable::SetMargins(float left, float top, float right, float bottom)
{
	if(left < 0) left = 0;
	if(top < 0) top = 0;
	if(right < 0) right = 0;
	if(bottom < 0) bottom = 0;

	ERect r(left, top, right, bottom);
	if(r != fMargins)
	{
		fMargins = r;
		Invalidate();
	}
}


void
ETextEditable::GetMargins(float *left, float *top, float *right, float *bottom) const
{
	if(left) *left = fMargins.left;
	if(top) *top = fMargins.top;
	if(right) *right = fMargins.right;
	if(bottom) *bottom = fMargins.bottom;
}


void
ETextEditable::SetFont(const EFont *font, euint8 mask)
{
	EFont fontPrev;
	EFont fontCurr;
	GetFont(&fontPrev);
	EControl::SetFont(font, mask);
	GetFont(&fontCurr);

	if(fontPrev != fontCurr)
	{
		if(fCharWidths) delete[] fCharWidths;
		fCount = 0; fCharWidths = NULL;
		if(fText) fCharWidths = _CharWidths(fontCurr, fText, &fCount);
		Invalidate();
	}
}


void
ETextEditable::FrameResized(float new_width, float new_height)
{
	locationOffset = 0;
	Invalidate();
}


void
ETextEditable::Draw(ERect updateRect)
{
	if(!IsVisible()) return;

	ERect rect = Frame().OffsetToSelf(E_ORIGIN);
	rect.left += fMargins.left;
	rect.top += fMargins.top;
	rect.right -= fMargins.right;
	rect.bottom -= fMargins.bottom;

	if(!rect.IsValid()) return;

	ERegion clipping;
	GetClippingRegion(&clipping);
	if(clipping.CountRects() > 0) clipping &= (rect & updateRect);
	else clipping = (rect & updateRect);
	if(clipping.CountRects() <= 0) return;

	e_rgb_color bkColor = e_ui_color(E_DOCUMENT_BACKGROUND_COLOR);
	e_rgb_color fgColor = e_ui_color(E_DOCUMENT_TEXT_COLOR);

	if(!IsEnabled())
	{
		bkColor.disable(ViewColor());
		fgColor.disable(ViewColor());
	}

	if(!IsFocusChanging())
	{
		PushState();
		ConstrainClippingRegion(&clipping);
		SetDrawingMode(E_OP_COPY);
		SetPenSize(0);
		SetHighColor(bkColor);
		FillRect(rect & updateRect, E_SOLID_HIGH);
		PopState();
	}

	EFont font;
	e_font_height fontHeight;
	GetFont(&font);
	font.GetHeight(&fontHeight);

	if(fCount > 0 && !IsFocusChanging())
	{
		PushState();

		ConstrainClippingRegion(&clipping);

		float x = 0, y = 0;
		if(GetCharLocation(0, &x, &y, &font))
		{
			SetDrawingMode(E_OP_COPY);
			SetPenSize(0);
			SetHighColor(fgColor);
			SetLowColor(bkColor);
			_DrawString(fText, EPoint(x, y));

			if(IsEnabled() && IsSelected())
			{
				char *selectedText = DuplicateText(fSelectStart, fSelectEnd);
				if(selectedText != NULL)
				{
					x = 0; y = 0;
					if(GetCharLocation(fSelectStart, &x, &y, &font))
					{
						DrawSelectedBackground(updateRect);
						SetLowColor(e_ui_color(E_DOCUMENT_HIGHLIGHT_COLOR));
						_DrawString(selectedText, EPoint(x, y));
					}
					free(selectedText);
				}
			}
		}

		PopState();
	}

	if(IsEnabled() && IsEditable() && (IsFocus() || IsFocusChanging()))
	{
		PushState();
		ConstrainClippingRegion(&clipping);
		DrawCursor();
		PopState();
	}

	if((IsFocus() || IsFocusChanging()) && Window()->IsActivate() && IsEnabled() && (Flags() & E_NAVIGABLE))
	{
		e_rgb_color color = e_ui_color(E_NAVIGATION_BASE_COLOR);
		if(IsFocusChanging() && !IsFocus()) color = e_ui_color(E_DOCUMENT_BACKGROUND_COLOR);

		PushState();
		ConstrainClippingRegion(&clipping);
		SetDrawingMode(E_OP_COPY);
		SetPenSize(0);
		SetHighColor(color);
		StrokeRect(rect, E_SOLID_HIGH);
		PopState();
	}
}


void
ETextEditable::DrawSelectedBackground(ERect updateRect)
{
	if(fCount <= 0 || !IsEnabled()) return;
	if(fSelectStart < 0 || fSelectEnd < 0 || fSelectEnd < fSelectStart || fSelectEnd >= fCount || fCharWidths == NULL) return;

	ERect rect = Frame().OffsetToSelf(E_ORIGIN);
	rect.left += fMargins.left;
	rect.top += fMargins.top;
	rect.right -= fMargins.right;
	rect.bottom -= fMargins.bottom;

	if(!rect.IsValid()) return;

	EFont font;
	e_font_height fontHeight;
	GetFont(&font);
	font.GetHeight(&fontHeight);
	float sHeight = fontHeight.ascent + fontHeight.descent;

	ERect hlRect;
	if(!GetCharLocation(0, &(hlRect.left), NULL, &font)) return;
	hlRect.top = rect.Center().y - sHeight / 2.f - 1;
	hlRect.bottom = rect.Center().y + sHeight / 2.f + 1;

	for(eint32 i = 0; i < fSelectStart; i++)
	{
		hlRect.left += (float)ceil((double)fCharWidths[i]);
		hlRect.left += (float)ceil((double)(font.Spacing() * font.Size()));
	}

	hlRect.right = hlRect.left;

	for(eint32 i = fSelectStart; i <= fSelectEnd; i++)
	{
		hlRect.right += (float)ceil((double)fCharWidths[i]);
		if(i != fSelectEnd) hlRect.right += (float)ceil((double)(font.Spacing() * font.Size()));
	}

	hlRect &= updateRect;
	if(!hlRect.IsValid()) return;

	e_rgb_color hlColor = e_ui_color(E_DOCUMENT_HIGHLIGHT_COLOR);

	PushState();

	SetDrawingMode(E_OP_COPY);
	SetPenSize(0);
	SetHighColor(hlColor);
	FillRect(hlRect, E_SOLID_HIGH);

	PopState();
}


void
ETextEditable::DrawCursor()
{
	if(!IsEnabled() || !IsEditable() || fPosition < 0 || fPosition > fCount || (fCount > 0 && fCharWidths == NULL)) return;
	if(Window() == NULL || Window()->IsActivate() == false) return;
	if(!(IsFocus() || IsFocusChanging())) return;

	ERect rect = Frame().OffsetToSelf(E_ORIGIN);
	rect.left += fMargins.left;
	rect.top += fMargins.top;
	rect.right -= fMargins.right;
	rect.bottom -= fMargins.bottom;

	if(!rect.IsValid()) return;

	EFont font;
	e_font_height fontHeight;
	GetFont(&font);
	font.GetHeight(&fontHeight);
	float sHeight = fontHeight.ascent + fontHeight.descent;

	EPoint pt1;
	if(!GetCharLocation(fPosition, &(pt1.x), NULL, &font)) return;
	pt1.x -= 1;
	pt1.y = rect.Center().y - sHeight / 2.f;

	EPoint pt2 = pt1;
	pt2.y += sHeight;

	e_rgb_color crColor = e_ui_color(E_DOCUMENT_CURSOR_COLOR);

	if(IsFocusChanging() && !IsFocus())
	{
		if(fPosition > fSelectStart && fPosition <= fSelectEnd && fSelectEnd > fSelectStart)
		{
			crColor = e_ui_color(E_DOCUMENT_HIGHLIGHT_COLOR);
		}
		else
		{
			crColor = e_ui_color(E_DOCUMENT_BACKGROUND_COLOR);
		}
	}

	PushState();

	SetDrawingMode(E_OP_COPY);
	SetPenSize(0);
	SetHighColor(crColor);
	StrokeLine(pt1, pt2, E_SOLID_HIGH);

	PopState();
}


void
ETextEditable::MouseDown(EPoint where)
{
	ERect rect = Frame().OffsetToSelf(E_ORIGIN);
	rect.left += fMargins.left;
	rect.top += fMargins.top;
	rect.right -= fMargins.right;
	rect.bottom -= fMargins.bottom;

	if(!IsEnabled() || !rect.Contains(where) || !QueryCurrentMouse(true, E_PRIMARY_MOUSE_BUTTON)) return;
	if(!(IsEditable() || IsSelectable())) return;
	if(!IsFocus()) MakeFocus(true);

	EFont font;
	GetFont(&font);

	float x = 0;
	if(!GetCharLocation(0, &x, NULL, &font)) return;

	eint32 pos = 0;

	if(where.x > x)
	{
		for(eint32 i = 0; i <= fCount; i++)
		{
			if(i == fCount)
			{
				pos = fCount;
			}
			else
			{
				x += (float)ceil((double)fCharWidths[i]);
				if(where.x < x)
				{
					pos = i;
					break;
				}
				x += (float)ceil((double)(font.Spacing() * font.Size()));
				if(where.x < x)
				{
					pos = i + 1;
					break;
				}
			}
		}
	}

	bool redraw = IsSelected();

	if(IsFocus() && fSelectTracking < 0)
	{
		if(!(!IsSelectable() || SetPrivateEventMask(E_POINTER_EVENTS, E_LOCK_WINDOW_FOCUS) != E_OK))
		{
			fSelectStart = fSelectEnd = -1;
			fSelectTracking = pos;
		}
		else
		{
			fSelectStart = fSelectEnd = -1;
			fSelectTracking = -1;
		}

		// call virtual function
		Select(fSelectStart, fSelectEnd);
	}

	if(fPosition != pos)
	{
		fPosition = pos;
		redraw = true;
	}

	if(redraw) Invalidate();
}


void
ETextEditable::MouseUp(EPoint where)
{
	fSelectTracking = -1;

	ERect rect = Frame().OffsetToSelf(E_ORIGIN);
	rect.left += fMargins.left;
	rect.top += fMargins.top;
	rect.right -= fMargins.right;
	rect.bottom -= fMargins.bottom;

	if(rect.Contains(where)) etk_app->ObscureCursor();
}


void
ETextEditable::MouseMoved(EPoint where, euint32 code, const EMessage *a_message)
{
	ERect rect = Frame().OffsetToSelf(E_ORIGIN);
	rect.left += fMargins.left;
	rect.top += fMargins.top;
	rect.right -= fMargins.right;
	rect.bottom -= fMargins.bottom;

	if(rect.Contains(where) == false || code == E_EXITED_VIEW)
	{
		etk_app->SetCursor(E_CURSOR_SYSTEM_DEFAULT, false);
		return;
	}

	etk_app->SetCursor(E_CURSOR_I_BEAM, false);

	if(!IsEnabled() || !IsSelectable() || fSelectTracking < 0) return;

	EWindow *win = Window();
	if(!win) return;

	if(!VisibleBounds().Contains(where)) return;
	if(!(IsEditable() || IsSelectable())) return;

	EFont font;
	GetFont(&font);

	float x = 0;
	if(!GetCharLocation(0, &x, NULL, &font)) return;

	eint32 pos = 0;

	if(where.x > x)
	{
		for(eint32 i = 0; i <= fCount; i++)
		{
			if(i == fCount)
			{
				pos = fCount;
			}
			else
			{
				x += (float)ceil((double)fCharWidths[i]);
				if(where.x < x)
				{
					pos = i;
					break;
				}
				x += (float)ceil((double)(font.Spacing() * font.Size()));
				if(where.x < x)
				{
					pos = i + 1;
					break;
				}
			}
		}
	}

	bool redraw = false;

	eint32 oldStart = fSelectStart;
	eint32 oldEnd = fSelectEnd;
	if(pos == fSelectTracking)
	{
		if(IsSelected()) redraw = true;
		fSelectStart = fSelectEnd = -1;
	}
	else if(pos > fSelectTracking)
	{
		fSelectStart = fSelectTracking;
		fSelectEnd = pos - 1;
	}
	else // pos < fSelectTracking
	{
		fSelectStart = pos;
		fSelectEnd = fSelectTracking - 1;
	}

	if(oldStart != fSelectStart || oldEnd != fSelectEnd)
	{
		// call virtual function
		Select(fSelectStart, fSelectEnd);
		redraw = true;
	}

	if(fPosition != pos)
	{
		fPosition = pos;
		redraw = true;
	}

	if(redraw) Invalidate();
}


void
ETextEditable::WindowActivated(bool state)
{
	fSelectTracking = -1;
	Invalidate();
}


void
ETextEditable::KeyDown(const char *bytes, eint32 numBytes)
{
	if(!IsEnabled() || !(IsEditable() || IsSelectable()) || !IsFocus() || numBytes < 1) return;
	if(bytes[0] == E_ENTER) return;

	EWindow *win = Window();
	if(!win) return;

	EMessage *msg = win->CurrentMessage();
	if(!msg || !(msg->what == E_KEY_DOWN || msg->what == E_UNMAPPED_KEY_DOWN)) return;

	eint32 modifiers = 0;
	msg->FindInt32("modifiers", &modifiers);
	if((modifiers & E_CONTROL_KEY) || (modifiers & E_COMMAND_KEY) ||
	   (modifiers & E_MENU_KEY) || (modifiers & E_OPTION_KEY)) return;

	bool shift_only = false;
	if(IsSelectable())
	{
		modifiers &= ~(E_CAPS_LOCK | E_SCROLL_LOCK | E_NUM_LOCK | E_LEFT_SHIFT_KEY | E_RIGHT_SHIFT_KEY);
		if(modifiers == E_SHIFT_KEY) shift_only = true;
	}

	if(numBytes == 1)
	{
		switch(bytes[0])
		{
			case E_ESCAPE:
				if(IsSelectable() && (fSelectTracking > 0 || IsSelected()))
				{
					fSelectTracking = -1;
					fSelectStart = fSelectEnd = -1;

					// call virtual function
					Select(fSelectStart, fSelectEnd);

					Invalidate();
				}
				break;

			case E_UP_ARROW:
			case E_DOWN_ARROW:
				break;

			case E_LEFT_ARROW:
				{
					bool redraw = false;

					eint32 oldStart = fSelectStart;
					eint32 oldEnd = fSelectEnd;
					if(IsSelectable() && shift_only)
					{
						if(fSelectTracking < 0)
						{
							if(IsSelected() && fSelectStart == fPosition)
							{
								fSelectTracking = fSelectEnd + 1;
								if(fPosition > 0) fSelectStart = fPosition - 1;
							}
							else
							{
								fSelectTracking = fPosition;
								fSelectStart = fSelectEnd = fPosition - 1;
							}
						}
						else if(fPosition > 0)
						{
							if(fPosition <= fSelectTracking)
							{
								fSelectStart = fPosition - 1;
								fSelectEnd = fSelectTracking - 1;
							}
							else
							{
								fSelectStart = fSelectTracking;
								fSelectEnd = fPosition - 2;
							}
						}
					}
					else if(IsSelectable())
					{
						fSelectStart = fSelectEnd = -1;
					}

					if(oldStart != fSelectStart || oldEnd != fSelectEnd)
					{
						// call virtual function
						Select(fSelectStart, fSelectEnd);
						redraw = true;
					}

					if(fPosition > 0)
					{
						fPosition = fPosition - 1;
						redraw = true;
					}

					if(redraw) Invalidate();
				}
				break;

			case E_RIGHT_ARROW:
				{
					bool redraw = false;

					eint32 oldStart = fSelectStart;
					eint32 oldEnd = fSelectEnd;
					if(IsSelectable() && shift_only)
					{
						if(fSelectTracking < 0)
						{
							if(IsSelected() && fSelectEnd == fPosition - 1)
							{
								fSelectTracking = fSelectStart;
								if(fPosition < fCount) fSelectEnd = fPosition;
							}
							else
							{
								fSelectTracking = fPosition;
								fSelectStart = fSelectEnd = fPosition;
							}
						}
						else if(fPosition < fCount)
						{
							if(fPosition >= fSelectTracking)
							{
								fSelectStart = fSelectTracking;
								fSelectEnd = fPosition;
							}
							else
							{
								fSelectStart = fPosition + 1;
								fSelectEnd = fSelectTracking - 1;
							}
						}
					}
					else if(IsSelectable())
					{
						fSelectStart = fSelectEnd = -1;
					}

					if(oldStart != fSelectStart || oldEnd != fSelectEnd)
					{
						// call virtual function
						Select(fSelectStart, fSelectEnd);
						redraw = true;
					}

					if(fPosition < fCount)
					{
						fPosition = fPosition + 1;
						redraw = true;
					}

					if(redraw) Invalidate();
				}
				break;

			case E_DELETE:
				if(IsSelectable() && IsEditable() && IsSelected())
				{
					eint32 oldPos = fSelectStart;
					RemoveText(fSelectStart, fSelectEnd);
					SetPosition(oldPos);
				}
				else if(fPosition < fCount && fPosition >= 0 && IsEditable())
				{
					RemoveText(fPosition, fPosition);
				}
				break;

			case E_BACKSPACE:
				if(IsSelectable() && IsEditable() && IsSelected())
				{
					eint32 oldPos = fSelectStart;
					RemoveText(fSelectStart, fSelectEnd);
					SetPosition(oldPos);
				}
				else if(fPosition > 0 && fPosition <= fCount && IsEditable())
				{
					eint32 oldCount = fCount;
					eint32 oldPos = fPosition;
					RemoveText(fPosition - 1, fPosition - 1);
					if(fCount < oldCount && oldPos == fPosition) SetPosition(oldPos - 1);
				}
				break;

			case E_HOME:
				{
					bool redraw = false;

					eint32 oldStart = fSelectStart;
					eint32 oldEnd = fSelectEnd;
					if(IsSelectable() && shift_only)
					{
						if(fSelectTracking < 0)
						{
							if(IsSelected() && fSelectStart == fPosition)
								fSelectTracking = fSelectEnd + 1;
							else
								fSelectTracking = fPosition;
						}
						fSelectStart = 0;
						fSelectEnd = fSelectTracking - 1;
					}
					else if(IsSelectable())
					{
						fSelectStart = fSelectEnd = -1;
					}

					if(oldStart != fSelectStart || oldEnd != fSelectEnd)
					{
						// call virtual function
						Select(fSelectStart, fSelectEnd);
						redraw = true;
					}

					if(fPosition != 0)
					{
						fPosition = 0;
						redraw = true;
					}

					if(redraw) Invalidate();
				}
				break;

			case E_END:
				{
					bool redraw = false;

					eint32 oldStart = fSelectStart;
					eint32 oldEnd = fSelectEnd;
					if(IsSelectable() && shift_only)
					{
						if(fSelectTracking < 0)
						{
							if(IsSelected() && fSelectEnd == fPosition - 1)
								fSelectTracking = fSelectStart;
							else
								fSelectTracking = fPosition;
						}
						fSelectStart = fSelectTracking;
						fSelectEnd = fCount - 1;
					}
					else if(IsSelectable())
					{
						fSelectStart = fSelectEnd = -1;
					}

					if(oldStart != fSelectStart || oldEnd != fSelectEnd)
					{
						// call virtual function
						Select(fSelectStart, fSelectEnd);
						redraw = true;
					}

					if(fPosition != fCount)
					{
						fPosition = fCount;
						redraw = true;
					}

					if(redraw) Invalidate();
				}
				break;

			default:
				if(bytes[0] >= 0x20 && bytes[0] <= 0x7e && IsEditable()) // printable
				{
					if(IsSelectable() && IsSelected())
					{
						eint32 oldPos = fSelectStart;
						RemoveText(fSelectStart, fSelectEnd);
						InsertText(bytes, 1, oldPos);
						SetPosition(oldPos + 1);
					}
					else
					{
						eint32 oldCount = fCount;
						eint32 oldPos = fPosition;
						InsertText(bytes, 1, fPosition);
						if(fCount > oldCount && oldPos == fPosition) SetPosition(oldPos + 1);
					}
				}
				break;
		}
	}
	else
	{
		if(IsEditable())
		{
			eint32 len = e_utf8_strlen(bytes);
			if(len > 0)
			{
				if(IsSelectable() && IsSelected())
				{
					eint32 oldPos = fSelectStart;
					RemoveText(fSelectStart, fSelectEnd);
					InsertText(bytes, len, oldPos);
					SetPosition(oldPos + len);
				}
				else
				{
					eint32 oldCount = fCount;
					eint32 oldPos = fPosition;
					InsertText(bytes, len, fPosition);
					if(fCount > oldCount && oldPos == fPosition) SetPosition(oldPos + len);
				}
			}
		}

		// TODO: input method
	}
}


void
ETextEditable::KeyUp(const char *bytes, eint32 numBytes)
{
	if(!IsEnabled() || !IsEditable() || !IsFocus() || numBytes != 1 || bytes[0] != E_ENTER) return;

	if(Message() != NULL && fCount >= 0 && fText != NULL)
	{
		EMessage msg(*Message());
		msg.AddString("etk:texteditable-content", fText);
		Invoke(&msg);
	}
	else
	{
		Invoke();
	}
}


void
ETextEditable::MessageReceived(EMessage *msg)
{
	if(msg->what == E_MODIFIERS_CHANGED)
	{
		eint32 modifiers = 0, old_modifiers = 0;
		msg->FindInt32("modifiers", &modifiers);
		msg->FindInt32("etk:old_modifiers", &old_modifiers);
		if((old_modifiers & E_SHIFT_KEY) && !(modifiers & E_SHIFT_KEY)) fSelectTracking = -1;
	}
	EControl::MessageReceived(msg);
}


void
ETextEditable::GetPreferredSize(float *width, float *height)
{
	if(!width && !height) return;

	EFont font;
	GetFont(&font);

	if(width)
	{
		*width = fText ? (float)ceil((double)_StringWidth(font, fText)) : 0;
		*width += 6;
	}

	if(height)
	{
		e_font_height fontHeight;
		font.GetHeight(&fontHeight);
		*height = fText ? (float)ceil((double)(fontHeight.ascent + fontHeight.descent)) : 0;
		*height += 4;
	}
}


bool
ETextEditable::GetCharLocation(eint32 pos, float *x, float *y, EFont *tFont)
{
	if(!x) return false;

	ERect rect = Frame().OffsetToSelf(E_ORIGIN);
	rect.left += fMargins.left;
	rect.top += fMargins.top;
	rect.right -= fMargins.right;
	rect.bottom -= fMargins.bottom;

	rect.InsetBy(2, 2);

	if(!rect.IsValid()) return false;

	EFont font;
	e_font_height fontHeight;

	if(tFont) font = *tFont;
	else GetFont(&font);
	font.GetHeight(&fontHeight);
	float sHeight = fontHeight.ascent + fontHeight.descent;
	float strWidth = (fCount <= 0 ? 0.f : max_c(0.f, _StringWidth(font, fText)));
	float fontSpacing = (float)ceil((double)font.Spacing() * font.Size());

	if(fAlignment == E_ALIGN_RIGHT) *x = rect.right - strWidth;
	else if(fAlignment == E_ALIGN_CENTER) *x = rect.Center().x - strWidth / 2.f;
	else *x = rect.left; /* E_ALIGN_LEFT */
	if(y) *y = (rect.Center().y - sHeight/ 2.f + fontHeight.ascent + 1);

	if(strWidth <= rect.Width() || !IsEnabled() ||
	   !(IsEditable() || (IsSelectable() && IsSelected())) ||
	   fPosition < 0 || fPosition > fCount)
	{
		locationOffset = 0;
	}
	else
	{
		float xx = *x + locationOffset;

		if(fPosition > 0 && fPosition < fCount)
		{
			const char *p = e_utf8_at((const char*)fText, fPosition, NULL);
			if(p != NULL)
			{
				EString str;
				str.Append(fText, (eint32)(p - (const char*)fText));
				xx += _StringWidth(font, str.String()) + fontSpacing;
			}
		}
		else if(fPosition == fCount)
		{
			xx += strWidth + fontSpacing;
		}

		if(xx < rect.left)
			locationOffset += (rect.left - xx);
		else if(xx > rect.right)
			locationOffset += (rect.right - xx);
	}

	*x += locationOffset;

	if(pos > 0 && pos < fCount)
	{
		const char *p = e_utf8_at((const char*)fText, pos, NULL);
		if(p != NULL)
		{
			EString str;
			str.Append(fText, (eint32)(p - (const char*)fText));
			*x += _StringWidth(font, str.String()) + fontSpacing;
		}
	}
	else if(pos < 0 || pos >= fCount)
	{
		*x += strWidth + fontSpacing;
	}

	return true;
}


void
ETextEditable::MakeFocus(bool focusState)
{
	if(!focusState) fSelectTracking = -1;
	EControl::MakeFocus(focusState);
}


void
ETextEditable::SetMaxChars(eint32 max)
{
	if(max < 0) max = E_MAXINT32;
	if(fMaxChars != max)
	{
		fMaxChars = max;
		if(fCount > fMaxChars)
		{
			RemoveText(fMaxChars, -1);
			Invalidate();
		}
	}
}


eint32
ETextEditable::MaxChars() const
{
	return fMaxChars;
}


void
ETextEditable::HideTyping(euint8 flag)
{
	if((flag != 0x00 && flag < 0x20) || flag > 0x7e) flag = 0x01;

	if(fTypingHidden != flag)
	{
		fTypingHidden = flag;

		EString aStr(fText);
		ETextEditable::SetText(aStr.String());

		Invalidate();
	}
}


euint8
ETextEditable::IsTypingHidden() const
{
	return fTypingHidden;
}


float
ETextEditable::_StringWidth(const EFont &font, const char *str) const
{
	if(fTypingHidden == 0x01 || str == NULL || *str == 0) return 0;
	if(fTypingHidden == 0x00) return font.StringWidth(str);

	EString aStr;
	aStr.Append(*((char*)&fTypingHidden), e_utf8_strlen(str));
	return font.StringWidth(aStr);
}


float*
ETextEditable::_CharWidths(const EFont &font, const char *str, eint32 *count) const
{
	if(fTypingHidden == 0x01 || str == NULL || *str == 0) return NULL;
	if(fTypingHidden == 0x00) return font.CharWidths(str, count);

	EString aStr;
	aStr.Append(*((char*)&fTypingHidden), e_utf8_strlen(str));
	return font.CharWidths(aStr.String(), count);
}


void
ETextEditable::_DrawString(const char *str, EPoint location)
{
	if(fTypingHidden == 0x01 || str == NULL || *str == 0) return;

	if(fTypingHidden == 0x00)
	{
		DrawString(str, location);
	}
	else
	{
		EString aStr;
		aStr.Append(*((char*)&fTypingHidden), e_utf8_strlen(str));
		DrawString(aStr.String(), location);
	}
}

