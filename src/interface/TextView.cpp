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
 * File: TextView.cpp
 * Description: ETextView --- a multi-lines editable field
 * 
 * --------------------------------------------------------------------------*/

// TODO: speed up

#include <stdlib.h>

#include "./../support/ClassInfo.h"
#include "./../storage/File.h"
#include "./../app/Application.h"
#include "./../app/Clipboard.h"

#include "Window.h"
#include "ScrollView.h"
#include "TextView.h"


typedef struct e_text_line {
	eint32 length;
	float width;
	float height;
	float max_ascent;
	e_text_run_array *array;
} e_text_line;


#define ETK_TEXT_VIEW_LINE_SPACING	0


ETextView::ETextView(ERect frame, const char *name, ERect textRect, euint32 resizeMode, euint32 flags)
	: EView(frame, name, resizeMode, flags),
	  fEditable(true), fSelectable(true), fStylable(true), fAlignment(E_ALIGN_LEFT), fMaxBytes(E_MAXINT32),
	  fTabWidth(-8.f), fAutoindent(false), fTypingHidden(0),
	  fSelectTracking(-1), fSelectStart(-1), fSelectEnd(-1), fCurrentLine(0), fCursor(0)
{
	if((fRunArray = (e_text_run_array*)malloc(sizeof(e_text_run_array))) != NULL)
	{
		fRunArray->count = 1;
		fRunArray->runs[0].offset = 0;
		fRunArray->runs[0].font = *etk_plain_font;
		fRunArray->runs[0].color.set_to(0, 0, 0, 255);
		fRunArray->runs[0].background.set_to(0, 0, 0, 0);
		fRunArray->runs[0].underline = false;
	}

	SetTextRect(textRect);

	fTextBkColor = e_ui_color(E_DOCUMENT_BACKGROUND_COLOR);
}


ETextView::ETextView(ERect frame, const char *name, ERect textRect, const EFont *font, const e_rgb_color *color,
		     euint32 resizeMode, euint32 flags)
	: EView(frame, name, resizeMode, flags),
	  fEditable(true), fSelectable(true), fStylable(true), fAlignment(E_ALIGN_LEFT), fMaxBytes(E_MAXINT32),
	  fTabWidth(-8.f), fAutoindent(false), fTypingHidden(0),
	  fSelectTracking(-1), fSelectStart(-1), fSelectEnd(-1), fCurrentLine(0), fCursor(0)
{
	if((fRunArray = (e_text_run_array*)malloc(sizeof(e_text_run_array))) != NULL)
	{
		fRunArray->count = 1;
		fRunArray->runs[0].offset = 0;
		fRunArray->runs[0].font = (font ? *font : *etk_plain_font);
		fRunArray->runs[0].color.set_to(0, 0, 0, 255);
		if(color) fRunArray->runs[0].color = *color;
		fRunArray->runs[0].background.set_to(0, 0, 0, 0);
		fRunArray->runs[0].underline = false;
	}

	SetTextRect(textRect);

	fTextBkColor = e_ui_color(E_DOCUMENT_BACKGROUND_COLOR);
}


ETextView::~ETextView()
{
	if(fRunArray) free(fRunArray);

	e_text_line *line;
	while((line = (e_text_line*)fLines.RemoveItem((eint32)0)) != NULL)
	{
		if(line->array) free(line->array);
		free(line);
	}
}


void
ETextView::ReScanRunArray(eint32 fromLine, eint32 toLine)
{
	if(fromLine < 0 || fromLine >= fLines.CountItems()) return;
	if(toLine < 0 || toLine >= fLines.CountItems()) toLine = fLines.CountItems() - 1;

	eint32 arrayOffset = 0;
	eint32 nextLineOffset = 0;

	for(eint32 i = 0; i < fLines.CountItems(); i++)
	{
		e_text_line *line = (e_text_line*)fLines.ItemAt(i);

		eint32 curLineOffset = nextLineOffset;
		nextLineOffset += line->length + 1;

		for(; !(fRunArray == NULL || arrayOffset >= fRunArray->count - 1); arrayOffset++)
		{
			e_text_run *nextRun = &(fRunArray->runs[arrayOffset + 1]);
			if(nextRun->offset < curLineOffset) continue;
			break;
		}

		if(i < fromLine) continue;
		if(i > toLine) break;

		eint32 realCount = 1;
		if(line->array) realCount = line->array->count;
		else if((line->array = (e_text_run_array*)malloc(sizeof(e_text_run_array))) == NULL) continue;
		line->array->count = 0;

		for(eint32 k = arrayOffset; !(fRunArray == NULL || k >= fRunArray->count); k++)
		{
			e_text_run *curRun = &(fRunArray->runs[k]);
			if(curRun->offset >= nextLineOffset) break;

			line->array->runs[line->array->count] = *curRun;
			line->array->runs[line->array->count].offset = max_c(curRun->offset - curLineOffset, 0);
			if(!(line->array->count != 0 || line->array->runs[0].offset == 0)) line->array->runs[0].offset = 0;
			line->array->count += 1;

			if(line->array->count < realCount || k == fRunArray->count - 1) continue;

			void *newPtr = realloc(line->array, sizeof(e_text_run_array) + (size_t)line->array->count * sizeof(e_text_run));
			if(newPtr == NULL) break;

			line->array = (e_text_run_array*)newPtr;
			realCount = line->array->count + 1;
		}

		if(line->array->count == 0)
		{
			free(line->array);
			line->array = NULL;
		}
		else if(realCount != line->array->count)
		{
			void *newPtr = realloc(line->array,
					       sizeof(e_text_run_array) + (size_t)(line->array->count - 1) * sizeof(e_text_run));
			if(newPtr != NULL) line->array = (e_text_run_array*)newPtr;
		}
	}
}


void
ETextView::ReScanSize(eint32 fromLine, eint32 toLine)
{
	if(fromLine < 0 || fromLine >= fLines.CountItems()) return;
	if(toLine < 0 || toLine >= fLines.CountItems()) toLine = fLines.CountItems() - 1;

	eint32 nextLineOffset = 0;

	for(eint32 i = 0; i < fLines.CountItems(); i++)
	{
		e_text_line *line = (e_text_line*)fLines.ItemAt(i);

		eint32 curLineOffset = nextLineOffset;
		nextLineOffset += line->length + 1;

		if(i < fromLine) continue;
		if(i > toLine) break;

		line->width = 0;
		line->height = 0;
		line->max_ascent = 0;

		const char *str = fText.String() + curLineOffset;
		e_font_height fontHeight;

		if(line->array == NULL || line->array->count <= 0)
		{
			line->width = _StringWidth(*etk_plain_font, str, line->length);

			etk_plain_font->GetHeight(&fontHeight);
			line->height = fontHeight.ascent + fontHeight.descent;
			line->max_ascent = fontHeight.ascent;
		}
		else for(eint32 k = 0; k < line->array->count; k++)
		{
			e_text_run *curRun = &(line->array->runs[k]);
			e_text_run *nextRun = (k < line->array->count - 1 ? &(line->array->runs[k + 1]) : NULL);

			EFont curFont(curRun->font);
			line->width += _StringWidth(curFont, str + curRun->offset,
						    (nextRun == NULL ? line->length : nextRun->offset) - curRun->offset);
			if(nextRun)
			{
				EFont nextFont(nextRun->font);
				line->width += max_c(curFont.Spacing() * curFont.Size(), nextFont.Spacing() * nextFont.Size());
			}

			curFont.GetHeight(&fontHeight);
			line->height = max_c(line->height, fontHeight.ascent + fontHeight.descent);
			line->max_ascent = max_c(line->max_ascent, fontHeight.ascent);
		}
	}
}


void
ETextView::ReScanLines()
{
	e_text_line *line;
	eint32 aOffset;
	eint32 found;

	for(aOffset = 0; !(fRunArray == NULL || aOffset >= fRunArray->count - 1);)
	{
		if(memcmp((char*)&(fRunArray->runs[aOffset]) + sizeof(eint32),
			  (char*)&(fRunArray->runs[aOffset + 1]) + sizeof(eint32),
			  sizeof(e_text_run) - sizeof(eint32)) == 0)
		{
			if(aOffset < fRunArray->count - 2)
				memmove(&(fRunArray->runs[aOffset + 1]),
					&(fRunArray->runs[aOffset + 2]),
					sizeof(e_text_run) * (fRunArray->count - aOffset - 2));
			fRunArray->count -= 1;
		}
		else
		{
			aOffset++;
		}
	}

	while((line = (e_text_line*)fLines.RemoveItem((eint32)0)) != NULL)
	{
		if(line->array) free(line->array);
		free(line);
	}

	if((line = (e_text_line*)malloc(sizeof(e_text_line))) == NULL) return;
	if(fLines.AddItem(line) == false) {free(line); return;}

	aOffset = 0;
	while((found = fText.FindFirst("\n", aOffset)) >= 0)
	{
		line->length = found - aOffset;
		line->array = NULL;
		aOffset = found + 1;

		e_text_line *newLine = (e_text_line*)malloc(sizeof(e_text_line));
		if(newLine == NULL) break;
		if(fLines.AddItem(newLine) == false) {free(newLine); break;}
		line = newLine;
	}

	if(found < 0)
	{
		line->length = fText.Length() - aOffset;
		line->array = NULL;

		if(line->length < 0)
		{
			fLines.RemoveItem(line);
			free(line);
		}
	}

	ReScanRunArray(0, -1);
	ReScanSize(0, -1);

	if(fCurrentLine >= fLines.CountItems())
		fCurrentLine = max_c(fLines.CountItems() - 1, 0);
}


eint32
ETextView::CountLines() const
{
	return fLines.CountItems();
}


eint32
ETextView::CurrentLine() const
{
	return fCurrentLine;
}


void
ETextView::GoToLine(eint32 index)
{
	if(index < 0 || index >= fLines.CountItems()) return;

	if(fCurrentLine != index)
	{
		eint32 pos = OffsetAt(index, false);

		e_text_line *oldLine = (e_text_line*)fLines.ItemAt(fCurrentLine);
		e_text_line *newLine = (e_text_line*)fLines.ItemAt(index);

		if(fAlignment == E_ALIGN_RIGHT && oldLine)
		{
			eint32 oldOffset = oldLine->length - min_c(oldLine->length, fCursor);
			pos += min_c(newLine->length, max_c(newLine->length - oldOffset, 0));
		}
		else if(fAlignment == E_ALIGN_CENTER && oldLine)
		{
			eint32 oldOffset = oldLine->length / 2 - min_c(oldLine->length, fCursor);
			pos += min_c(newLine->length, max_c(newLine->length / 2 - oldOffset, 0));
		}
		else
		{
			pos += min_c(newLine->length, fCursor);
		}

		SetPosition(pos, true, false);
	}
}


eint32
ETextView::LineAt(eint32 offset, bool utf8) const
{
	if(offset < 0 || offset > (utf8 ? fText.CountChars() : fText.Length())) return -1;
	if(utf8) offset = (offset == fText.CountChars() ? fText.Length() : (fText.CharAt(offset, NULL) - fText.String()));

	if(offset == 0) return 0;
	if(offset == fText.Length()) return max_c(fLines.CountItems() - 1, 0);

	eint32 nextLineOffset = 0;
	for(eint32 i = 0; i < fLines.CountItems(); i++)
	{
		e_text_line *line = (e_text_line*)fLines.ItemAt(i);
		nextLineOffset += line->length + 1;
		if(offset < nextLineOffset) return i;
	}

	// should not reach here
	return -1;
}


eint32
ETextView::LineAt(EPoint pt, bool visible) const
{
	ERect rect = TextRect();

	if(visible)
	{
		ERegion region = VisibleBoundsRegion();
		region &= rect;
		if(region.Contains(pt) == false) return -1;
	}

	if(pt.y <= rect.top) return 0;

	float yStart, yEnd;
	yStart = yEnd = rect.top;

	for(eint32 i = 0; i < fLines.CountItems(); i++)
	{
		e_text_line *line = (e_text_line*)fLines.ItemAt(i);

		if(i > 0) yStart = yEnd + ETK_TEXT_VIEW_LINE_SPACING + UnitsPerPixel();
		yEnd = yStart + line->height;

		if(pt.y <= yEnd) return i;
	}

	return -1;
}


EPoint
ETextView::PointAt(eint32 offset, float *height, bool max_height, bool utf8) const
{
	ERect rect = TextRect();

	if(height) *height = 0;

	if(!rect.IsValid() || offset < 0 || offset > (utf8 ? fText.CountChars() : fText.Length())) return EPoint(-1.f, -1.f);
	if(utf8) offset = (offset == fText.CountChars() ? fText.Length() : (fText.CharAt(offset, NULL) - fText.String()));

	eint32 nextLineOffset = 0;

	float yStart, yEnd;
	yStart = yEnd = rect.top;

	for(eint32 i = 0; i < fLines.CountItems(); i++)
	{
		e_text_line *line = (e_text_line*)fLines.ItemAt(i);

		if(i > 0) yStart = yEnd + ETK_TEXT_VIEW_LINE_SPACING + UnitsPerPixel();
		yEnd = yStart + line->height;

		if(yStart > rect.bottom) break;

		eint32 curLineOffset = nextLineOffset;
		nextLineOffset += line->length + 1;

		if(offset < nextLineOffset)
		{
			const char *str = fText.String() + curLineOffset;
			eint32 __offset = offset - curLineOffset;
			e_font_height fontHeight;
			EPoint pt(rect.left, yStart);
			float h = 0;

			if(fAlignment == E_ALIGN_RIGHT) pt.x = rect.right - line->width - 1.f;
			else if(fAlignment == E_ALIGN_CENTER) pt.x = rect.left + (rect.Width() - line->width) / 2.f - 1.f;

			if(line->array == NULL || line->array->count <= 0)
			{
				pt.x += _StringWidth(*etk_plain_font, str, __offset);
				if(__offset != 0) pt.x += etk_plain_font->Spacing() * etk_plain_font->Size();

				etk_plain_font->GetHeight(&fontHeight);
				h = (max_height ? line->height : (fontHeight.ascent + fontHeight.descent));
				if(!max_height) pt.y += line->max_ascent - fontHeight.ascent;
			}
			else for(eint32 k = 0; k < line->array->count; k++)
			{
				e_text_run *curRun = &(line->array->runs[k]);
				e_text_run *nextRun = (k < line->array->count - 1 ? &(line->array->runs[k + 1]) : NULL);

				EFont curFont(curRun->font);

				if(__offset >= curRun->offset && (nextRun == NULL || __offset < nextRun->offset))
				{
					pt.x += _StringWidth(curFont, str + curRun->offset, __offset - curRun->offset);
					if(__offset != curRun->offset) pt.x += curFont.Spacing() * curFont.Size();

					curFont.GetHeight(&fontHeight);
					h = (max_height ? line->height : (fontHeight.ascent + fontHeight.descent));
					if(!max_height) pt.y += line->max_ascent - fontHeight.ascent;

					break;
				}

				pt.x += _StringWidth(curFont, str + curRun->offset,
						     (nextRun == NULL ? line->length : nextRun->offset) - curRun->offset);
				if(nextRun)
				{
					EFont nextFont(nextRun->font);
					pt.x += max_c(curFont.Spacing() * curFont.Size(), nextFont.Spacing() * nextFont.Size());
				}

				if(pt.x > rect.right) break;
			}

			if(!rect.Contains(pt)) break;

			if(height) *height = h;
			return pt;
		}
	}

	return EPoint(-1.f, -1.f);
}


eint32
ETextView::OffsetAt(EPoint pt, bool visible, bool utf8) const
{
	ERect rect = TextRect();

	eint32 nextLineOffset = 0;

	float yStart, yEnd;
	yStart = yEnd = rect.top;

	eint32 index = LineAt(pt, visible);

	for(eint32 i = 0; i <= index; i++)
	{
		e_text_line *line = (e_text_line*)fLines.ItemAt(i);

		if(i > 0) yStart = yEnd + ETK_TEXT_VIEW_LINE_SPACING + UnitsPerPixel();
		yEnd = yStart + line->height;

		eint32 curLineOffset = nextLineOffset;
		nextLineOffset += line->length + 1;

		if(i != index) continue;

		eint32 retVal = curLineOffset;

		float start = rect.left;
		if(fAlignment == E_ALIGN_RIGHT) start = rect.right - line->width - 1.f;
		else if(fAlignment == E_ALIGN_CENTER) start = rect.left + (rect.Width() - line->width) / 2.f - 1.f;

		if(pt.x > start)
		{
			euint8 nbytes;
			const char *str = e_utf8_at(fText.String() + curLineOffset, 0, &nbytes);
			const char *tmp = NULL;

			float xStart, xEnd;
			xStart = xEnd = start;

			if(line->array == NULL || line->array->count <= 0)
			{
				for(tmp = str; !(tmp == NULL || tmp - str > line->length); tmp = e_utf8_next(tmp, &nbytes))
				{
					if(tmp != str) xStart = xEnd + etk_plain_font->Spacing() * etk_plain_font->Size();
					xEnd = xStart + _StringWidth(*etk_plain_font, tmp, (eint32)nbytes);
					if(pt.x <= xEnd) break;
				}
			}
			else for(eint32 k = 0; k < line->array->count; k++)
			{
				e_text_run *curRun = &(line->array->runs[k]);
				e_text_run *nextRun = (k < line->array->count - 1 ? &(line->array->runs[k + 1]) : NULL);

				EFont curFont(curRun->font);

				bool found = false;
				const char *aStr = e_utf8_at(str + curRun->offset, 0, &nbytes);

				for(tmp = aStr; !(tmp == NULL || tmp - str > line->length ||
					(nextRun == NULL ? false : tmp - str >= nextRun->offset));
				    tmp = e_utf8_next(tmp, &nbytes))
				{
					if(tmp != aStr) xStart = xEnd + curFont.Spacing() * curFont.Size();
					xEnd = xStart + _StringWidth(curFont, tmp, (eint32)nbytes);
					if(pt.x <= xEnd) {found = true; break;}
				}

				if(found == true) break;
				else tmp = NULL;

				if(nextRun)
				{
					EFont nextFont(nextRun->font);
					xEnd += max_c(curFont.Spacing() * curFont.Size(), nextFont.Spacing() * nextFont.Size());
					xStart = xEnd;
				}
			}

			retVal += ((tmp == NULL || tmp - str > line->length) ? line->length : tmp - str);
		}

		return(utf8 ? e_utf8_strlen_etc(fText.String(), retVal) : retVal);
	}

	return -1;
}


eint32
ETextView::OffsetAt(eint32 a_line, bool utf8) const
{
    if(a_line < 0 || a_line >= fLines.CountItems()) return -1;

	eint32 lineOffset = 0;
    for(eint32 i = 0; i < a_line; i++)
	{
        e_text_line *lline = (e_text_line*)fLines.ItemAt(i);
        lineOffset += lline->length + 1;
	}

	return(utf8 ? e_utf8_strlen_etc(fText.String(), lineOffset) : lineOffset);
}


float
ETextView::LineWidth(eint32 lineIndex) const
{
	e_text_line *line = (e_text_line*)fLines.ItemAt(lineIndex);
	return(line ? line->width : 0.f);
}


float
ETextView::LineHeight(eint32 lineIndex) const
{
	e_text_line *line = (e_text_line*)fLines.ItemAt(lineIndex);
	return(line ? line->height : 0.f);
}


float
ETextView::TextHeight(eint32 fromLineIndex, eint32 toLineIndex) const
{
	if(fromLineIndex < 0 || fromLineIndex >= fLines.CountItems()) return 0;
	if(toLineIndex < 0 || toLineIndex >= fLines.CountItems()) toLineIndex = fLines.CountItems() - 1;

	float height = 0;
	for(eint32 i = fromLineIndex; i <= toLineIndex; i++)
	{
		e_text_line *line = (e_text_line*)fLines.ItemAt(i);
		height += line->height;
		if(i != toLineIndex) height += ETK_TEXT_VIEW_LINE_SPACING + UnitsPerPixel();
	}

	return height;
}


void
ETextView::GetTextRegion(eint32 startPos, eint32 endPos, ERegion *region, bool utf8) const
{
	if(region == NULL) return;

	region->MakeEmpty();

	ERect rect = TextRect();

	if(!rect.IsValid() || startPos < 0 || startPos >= (utf8 ? fText.CountChars() : fText.Length())) return;

	if(endPos < 0 || endPos > (utf8 ? fText.CountChars() : fText.Length())) endPos = (utf8 ? fText.CountChars() : fText.Length());
	if(utf8 && endPos > startPos)
	{
		startPos = fText.CharAt(startPos, NULL) - fText.String();
		endPos = (endPos == fText.CountChars() ? fText.Length() : (fText.CharAt(endPos, NULL) - fText.String()));
	}

	if(endPos <= startPos) return;

	eint32 nextLineOffset = 0;

	float yStart, yEnd;
	yStart = yEnd = rect.top;

	for(eint32 i = 0; i < fLines.CountItems(); i++)
	{
		e_text_line *line = (e_text_line*)fLines.ItemAt(i);

		if(i > 0) yStart = yEnd + ETK_TEXT_VIEW_LINE_SPACING + UnitsPerPixel();
		yEnd = yStart + line->height;

		eint32 curLineOffset = nextLineOffset;
		nextLineOffset += line->length + 1;

		if(startPos >= nextLineOffset) continue;
		if(endPos <= curLineOffset) break;

		const char *str = fText.String() + curLineOffset;
		eint32 __offset = max_c(startPos - curLineOffset, 0);

		ERect r;
		r.left = rect.left;
		r.top = yStart;
		r.bottom = yEnd;

		if(fAlignment == E_ALIGN_RIGHT) r.left = rect.right - line->width - 1.f;
		else if(fAlignment == E_ALIGN_CENTER) r.left = rect.left + (rect.Width() - line->width) / 2.f - 1.f;
		r.right = r.left - 1.f;

		if(line->array == NULL || line->array->count <= 0)
		{
			r.left += _StringWidth(*etk_plain_font, str, __offset);
			if(__offset != 0) r.left += etk_plain_font->Spacing() * etk_plain_font->Size();
			r.right = r.left + _StringWidth(*etk_plain_font, str + __offset,
							min_c(endPos - curLineOffset, line->length) - __offset);
		}
		else for(eint32 k = 0; k < line->array->count; k++)
		{
			e_text_run *curRun = &(line->array->runs[k]);
			e_text_run *nextRun = (k < line->array->count - 1 ? &(line->array->runs[k + 1]) : NULL);

			EFont curFont(curRun->font);

			if(__offset >= curRun->offset && (nextRun == NULL || __offset < nextRun->offset))
			{
				r.left += _StringWidth(curFont, str + curRun->offset, __offset - curRun->offset);
				if(__offset != curRun->offset) r.left += curFont.Spacing() * curFont.Size();

				eint32 curRunOffset = __offset;
				__offset = min_c(endPos - curLineOffset, line->length);
				r.right = r.left;

				for(; k < line->array->count; (curRunOffset = -1), k++)
				{
					curRun = &(line->array->runs[k]);
					nextRun = (k < line->array->count - 1 ? &(line->array->runs[k + 1]) : NULL);
					curFont = curRun->font;

					if(curRunOffset < 0) curRunOffset = curRun->offset;

					if(__offset >= curRunOffset && (nextRun == NULL || __offset < nextRun->offset))
					{
						r.right += _StringWidth(curFont, str + curRunOffset, __offset - curRunOffset);
						break;
					}

					r.right += _StringWidth(curFont, str + curRunOffset,
								(nextRun == NULL ? line->length : nextRun->offset) - curRunOffset);
					if(nextRun)
					{
						EFont nextFont(nextRun->font);
						r.right += max_c(curFont.Spacing() * curFont.Size(),
								    nextFont.Spacing() * nextFont.Size());
					}
				}

				break;
			}

			r.left += _StringWidth(curFont, str + curRun->offset,
					       (nextRun == NULL ? line->length : nextRun->offset) - curRun->offset);
			if(nextRun)
			{
				EFont nextFont(nextRun->font);
				r.left += max_c(curFont.Spacing() * curFont.Size(), nextFont.Spacing() * nextFont.Size());
			}
		}

		r &= rect;
		region->Include(r);
	}
}


void
ETextView::ScrollToOffset(eint32 offset, bool utf8)
{
	EScrollView *scrollView = e_cast_as(Parent(), EScrollView);
	if(scrollView == NULL || scrollView->Target() != this) return;

	float height = 0;
	EPoint pt = PointAt(offset, &height, true, utf8);
	ERect rect(pt, pt + EPoint(0, height));

	ERect validRect = ConvertFromParent(scrollView->TargetFrame());

	if(Frame().OffsetToSelf(E_ORIGIN).Contains(rect) == false || validRect.Contains(rect)) return;

	EPoint aPt = rect.LeftTop();
	aPt.ConstrainTo(validRect);
	pt = rect.LeftTop() - aPt;
	ScrollBy(pt.x, pt.y);

	validRect = ConvertFromParent(scrollView->TargetFrame());
	aPt = rect.RightBottom();
	if(validRect.Contains(aPt) == false)
	{
		aPt.ConstrainTo(validRect);
		pt = rect.RightBottom() - aPt;
		ScrollBy(pt.x, pt.y);
	}
}


void
ETextView::ScrollToSelection()
{
	if(!IsSelected()) return;
	ScrollToOffset(fSelectStart, false);
}


void
ETextView::FrameResized(float new_width, float new_height)
{
	// TODO
}


const char*
ETextView::Text() const
{
	return fText.String();
}


eint32
ETextView::TextLength() const
{
	return fText.Length();
}


eint32
ETextView::TextChars() const
{
	return fText.CountChars();
}


char
ETextView::ByteAt(eint32 index) const
{
	return fText.ByteAt(index);
}


const char*
ETextView::CharAt(eint32 index, euint8 *length) const
{
	return fText.CharAt(index, length);
}


void
ETextView::GetText(eint32 offset, eint32 length, char *buffer) const
{
	GetText(offset, length, buffer, length, false);
}


void
ETextView::GetText(eint32 offset, eint32 length, char *buffer, eint32 buffer_size_in_bytes, bool utf8) const
{
	if(buffer_size_in_bytes <= 0 || offset < 0 || length == 0 || fText.String() == NULL) return;

	const char *start = NULL;

	if(length < 0 || utf8 == false)
	{
		if(length < 0) length = fText.Length() - offset;
		else length = min_c(length, fText.Length() - offset);

		if(length > 0) start = fText.String() + offset;
	}
	else if((length = min_c(length, fText.CountChars() - offset)) > 0)
	{
		start = fText.CharAt(offset, NULL);
		length = (offset + length < fText.CountChars() ?
				fText.CharAt(offset + length, NULL) : (fText.String() + fText.Length())) - start;
	}

	if(start == NULL || length <= 0) return;

	memcpy(buffer, start, (size_t)min_c(length, buffer_size_in_bytes));
}


void
ETextView::SetRunArray(eint32 startPos, eint32 endPos, const e_text_run_array *runs, bool utf8)
{
	if(fStylable == false || fText.Length() <= 0)
	{
		if(runs == NULL || runs->count < 1)
		{
			if(fRunArray != NULL) {free(fRunArray); fRunArray = NULL;}
			else return; // nothing needed to change
		}
		else
		{
			if(fStylable && (runs->count != 1 || runs->runs[0].offset != 0)) return; // bad value

			if(fRunArray == NULL)
				if((fRunArray = (e_text_run_array*)malloc(sizeof(e_text_run_array))) == NULL) return;

			memcpy(&(fRunArray->runs[0]), &(runs->runs[0]), sizeof(e_text_run));
			fRunArray->runs[0].offset = 0;
			fRunArray->count = 1;
		}
	}
	else
	{
		if(startPos < 0) return;
		if(endPos < 0 || endPos > (utf8 ? fText.CountChars() : fText.Length()))
			endPos = (utf8 ? fText.CountChars() : fText.Length());
		if(utf8 && endPos > startPos)
		{
			startPos = fText.CharAt(startPos, NULL) - fText.String();
			endPos = (endPos == fText.CountChars() ? fText.Length() : (fText.CharAt(endPos, NULL) - fText.String()));
		}
		if(endPos <= startPos) return;

		const char *start = fText.String() + startPos;

		for(eint32 k = 0; !(runs == NULL || k >= runs->count); k++)
		{
			const e_text_run *run = &(runs->runs[k]);
			eint32 aOffset = run->offset;
			if(aOffset > 0 && utf8) aOffset = e_utf8_at(start, aOffset, NULL) - start;
			if(aOffset < 0 || (aOffset += startPos) >= endPos) continue;

			eint32 requestCount = (fRunArray ? fRunArray->count + 1 : 1);
			void *newPtr = realloc(fRunArray,
					       sizeof(e_text_run_array) + (size_t)(requestCount - 1) * sizeof(e_text_run));
			if(newPtr == NULL) break;

			fRunArray = (e_text_run_array*)newPtr;
			fRunArray->count = requestCount;

			eint32 i = (requestCount < 2 || fRunArray->runs[requestCount - 2].offset > aOffset) ? 0 : fRunArray->count - 1;
			for(; i < fRunArray->count; i++)
			{
				e_text_run *curRun = &(fRunArray->runs[i]);

				if(i == fRunArray->count - 1 || curRun->offset == aOffset)
				{
					*curRun = *run;
					curRun->offset = aOffset;
					break;
				}
				else if(curRun->offset > aOffset)
				{
					e_text_run *nextRun = &(fRunArray->runs[i + 1]);
					memmove(nextRun, curRun, (size_t)(fRunArray->count - i - 1) * sizeof(e_text_run));
					*curRun = *run;
					curRun->offset = aOffset;
					break;
				}
			}
		}
	}

	ReScanLines();
}


// return value must free by "free"
e_text_run_array*
ETextView::RunArray(eint32 _startPos, eint32 endPos, eint32 *length, bool utf8) const
{
	if(length) *length = 0;

	eint32 startPos = _startPos;
	if(fRunArray == NULL || fRunArray->count < 1 || startPos < 0) return NULL;
	if(endPos < 0 || endPos > (utf8 ? fText.CountChars() : fText.Length())) endPos = (utf8 ? fText.CountChars() : fText.Length());
	if(utf8 && endPos > startPos)
	{
		startPos = fText.CharAt(startPos, NULL) - fText.String();
		endPos = (endPos == fText.CountChars() ? fText.Length() : (fText.CharAt(endPos, NULL) - fText.String()));
	}

	if(fText.Length() <= 0 && startPos == 0 && fRunArray->runs[0].offset == 0)
	{
		e_text_run_array *retRuns = (e_text_run_array*)malloc(sizeof(e_text_run_array));
		if(retRuns)
		{
			memcpy(retRuns, fRunArray, sizeof(e_text_run_array));
			if(length) *length = (eint32)sizeof(e_text_run_array);
		}
		return retRuns;
	}

	if(startPos >= fText.Length() || endPos <= startPos) return NULL;

	// enough for it needs
	e_text_run_array *retRuns = (e_text_run_array*)malloc(sizeof(e_text_run_array) +
							      (size_t)(fRunArray->count - 1) * sizeof(e_text_run));
	if(retRuns == NULL) return NULL;

	retRuns->count = 0;

	for(eint32 i = 0; i < fRunArray->count; i++)
	{
		e_text_run *curRun = &(fRunArray->runs[i]);
		if(curRun->offset >= endPos) break;

		eint32 nextOffset = (i < fRunArray->count - 1 ? fRunArray->runs[i + 1].offset : fText.Length());
		if(nextOffset <= startPos) continue;

		e_text_run *destRun = &(retRuns->runs[retRuns->count++]);
		memcpy(destRun, curRun, sizeof(e_text_run));
		if(utf8) destRun->offset = e_utf8_strlen_etc(fText.String(), destRun->offset);
		destRun->offset = max_c(destRun->offset - _startPos, 0);
	}

	if(retRuns->count == 0)
	{
		free(retRuns);
		return NULL;
	}

	void *newPtr = realloc(retRuns, sizeof(e_text_run_array) + (size_t)(retRuns->count - 1) * sizeof(e_text_run));
	if(newPtr != NULL) retRuns = (e_text_run_array*)newPtr;

	if(length) *length = (eint32)(sizeof(e_text_run_array) + (size_t)(retRuns->count - 1) * sizeof(e_text_run));

	return retRuns;
}


void
ETextView::Insert(const char *text, const e_text_run_array *runs, bool utf8)
{
	if(text != NULL) Insert(fSelectStart, text, (utf8 ? e_utf8_strlen(text) : strlen(text)), runs, utf8);
}


void
ETextView::Insert(const char *text, eint32 length, const e_text_run_array *runs, bool utf8)
{
	Insert(fSelectStart, text, length, runs, utf8);
}


void
ETextView::Insert(eint32 offset, const char *text, eint32 length, const e_text_run_array *runs, bool utf8)
{
	eint32 oldStart = fSelectStart, oldEnd = fSelectEnd;

	InsertText(text, length, offset, (fStylable ? runs : NULL), utf8);

	if(oldStart != fSelectStart || oldEnd != fSelectEnd)
	{
		// call virtual function
		Select(fSelectStart, fSelectEnd, false);
	}
}


void
ETextView::InsertText(const char *start, eint32 length, eint32 offset, const e_text_run_array *runs, bool utf8)
{
	if(start == NULL || *start == 0 || length == 0 || offset > (utf8 ? fText.CountChars() : fText.Length())) return;
	if(offset < 0) offset = (utf8 ? fText.CountChars() : fText.Length());

	const char *end = NULL;
	if(length > 0) end = (utf8 ? e_utf8_at(start, length, NULL) : ((size_t)length >= strlen(start) ? NULL : start + length));
	length = (end == NULL ? strlen(start) : (end - start));

	if(utf8) offset = (offset < fText.CountChars() ? (fText.CharAt(offset, NULL) - fText.String()) : fText.Length());

	eint32 oldLength = fText.Length();
	if(oldLength + length > fMaxBytes) length = fMaxBytes - oldLength;
	if(length > 0) fText.Insert(start, length, offset);
	if(fText.Length() == oldLength) return;

	if(offset < oldLength)
	{
		for(eint32 i = 0; !(fRunArray == NULL || i >= fRunArray->count); i++)
		{
			e_text_run *curRun = &(fRunArray->runs[i]);
			eint32 nextOffset = (i < fRunArray->count - 1 ? fRunArray->runs[i + 1].offset : fText.Length());
			if(curRun->offset <= offset)
			{
				if(runs == NULL || nextOffset <= offset) continue;

				void *newPtr = realloc(fRunArray,
						       sizeof(e_text_run_array) + (size_t)fRunArray->count * sizeof(e_text_run));
				if(newPtr == NULL) continue;

				fRunArray = (e_text_run_array*)newPtr;
				fRunArray->count += 1;
				i += 1;

				if(i != fRunArray->count - 1)
					memmove(&(fRunArray->runs[i + 1]), &(fRunArray->runs[i]),
						(size_t)(fRunArray->count - i - 1) * sizeof(e_text_run));
				fRunArray->runs[i] = fRunArray->runs[i - 1];
				fRunArray->runs[i].offset = offset + length;

				continue;
			}

			curRun->offset += length;
		}
	}

	// TODO: not all lines
	if(runs)
	{
		eint32 aOffset = (utf8 ? e_utf8_strlen_etc(fText.String(), offset) : offset);
		eint32 aLen = (utf8 ? e_utf8_strlen_etc(fText.String() + offset, length) : length);
		SetRunArray(aOffset, aOffset + aLen, runs, utf8);
	}
	else
	{
		ReScanLines();
	}

	bool isSelected = (fSelectEnd > fSelectStart && fSelectStart >= 0);
	if(!isSelected || offset >= fSelectEnd) return;
	if(offset <= fSelectStart) fSelectStart += length;
	fSelectEnd += length;
}


void
ETextView::SetText(const char *text, const e_text_run_array *runs, bool utf8)
{
	Delete(0, -1, false);
	if(text != NULL) Insert(0, text, (utf8 ? e_utf8_strlen(text) : strlen(text)), runs, utf8);
	Invalidate();
}


void
ETextView::SetText(const char *text, eint32 length, const e_text_run_array *runs, bool utf8)
{
	Delete(0, -1, false);
	Insert(0, text, length, runs, utf8);
	Invalidate();
}


void
ETextView::SetText(EFile *file, eint64 fileOffset, eint32 length, const e_text_run_array *runs, bool utf8)
{
	Delete(0, -1, false);
	if(file == NULL || length <= 0) return;

	char *buffer = (char*)malloc((size_t)length + 1);
	if(buffer == NULL) return;
	bzero(buffer, (size_t)length + 1);

	ssize_t nRead = file->ReadAt(fileOffset, buffer, (size_t)length);
	if(nRead > 0) Insert(0, buffer, (eint32)nRead, runs, utf8);

	free(buffer);
	Invalidate();
}


void
ETextView::Delete(eint32 startPos, eint32 endPos, bool utf8)
{
	eint32 oldStart = fSelectStart, oldEnd = fSelectEnd;

	DeleteText(startPos, endPos, utf8);

	if(oldStart != fSelectStart || oldEnd != fSelectEnd)
	{
		// call virtual function
		Select(fSelectStart, fSelectEnd, false);
	}
}


void
ETextView::DeleteText(eint32 startPos, eint32 endPos, bool utf8)
{
	if(fText.Length() <= 0 || startPos < 0) return;

	if(endPos < 0 || endPos > (utf8 ? fText.CountChars() : fText.Length())) endPos = (utf8 ? fText.CountChars() : fText.Length());

	if(utf8 && endPos > startPos)
	{
		startPos = fText.CharAt(startPos, NULL) - fText.String();
		endPos = (endPos == fText.CountChars() ? fText.Length() : (fText.CharAt(endPos, NULL) - fText.String()));
	}

	eint32 length = endPos - startPos;
	if(length <= 0) return;

	eint32 oldLength = fText.Length();
	fText.Remove(startPos, length);
	if(fText.Length() == oldLength) return;

	if(fText.Length() <= 0)
	{
		if(fRunArray)
		{
			if(fRunArray->count > 1)
			{
				fRunArray->runs[0] = fRunArray->runs[fRunArray->count - 1];
				fRunArray->runs[0].offset = 0;
				fRunArray->count = 1;
			}

			void *newPtr = realloc(fRunArray, sizeof(e_text_run_array));
			if(newPtr) fRunArray = (e_text_run_array*)newPtr;
		}
		else if((fRunArray = (e_text_run_array*)malloc(sizeof(e_text_run_array))) != NULL)
		{
			fRunArray->count = 1;
			fRunArray->runs[0].offset = 0;
			fRunArray->runs[0].font = *etk_plain_font;
			fRunArray->runs[0].color.set_to(0, 0, 0, 255);
			fRunArray->runs[0].background.set_to(0, 0, 0, 0);
			fRunArray->runs[0].underline = false;
		}
	}
	else for(eint32 i = 0; !(fRunArray == NULL || i >= fRunArray->count); i++)
	{
		e_text_run *curRun = &(fRunArray->runs[i]);
		e_text_run *nextRun = (i < fRunArray->count - 1 ? &(fRunArray->runs[i + 1]) : NULL);

		if(curRun->offset < startPos) continue;
		if(curRun->offset >= endPos) {curRun->offset -= length; continue;}

		if(nextRun == NULL || nextRun->offset < endPos)
		{
			if(nextRun) memmove(curRun, nextRun, (size_t)(fRunArray->count - i - 1) * sizeof(e_text_run));
			if(fRunArray->count > 1)
			{
				fRunArray->count -= 1; i--;
				void *newPtr = realloc(fRunArray,
						       sizeof(e_text_run_array) + (size_t)(fRunArray->count - 1) * sizeof(e_text_run));
				if(newPtr) fRunArray = (e_text_run_array*)newPtr;
			}
		}
		else
		{
			curRun->offset = startPos;
		}
	}

	// TODO: not all lines
	ReScanLines();

	if(fText.Length() < fSelectTracking) fSelectTracking = -1;

	bool isSelected = (fSelectEnd > fSelectStart && fSelectStart >= 0);
	if(!isSelected || startPos >= fSelectEnd) return;

	if(startPos <= fSelectStart)
	{
		if(endPos >= fSelectEnd) fSelectStart = fSelectEnd = -1;
		else if(endPos <= fSelectEnd) {fSelectStart -= length; fSelectEnd -= length;}
		else {fSelectStart = startPos; fSelectEnd -= length;}
	}
	else
	{
		if(endPos >= fSelectEnd) fSelectEnd = startPos;
		else fSelectEnd -= length;
	}
}


void
ETextView::MakeEditable(bool editable)
{
	if(fEditable != editable)
	{
		fEditable = editable;
		// TODO
	}
}


bool
ETextView::IsEditable() const
{
	return fEditable;
}


void
ETextView::MakeSelectable(bool selectable)
{
	if(fSelectable != selectable)
	{
		fSelectable = selectable;
		fSelectTracking = -1;
	}
}


bool
ETextView::IsSelectable() const
{
	return fSelectable;
}


void
ETextView::SetStylable(bool stylable)
{
	if(fStylable != stylable)
	{
		if((fStylable = stylable) == false)
		{
			if(!(fRunArray == NULL || fRunArray->count <= 1))
			{
				fRunArray->count = 1;
				void *newPtr = realloc(fRunArray, sizeof(e_text_run_array));
				if(newPtr != NULL) fRunArray = (e_text_run_array*)newPtr;
			}

			if(fRunArray) fRunArray->runs[0].offset = 0;
		}

		ReScanLines();
		Invalidate();
	}
}


bool
ETextView::IsStylable() const
{
	return fStylable;
}


void
ETextView::SetTabWidth(float width)
{
	if(fTabWidth != width)
	{
		fTabWidth = width;
		ReScanSize(0, -1);
		Invalidate();
	}
}


float
ETextView::TabWidth() const
{
	return fTabWidth;
}


void
ETextView::SetAutoindent(bool flag)
{
	fAutoindent = flag;
}


bool
ETextView::DoesAutoindent() const
{
	return fAutoindent;
}


void
ETextView::HideTyping(euint8 flag)
{
	if((flag != 0x00 && flag < 0x20) || flag > 0x7e) flag = 0x01;

	if(fTypingHidden != flag)
	{
		fTypingHidden = flag;
		ReScanSize(0, -1);
		Invalidate();
	}
}


euint8
ETextView::IsTypingHidden() const
{
	return fTypingHidden;
}


void
ETextView::SetAlignment(e_alignment alignment)
{
	if(fAlignment != alignment)
	{
		fAlignment = alignment;
		Invalidate();
	}
}


e_alignment
ETextView::Alignment() const
{
	return fAlignment;
}


void
ETextView::SetMaxBytes(eint32 max)
{
	if(max < 0) max = E_MAXINT32;
	if(fMaxBytes != max)
	{
		fMaxBytes = max;
		if(fText.Length() > fMaxBytes)
		{
			Delete(fMaxBytes, -1, false);
			Invalidate();
		}
	}
}


eint32
ETextView::MaxBytes() const
{
	return fMaxBytes;
}


void
ETextView::Select(eint32 startPos, eint32 endPos, bool utf8)
{
	if((startPos == fSelectStart && endPos == fSelectEnd && utf8 == false) || fText.Length() <= 0) return;

	if(endPos < 0 || endPos > (utf8 ? fText.CountChars() : fText.Length())) endPos = (utf8 ? fText.CountChars() : fText.Length());

	if(utf8 && endPos > startPos && startPos >= 0)
	{
		startPos = fText.CharAt(startPos, NULL) - fText.String();
		endPos = (endPos == fText.CountChars() ? fText.Length() : (fText.CharAt(endPos, NULL) - fText.String()));
	}

	if(startPos < 0 || endPos <= startPos)
	{
		fSelectStart = fSelectEnd = -1;
	}
	else
	{
		fSelectStart = startPos;
		fSelectEnd = endPos;
	}
}


bool
ETextView::GetSelection(eint32 *startPos, eint32 *endPos, bool utf8) const
{
	bool isSelected = (fSelectEnd > fSelectStart && fSelectStart >= 0 && fSelectEnd <= fText.Length());

	if(isSelected)
	{
		if(startPos) *startPos = (utf8 ? e_utf8_strlen_etc(fText.String(), fSelectStart) : fSelectStart);
		if(endPos) *endPos = (utf8 ? e_utf8_strlen_etc(fText.String(), fSelectEnd) : fSelectEnd);
	}
	else
	{
		if(startPos) *startPos = 0;
		if(endPos) *endPos = 0;
	}

	return isSelected;
}


void
ETextView::Draw(ERect updateRect)
{
	ERect rect = TextRect();
	if(!rect.IsValid()) return;

	ERegion clipping;
	GetClippingRegion(&clipping);
	if(clipping.CountRects() > 0) clipping &= (rect & updateRect);
	else clipping = (rect & updateRect);
	if(clipping.CountRects() <= 0) return;

	PushState();

	SetDrawingMode(E_OP_COPY);
	SetPenSize(0);
	ConstrainClippingRegion(&clipping);

	e_rgb_color bkColor = fTextBkColor;
	if(!IsEnabled()) bkColor.disable(ViewColor());
	SetLowColor(bkColor);
	FillRect(rect & updateRect, E_SOLID_LOW);

	if(fTypingHidden == 0x01)
	{
		PopState();
		return;
	}

	ERegion selectRegion;
	GetTextRegion(fSelectStart, fSelectEnd, &selectRegion, false);
	selectRegion &= updateRect;
	if(selectRegion.CountRects() > 0)
	{
		e_rgb_color color = e_ui_color(E_DOCUMENT_HIGHLIGHT_COLOR);
		if(!IsEnabled()) color.disable(ViewColor());
		SetHighColor(color);
		FillRegion(&selectRegion, E_SOLID_HIGH);
	}

	eint32 nextLineOffset = 0;

	ERect r = rect;
	r.bottom = r.top;

	for(eint32 i = 0; i < fLines.CountItems(); i++)
	{
		e_text_line *line = (e_text_line*)fLines.ItemAt(i);

		eint32 curLineOffset = nextLineOffset;
		const char *str = fText.String() + curLineOffset;

		nextLineOffset += line->length + 1;

		if(i > 0) r.top = r.bottom + ETK_TEXT_VIEW_LINE_SPACING + UnitsPerPixel();
		r.bottom = r.top + line->height;

		if(!clipping.Intersects(r)) continue;

		EPoint penLocation(r.left, r.top + line->max_ascent + 1);
		eint32 k = 0;
		eint32 cursorPos = -1;
		if(fEditable && i == fCurrentLine) cursorPos = ((fCursor >= 0 && fCursor <= line->length) ? fCursor : line->length);
		if(line->length == 0 && cursorPos < 0) continue;

		if(fAlignment == E_ALIGN_RIGHT) penLocation.x = r.right - line->width - 1.f;
		else if(fAlignment == E_ALIGN_CENTER) penLocation.x = r.left + (r.Width() - line->width) / 2.f - 1.f;

		do {
			if(line->array == NULL || line->array->count <= 0)
			{
				e_rgb_color fgColor = e_make_rgb_color(0, 0, 0, 255);
				if(!IsEnabled()) fgColor.disable(ViewColor());

				EView::SetHighColor(fgColor);
				_DrawString(*etk_plain_font, str, penLocation, line->length);

				if(cursorPos >= 0 && IsFocus()) 
				{
					e_rgb_color color = e_ui_color(E_DOCUMENT_CURSOR_COLOR);
					if(!IsEnabled()) color.disable(ViewColor());
					EView::SetHighColor(color);
					ERect aRect;
					aRect.left = r.left + _StringWidth(*etk_plain_font, str, cursorPos);
					aRect.right = aRect.left;
					aRect.top = r.top;
					aRect.bottom = r.top + line->height;
					FillRect(aRect, E_SOLID_HIGH);
				}
			}
			else
			{
				e_text_run *curRun = &(line->array->runs[k]);
				e_text_run *nextRun = (k < line->array->count - 1 ? &(line->array->runs[k + 1]) : NULL);

				EFont curFont(curRun->font);

				e_rgb_color fgColor = curRun->color;
                bkColor = curRun->background;
				eint32 len = (nextRun == NULL ? line->length : min_c(line->length, nextRun->offset)) - curRun->offset;
				float strWidth = _StringWidth(curFont, str + curRun->offset, len);

				if(!IsEnabled())
				{
					fgColor.disable(ViewColor());
					if(bkColor.alpha > 0) {bkColor.alpha = 0xff; bkColor.disable(ViewColor());}
				}

				if(bkColor.alpha > 0)
				{
					EView::SetHighColor(bkColor);
					ERect aRect;
					aRect.left = penLocation.x;
					aRect.right = penLocation.x + strWidth;
					aRect.top = r.top;
					aRect.bottom = r.top + line->height;

					if(IsSelected() == false)
					{
						FillRect(aRect, E_SOLID_HIGH);
					}
					else
					{
						eint32 pos0, pos1;
#define SIMPLE_INTERSECTION(s0, e0, s1, e1, s2, e2) \
	do { \
		s0 = max_c(s1, s2); \
		e0 = min_c(e1, e2); \
	} while(false)
						SIMPLE_INTERSECTION(pos0, pos1,
								    curRun->offset, curRun->offset + len,
								    fSelectStart - curLineOffset, fSelectEnd - curLineOffset);
#undef SIMPLE_INTERSECTION
						do {
							if(pos0 > pos1)
							{
								FillRect(aRect, E_SOLID_HIGH);
								break;
							}

							if(pos0 > curRun->offset)
							{
								aRect.right = penLocation.x +
									      _StringWidth(curFont,
											   str + curRun->offset,
											   pos0 - curRun->offset);
								FillRect(aRect, E_SOLID_HIGH);
							}

							if(pos1 < curRun->offset + len)
							{
								aRect.left = penLocation.x + curFont.Spacing() * curFont.Size() +
									     _StringWidth(curFont,
											  str + curRun->offset,
											  pos1 - curRun->offset);
								aRect.right = aRect.left +
									      _StringWidth(curFont,
											   str + pos1,
											   curRun->offset + len - pos1);
								FillRect(aRect, E_SOLID_HIGH);
							}
						} while(false);
					}
				}

				EView::SetHighColor(fgColor);
				_DrawString(curFont, str + curRun->offset, penLocation, len);
				if(curRun->underline) StrokeLine(penLocation, penLocation + EPoint(strWidth, 0), E_SOLID_HIGH);

				if(cursorPos >= curRun->offset && (nextRun == NULL || cursorPos < nextRun->offset) && IsFocus())
				{
					e_rgb_color color = e_ui_color(E_DOCUMENT_CURSOR_COLOR);
					if(!IsEnabled()) color.disable(ViewColor());
					EView::SetHighColor(color);

					e_font_height fontHeight;
					curFont.GetHeight(&fontHeight);

					ERect aRect;
					aRect.left = penLocation.x +
						     _StringWidth(curFont, str + curRun->offset, cursorPos - curRun->offset);
					aRect.right = aRect.left;
					aRect.top = penLocation.y - fontHeight.ascent - 1;
					aRect.bottom = aRect.top + fontHeight.ascent + fontHeight.descent;
					FillRect(aRect, E_SOLID_HIGH);
				}
				penLocation.x += strWidth;

				if(nextRun)
				{
					EFont nextFont(nextRun->font);
					penLocation.x += max_c(curFont.Spacing() * curFont.Size(), nextFont.Spacing() * nextFont.Size());
				}
			}
		} while(!(line->length == 0 || line->array == NULL || ++k >= line->array->count || penLocation.x > r.right));
	}

	if(fLines.CountItems() <= 0 && fEditable && IsFocus())
	{
		EFont font(*etk_plain_font);
		e_font_height fontHeight;
		if(!(fRunArray == NULL || fRunArray->count <= 0)) font = fRunArray->runs[0].font;
		font.GetHeight(&fontHeight);

		e_rgb_color color = e_ui_color(E_DOCUMENT_CURSOR_COLOR);
		if(!IsEnabled()) color.disable(ViewColor());
		EView::SetHighColor(color);
		StrokeLine(rect.LeftTop(), rect.LeftTop() + EPoint(0, fontHeight.ascent + fontHeight.descent), E_SOLID_HIGH);
	}

	PopState();
}


void
ETextView::GetPreferredSize(float *width, float *height)
{
	if(width)
	{
		*width = 0;
		for(eint32 i = 0; i < fLines.CountItems(); i++)
		{
			e_text_line *line = (e_text_line*)fLines.ItemAt(i);
			if(line->width > *width) *width = line->width;
		}
		*width += fMargins.left + fMargins.right + 2;
	}

	if(height)
	{
		*height = TextHeight(0, -1);
		*height += fMargins.top + fMargins.bottom + 2;
	}
}


void
ETextView::SetTextRect(ERect textRect)
{
	ERect rect = Frame().OffsetToSelf(E_ORIGIN);
	if(!textRect.IsValid()) textRect = Frame().OffsetToSelf(E_ORIGIN);

	fMargins.left = textRect.left;
	fMargins.top = textRect.top;
	fMargins.right = rect.right - textRect.right;
	fMargins.bottom = rect.bottom - textRect.bottom;

	Invalidate();
}


ERect
ETextView::TextRect() const
{
	ERect rect = Frame().OffsetToSelf(E_ORIGIN);

	rect.left += fMargins.left;
	rect.top += fMargins.top;
	rect.right -= fMargins.right;
	rect.bottom -= fMargins.bottom;

	return rect;
}


void
ETextView::SetTextBackground(e_rgb_color color)
{
	if(fTextBkColor != color)
	{
		fTextBkColor = color;
		Invalidate();
	}
}


e_rgb_color
ETextView::TextBackground() const
{
	return fTextBkColor;
}


void
ETextView::FloorPosition(eint32 *pos)
{
	if(pos == NULL) return;

	if(fText.Length() <= 0 || *pos >= fText.Length())
	{
		*pos = (fText.Length() <= 0 ? 0 : fText.Length());
		return;
	}
	else if(*pos < 0)
	{
		*pos = 0;
		return;
	}

	const char *tmp = fText.String() + (*pos);
	while(*pos > 0)
	{
		if(e_utf8_is_token(tmp--)) break;
		(*pos) -= 1;
	}
}


void
ETextView::CeilPosition(eint32 *pos)
{
	if(pos == NULL) return;

	if(fText.Length() <= 0 || *pos >= fText.Length())
	{
		*pos = (fText.Length() <= 0 ? 0 : fText.Length());
		return;
	}

	if(*pos < 0) *pos = 0;
	const char *tmp = fText.String() + (*pos);
	while(*pos < fText.Length())
	{
		if(e_utf8_is_token(tmp++)) break;
		(*pos) += 1;
	}
}


void
ETextView::SetPosition(eint32 pos, bool response, bool utf8)
{
	if(IsEditable() == false) return;

	eint32 lineIndex = LineAt(pos, utf8);
	eint32 currentLine = fCurrentLine;
	eint32 currentCursor = fCursor;
	eint32 lineOffset = 0;

	do {
		if(lineIndex < 0) break;

		currentLine = lineIndex;
		e_text_line *line = (e_text_line*)fLines.ItemAt(currentLine);

		if(pos == (utf8 ? fText.CountChars() : fText.Length()))
		{
			currentCursor = (line ? line->length : 0);
			lineOffset = fText.Length() - currentCursor;
		}
		else
		{
			lineOffset = OffsetAt(currentLine, false);

			if(!utf8) CeilPosition(&pos);

			// TODO: speed up when utf8 mode
			if(utf8)
				currentCursor = fText.CharAt(pos, NULL) -
						fText.CharAt(e_utf8_strlen_etc(fText.String(), lineOffset), NULL);
			else
				currentCursor = pos - lineOffset;
		}

		if(min_c(currentCursor, fCursor) >= (line ? line->length : 0)) currentCursor = fCursor;
	} while(false);

	if(currentLine != fCurrentLine || currentCursor != fCursor)
	{
		fCurrentLine = currentLine;
		fCursor = currentCursor;

		if(response) ScrollToOffset(min_c(lineOffset + currentCursor, fText.Length()), false);
	}
}


eint32
ETextView::Position(bool utf8, eint32 *lineOffset) const
{
	eint32 pos = 0;
	if(lineOffset) *lineOffset = 0;

	for(eint32 i = 0; i <= fCurrentLine && i < fLines.CountItems(); i++)
	{
		e_text_line *line = (e_text_line*)fLines.ItemAt(i);
		if(i == fCurrentLine)
		{
			if(lineOffset) *lineOffset = (utf8 ? e_utf8_strlen_etc(fText.String(), pos) : pos);
			pos += min_c(line->length, fCursor);
			return(utf8 ? e_utf8_strlen_etc(fText.String(), pos) : pos);
		}
		pos += line->length + 1;
	}

	return 0;
}


void
ETextView::MouseDown(EPoint where)
{
	if(!IsEnabled() || !TextRect().Contains(where) || !QueryCurrentMouse(true, E_PRIMARY_MOUSE_BUTTON)) return;
	if(!(IsEditable() || IsSelectable())) return;
	if(!IsFocus()) MakeFocus(true);

	eint32 pos = OffsetAt(where, true, false);
	if(pos < 0) return;

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
		Select(fSelectStart, fSelectEnd, false);
	}

	if(Position(false, NULL) != pos)
	{
		SetPosition(pos, false, false);
		redraw = true;
	}

	if(redraw) Invalidate();
}


void
ETextView::MouseUp(EPoint where)
{
	fSelectTracking = -1;
	if(TextRect().Contains(where)) etk_app->ObscureCursor();
}


void
ETextView::MouseMoved(EPoint where, euint32 code, const EMessage *a_message)
{
	if(TextRect().Contains(where) == false || code == E_EXITED_VIEW)
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

	eint32 pos = OffsetAt(where, true, false);
	if(pos < 0) return;

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
		fSelectEnd = pos;
	}
	else // pos < fSelectTracking
	{
		fSelectStart = pos;
		fSelectEnd = fSelectTracking;
	}

	if(oldStart != fSelectStart || oldEnd != fSelectEnd)
	{
		// call virtual function
		Select(fSelectStart, fSelectEnd, false);
		redraw = true;
	}

	if(Position(false, NULL) != pos)
	{
		SetPosition(pos, false, false);
		redraw = true;
	}

	if(redraw) Invalidate();
}


void
ETextView::Cut(EClipboard *clipboard)
{
	if(clipboard == NULL || IsSelected() == false) return;

	ETextView::Copy(clipboard);

	eint32 oldPos = fSelectStart;
	Delete();
	SetPosition(oldPos, true, false);

	Invalidate();
}


void
ETextView::Copy(EClipboard *clipboard) const
{
	EMessage *clipMsg = NULL;

	if(clipboard == NULL || IsSelected() == false || clipboard->Lock() == false) return;
	if((clipMsg = clipboard->Data()) != NULL)
	{
		clipboard->Clear();

		eint32 runsBytes = 0;
		e_text_run_array *runs = RunArray(fSelectStart, fSelectEnd, &runsBytes, false);
		clipMsg->AddData("text/plain", E_MIME_TYPE,
				 fText.String() + fSelectStart, (ssize_t)(fSelectEnd - fSelectStart));
		if(runs != NULL)
		{
			clipMsg->AddData("text/runs", E_MIME_TYPE, runs, (ssize_t)runsBytes);
			free(runs);
		}

		clipboard->Commit();
	}

	clipboard->Unlock();
}


bool
ETextView::AcceptsPaste(EClipboard *clipboard)
{
	return(fEditable);
}


void
ETextView::Paste(EClipboard *clipboard)
{
	EString str;
	EMessage *clipMsg = NULL;
	e_text_run_array *runs = NULL;

	if(AcceptsPaste(clipboard) == false || clipboard == NULL || clipboard->Lock() == false) return;
	if((clipMsg = clipboard->Data()) != NULL)
	{
		const char *text = NULL;
		ssize_t len = 0;
		if(clipMsg->FindData("text/plain", E_MIME_TYPE, (const void**)&text, &len)) str.SetTo(text, (eint32)len);

		void *tmp = NULL;
		if(!(fStylable == false ||
		     clipMsg->FindData("text/runs", E_MIME_TYPE, (const void**)&tmp, &len) == false ||
		     len < (ssize_t)sizeof(e_text_run_array) ||
		     ((size_t)len - sizeof(e_text_run_array)) % sizeof(e_text_run) != 0 ||
		     (runs = (e_text_run_array*)malloc((size_t)len)) == NULL))
		{
			memcpy((void*)runs, tmp, (size_t)len);
		}
	}
	clipboard->Unlock();

	if(str.Length() > 0)
	{
		eint32 curPos = Position(false, NULL);
		if(IsSelected()) {curPos = fSelectStart; Delete();}

		eint32 oldLength = fText.Length();
		Insert(curPos, str.String(), -1, runs, false);
		SetPosition(curPos + (fText.Length() - oldLength), true, false);
	}

	if(runs) free(runs);

	Invalidate();
}


void
ETextView::KeyDown(const char *bytes, eint32 numBytes)
{
	if(!IsEnabled() || !(IsEditable() || IsSelectable()) || !IsFocus() || numBytes < 1) return;

	EWindow *win = Window();
	if(!win) return;

	EMessage *msg = win->CurrentMessage();
	if(!msg || !(msg->what == E_KEY_DOWN || msg->what == E_UNMAPPED_KEY_DOWN)) return;

	eint32 modifiers = 0;
	msg->FindInt32("modifiers", &modifiers);

	if(!(numBytes != 1 || msg->what != E_KEY_DOWN || !IsEnabled() || !IsEditable() || !(modifiers & E_CONTROL_KEY)))
	{
		if(*bytes == 'c' || *bytes == 'C') {Copy(&etk_clipboard); return;}
		else if(*bytes == 'x' || *bytes == 'X') {Cut(&etk_clipboard); return;}
		else if(*bytes == 'v' || *bytes == 'V') {Paste(&etk_clipboard); return;}
	}

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
		// TODO: region update
		e_text_line *line = (e_text_line*)fLines.ItemAt(fCurrentLine);
		switch(bytes[0])
		{
			case E_ESCAPE:
				if(IsSelectable() && (fSelectTracking > 0 || IsSelected()))
				{
					fSelectTracking = -1;
					fSelectStart = fSelectEnd = -1;

					// call virtual function
					Select(fSelectStart, fSelectEnd, false);

					Invalidate();
				}
				break;

			case E_UP_ARROW:
				if(fCurrentLine <= 0) break;
			case E_LEFT_ARROW:
				{
					if(line == NULL) break;

					eint32 fPosition = Position(false, NULL);
					eint32 oldStart = fSelectStart;
					eint32 oldEnd = fSelectEnd;
					bool redraw = false;

					eint32 nPosition;
					if(bytes[0] == E_UP_ARROW)
					{
						GoToLine(fCurrentLine - 1);
						nPosition = Position(false, NULL);
						redraw = true;
					}
					else
					{
						nPosition = fPosition - (fPosition > 0 ? 1 : 0);
						FloorPosition(&nPosition);
					}

					if(IsSelectable() && shift_only)
					{
						if(fSelectTracking < 0)
						{
							if(IsSelected() && fSelectStart == fPosition)
							{
								fSelectTracking = fSelectEnd;
								fSelectStart = nPosition;
							}
							else
							{
								fSelectTracking = fPosition;
								fSelectStart = nPosition;
								fSelectEnd = fPosition;
							}
						}
						else
						{
							if(nPosition < fSelectTracking)
							{
								fSelectStart = nPosition;
								fSelectEnd = fSelectTracking;
							}
							else
							{
								fSelectStart = fSelectTracking;
								fSelectEnd = nPosition;
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
						Select(fSelectStart, fSelectEnd, false);
						redraw = true;
					}

					if(fPosition != nPosition)
					{
						SetPosition(nPosition, true, false);
						redraw = true;
					}

					if(redraw) Invalidate();
				}
				break;

			case E_DOWN_ARROW:
				if(fCurrentLine >= fLines.CountItems()) break;
			case E_RIGHT_ARROW:
				{
					if(line == NULL) break;

					eint32 fPosition = Position(false, NULL);
					eint32 oldStart = fSelectStart;
					eint32 oldEnd = fSelectEnd;
					bool redraw = false;

					eint32 nPosition;
					if(bytes[0] == E_DOWN_ARROW)
					{
						GoToLine(fCurrentLine + 1);
						nPosition = Position(false, NULL);
						redraw = true;
					}
					else
					{
						nPosition = fPosition + (fPosition < fText.Length() ? 1 : 0);
						CeilPosition(&nPosition);
					}

					if(IsSelectable() && shift_only)
					{
						if(fSelectTracking < 0)
						{
							if(IsSelected() && fSelectEnd == fPosition)
							{
								fSelectTracking = fSelectStart;
								fSelectEnd = nPosition;
							}
							else
							{
								fSelectTracking = fPosition;
								fSelectStart = fPosition;
								fSelectEnd = nPosition;
							}
						}
						else
						{
							if(nPosition > fSelectTracking)
							{
								fSelectStart = fSelectTracking;
								fSelectEnd = nPosition;
							}
							else
							{
								fSelectStart = nPosition;
								fSelectEnd = fSelectTracking;
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
						Select(fSelectStart, fSelectEnd, false);
						redraw = true;
					}

					if(fPosition != nPosition)
					{
						SetPosition(nPosition, true, false);
						redraw = true;
					}

					if(redraw) Invalidate();
				}
				break;

			case E_DELETE:
				if(line == NULL || IsEditable() == false) break;
				if(IsSelectable() && IsSelected())
				{
					eint32 oldPos = fSelectStart;
					Delete();
					SetPosition(oldPos, true, false);
				}
				else
				{
					eint32 pos = Position(false, NULL);
					eint32 nextPos = pos + 1;
					CeilPosition(&nextPos);
					Delete(pos, nextPos);
				}
				fSelectTracking = -1;
				Invalidate();
				break;

			case E_BACKSPACE:
				if(line == NULL || IsEditable() == false) break;
				if(IsSelectable() && IsSelected())
				{
					eint32 oldPos = fSelectStart;
					Delete();
					SetPosition(oldPos, true, false);
				}
				else
				{
					eint32 pos = Position(false, NULL);
					eint32 prevPos = pos - 1;
					FloorPosition(&prevPos);
					Delete(prevPos, pos);
					SetPosition(prevPos, true, false);
				}
				fSelectTracking = -1;
				Invalidate();
				break;

			case E_HOME:
				{
					if(line == NULL) break;

					eint32 lineOffset;
					eint32 fPosition = Position(false, &lineOffset);
					eint32 oldStart = fSelectStart;
					eint32 oldEnd = fSelectEnd;
					bool redraw = false;

					if(IsSelectable() && shift_only)
					{
						if(fSelectTracking < 0)
						{
							if(IsSelected() && fSelectStart == fPosition)
								fSelectTracking = fSelectEnd;
							else
								fSelectTracking = fPosition;
						}
						fSelectStart = lineOffset;
						fSelectEnd = fSelectTracking;
					}
					else if(IsSelectable())
					{
						fSelectStart = fSelectEnd = -1;
					}

					if(oldStart != fSelectStart || oldEnd != fSelectEnd)
					{
						// call virtual function
						Select(fSelectStart, fSelectEnd, false);
						redraw = true;
					}

					if(fPosition != lineOffset)
					{
						SetPosition(lineOffset, true, false);
						redraw = true;
					}

					if(redraw) Invalidate();
				}
				break;

			case E_END:
				{
					if(line == NULL) break;

					eint32 lineOffset;
					eint32 fPosition = Position(false, &lineOffset);
					eint32 oldStart = fSelectStart;
					eint32 oldEnd = fSelectEnd;
					bool redraw = false;

					if(IsSelectable() && shift_only)
					{
						if(fSelectTracking < 0)
						{
							if(IsSelected() && fSelectEnd == fPosition)
								fSelectTracking = fSelectStart;
							else
								fSelectTracking = fPosition;
						}
						fSelectStart = fSelectTracking;
						fSelectEnd = lineOffset + line->length;
					}
					else if(IsSelectable())
					{
						fSelectStart = fSelectEnd = -1;
					}

					if(oldStart != fSelectStart || oldEnd != fSelectEnd)
					{
						// call virtual function
						Select(fSelectStart, fSelectEnd, false);
						redraw = true;
					}

					if(fPosition != lineOffset + line->length)
					{
						SetPosition(lineOffset + line->length, true, false);
						redraw = true;
					}

					if(redraw) Invalidate();
				}
				break;

			case E_PAGE_DOWN:
			case E_PAGE_UP:
				{
					EScrollView *scrollView = e_cast_as(Parent(), EScrollView);
					if(scrollView == NULL || scrollView->Target() != this) break;

					ERect validRect = ConvertFromParent(scrollView->TargetFrame());
					float yOffset = 0;
					if(bytes[0] == E_PAGE_UP)
					{
						if(validRect.top <= 0) break;
						yOffset = -min_c(validRect.top, validRect.Height());
					}
					else
					{
						ERect bounds = Frame().OffsetToSelf(E_ORIGIN);
						if(validRect.bottom >= bounds.bottom) break;
						yOffset = min_c(bounds.bottom - validRect.bottom, validRect.Height());
					}
					if(yOffset == 0) break;

					ScrollBy(0, yOffset);

					validRect = ConvertFromParent(scrollView->TargetFrame());
					ScrollToOffset(OffsetAt(validRect.LeftTop(), false), false);
				}
				break;

			default:
				{
					if(!IsEditable()) break;

					// printable || enter || tab
					if(bytes[0] == '\n' || bytes[0] == '\t' || (bytes[0] >= 0x20 && bytes[0] <= 0x7e))
					{
						EString aStr(bytes, 1);

						if(bytes[0] == '\n' && fAutoindent)
						{
							for(eint32 i = OffsetAt(fCurrentLine, false); i >= 0 && i < fText.Length(); i++)
							{
								if(!(fText[i] == ' ' || fText[i] == '\t')) break;
								aStr << fText[i];
							}
						}

						if(IsSelectable() && IsSelected())
						{
							eint32 oldPos = fSelectStart;
							Delete();

							eint32 oldLength = fText.Length();
							Insert(oldPos, aStr.String(), aStr.Length(), NULL, false);
							SetPosition(oldPos + (fText.Length() - oldLength), true, false);
						}
						else
						{
							eint32 oldPos = Position(false, NULL);

							eint32 oldLength = fText.Length();
							Insert(oldPos, aStr.String(), aStr.Length(), NULL, false);
							SetPosition(oldPos + (fText.Length() - oldLength), true, false);
						}
						fSelectTracking = -1;
						Invalidate();
					}
				}
				break;
		}
	}
	else
	{
		if(IsEditable())
		{
			if(IsSelectable() && IsSelected())
			{
				eint32 oldPos = fSelectStart;
				Delete();

				eint32 oldLength = fText.Length();
				Insert(oldPos, bytes, numBytes, NULL, false);
				SetPosition(oldPos + (fText.Length() - oldLength), true, false);
			}
			else
			{
				eint32 oldPos = Position(false, NULL);

				eint32 oldLength = fText.Length();
				Insert(oldPos, bytes, numBytes, NULL, false);
				SetPosition(oldPos + (fText.Length() - oldLength), true, false);
			}
			fSelectTracking = -1;
			Invalidate();
		}

		// TODO: input method
	}
}


void
ETextView::KeyUp(const char *bytes, eint32 numBytes)
{
}


void
ETextView::MessageReceived(EMessage *msg)
{
	if(msg->what == E_MODIFIERS_CHANGED)
	{
		eint32 modifiers = 0, old_modifiers = 0;
		msg->FindInt32("modifiers", &modifiers);
		msg->FindInt32("etk:old_modifiers", &old_modifiers);
		if((old_modifiers & E_SHIFT_KEY) && !(modifiers & E_SHIFT_KEY)) fSelectTracking = -1;
	}
	EView::MessageReceived(msg);
}


void
ETextView::WindowActivated(bool state)
{
	fSelectTracking = -1;
	// TODO
}


void
ETextView::MakeFocus(bool focusState)
{
	if(!focusState) fSelectTracking = -1;
	EView::MakeFocus(focusState);
}


float
ETextView::_StringWidth(const EFont &font, const char *str, eint32 length) const
{
	if(fTypingHidden == 0x01 || str == NULL || *str == 0 || length == 0) return 0;
	if(fTypingHidden == 0x00) return font.StringWidth(str, length, fTabWidth);

	EString aStr;
	aStr.Append(*((char*)&fTypingHidden), e_utf8_strlen_etc(str, length));
	return font.StringWidth(aStr.String(), aStr.Length(), 0);
}


void
ETextView::_DrawString(const EFont &font, const char *str, EPoint location, eint32 length)
{
	if(fTypingHidden == 0x01 || str == NULL || *str == 0 || length == 0) return;

	EView::SetFont(&font, E_FONT_ALL);

	if(fTypingHidden == 0x00)
	{
		DrawString(str, location, length, fTabWidth);
	}
	else
	{
		EString aStr;
		aStr.Append(*((char*)&fTypingHidden), e_utf8_strlen_etc(str, length));
		DrawString(aStr.String(), location, aStr.Length(), 0);
	}
}

