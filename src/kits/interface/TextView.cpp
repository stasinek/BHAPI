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
 * File: TextView.cpp
 * Description: BTextView --- a multi-lines editable field
 *
 * --------------------------------------------------------------------------*/

// TODO: speed up

#include <TextView.h>
#include <Window.h>
#include <ScrollView.h>

#include <kits/app/Application.h>
#include <kits/app/Clipboard.h>
#include <kits/app/Message.h>
#include <kits/app/AppDefs.h>
#include <kits/support/ClassInfo.h>
#include <kits/debug/Errors.h>
#include <kits/support/UTF8.h>
#include <kits/storage/File.h>



namespace bhapi {
typedef struct text_line {
     __be_int32_t length;
    float width;
    float height;
    float max_ascent;
    bhapi::text_run_array *array;
} text_line;
}

#define BHAPI_TEXT_VIEW_LINE_SPACING	0


BTextView::BTextView(BRect frame, const char *name, BRect textRect,  __be_uint32_t resizeMode,  __be_uint32_t flags)
    : BView(frame, name, resizeMode, flags),
      fEditable(true), fSelectable(true), fStylable(true), fAlignment(B_ALIGN_LEFT), fMaxBytes(B_MAXINT32),
      fTabWidth(-8.f), fAutoindent(false), fTypingHidden(0),
      fSelectTracking(-1), fSelectStart(-1), fSelectEnd(-1), fCurrentLine(0), fCursor(0)
{
    if((fRunArray = (bhapi::text_run_array*)malloc(sizeof(bhapi::text_run_array))) != NULL)
    {
        fRunArray->count = 1;
        fRunArray->runs[0].offset = 0;
        fRunArray->runs[0].font = *bhapi::plain_font;
        fRunArray->runs[0].color.set_to(0, 0, 0, 255);
        fRunArray->runs[0].background.set_to(0, 0, 0, 0);
        fRunArray->runs[0].underline = false;
    }

    SetTextRect(textRect);

    fTextBkColor = bhapi::ui_color(B_DOCUMENT_BACKGROUND_COLOR);
}


BTextView::BTextView(BRect frame, const char *name, BRect textRect, const BFont *font, const bhapi::rgb_color *color,
              __be_uint32_t resizeMode,  __be_uint32_t flags)
    : BView(frame, name, resizeMode, flags),
      fEditable(true), fSelectable(true), fStylable(true), fAlignment(B_ALIGN_LEFT), fMaxBytes(B_MAXINT32),
      fTabWidth(-8.f), fAutoindent(false), fTypingHidden(0),
      fSelectTracking(-1), fSelectStart(-1), fSelectEnd(-1), fCurrentLine(0), fCursor(0)
{
    if((fRunArray = (bhapi::text_run_array*)malloc(sizeof(bhapi::text_run_array))) != NULL)
    {
        fRunArray->count = 1;
        fRunArray->runs[0].offset = 0;
        fRunArray->runs[0].font = (font ? *font : *bhapi::plain_font);
        fRunArray->runs[0].color.set_to(0, 0, 0, 255);
        if(color) fRunArray->runs[0].color = *color;
        fRunArray->runs[0].background.set_to(0, 0, 0, 0);
        fRunArray->runs[0].underline = false;
    }

    SetTextRect(textRect);

    fTextBkColor = bhapi::ui_color(B_DOCUMENT_BACKGROUND_COLOR);
}


BTextView::~BTextView()
{
    if(fRunArray) free(fRunArray);

    bhapi::text_line *line;
    while((line = (bhapi::text_line*)fLines.RemoveItem((__be_int32_t)0)) != NULL)
    {
        if(line->array) free(line->array);
        free(line);
    }
}


void 
BTextView::ReScanRunArray(__be_int32_t fromLine,  __be_int32_t toLine)
{
    if(fromLine < 0 || fromLine >= fLines.CountItems()) return;
    if(toLine < 0 || toLine >= fLines.CountItems()) toLine = fLines.CountItems() - 1;

     __be_int32_t arrayOffset = 0;
     __be_int32_t nextLineOffset = 0;

    for(__be_int32_t i = 0; i < fLines.CountItems(); i++)
    {
        bhapi::text_line *line = (bhapi::text_line*)fLines.ItemAt(i);

         __be_int32_t curLineOffset = nextLineOffset;
        nextLineOffset += line->length + 1;

        for(; !(fRunArray == NULL || arrayOffset >= fRunArray->count - 1); arrayOffset++)
        {
            bhapi::text_run *nextRun = &(fRunArray->runs[arrayOffset + 1]);
            if(nextRun->offset < curLineOffset) continue;
            break;
        }

        if(i < fromLine) continue;
        if(i > toLine) break;

         __be_int32_t realCount = 1;
        if(line->array) realCount = line->array->count;
        else if((line->array = (bhapi::text_run_array*)malloc(sizeof(bhapi::text_run_array))) == NULL) continue;
        line->array->count = 0;

        for(__be_int32_t k = arrayOffset; !(fRunArray == NULL || k >= fRunArray->count); k++)
        {
            bhapi::text_run *curRun = &(fRunArray->runs[k]);
            if(curRun->offset >= nextLineOffset) break;

            line->array->runs[line->array->count] = *curRun;
            line->array->runs[line->array->count].offset = max_c(curRun->offset - curLineOffset, 0);
            if(!(line->array->count != 0 || line->array->runs[0].offset == 0)) line->array->runs[0].offset = 0;
            line->array->count += 1;

            if(line->array->count < realCount || k == fRunArray->count - 1) continue;

            void *newPtr = realloc(line->array, sizeof(bhapi::text_run_array) + (size_t)line->array->count * sizeof(bhapi::text_run));
            if(newPtr == NULL) break;

            line->array = (bhapi::text_run_array*)newPtr;
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
                           sizeof(bhapi::text_run_array) + (size_t)(line->array->count - 1) * sizeof(bhapi::text_run));
            if(newPtr != NULL) line->array = (bhapi::text_run_array*)newPtr;
        }
    }
}


void 
BTextView::ReScanSize(__be_int32_t fromLine,  __be_int32_t toLine)
{
    if(fromLine < 0 || fromLine >= fLines.CountItems()) return;
    if(toLine < 0 || toLine >= fLines.CountItems()) toLine = fLines.CountItems() - 1;

     __be_int32_t nextLineOffset = 0;

    for(__be_int32_t i = 0; i < fLines.CountItems(); i++)
    {
        bhapi::text_line *line = (bhapi::text_line*)fLines.ItemAt(i);

         __be_int32_t curLineOffset = nextLineOffset;
        nextLineOffset += line->length + 1;

        if(i < fromLine) continue;
        if(i > toLine) break;

        line->width = 0;
        line->height = 0;
        line->max_ascent = 0;

        const char *str = fText.String() + curLineOffset;
        bhapi::font_height fontHeight;

        if(line->array == NULL || line->array->count <= 0)
        {
            line->width = _StringWidth(*bhapi::plain_font, str, line->length);

            bhapi::plain_font->GetHeight(&fontHeight);
            line->height = fontHeight.ascent + fontHeight.descent;
            line->max_ascent = fontHeight.ascent;
        }
        else for(__be_int32_t k = 0; k < line->array->count; k++)
        {
            bhapi::text_run *curRun = &(line->array->runs[k]);
            bhapi::text_run *nextRun = (k < line->array->count - 1 ? &(line->array->runs[k + 1]) : NULL);

            BFont curFont(curRun->font);
            line->width += _StringWidth(curFont, str + curRun->offset,
                            (nextRun == NULL ? line->length : nextRun->offset) - curRun->offset);
            if(nextRun)
            {
                BFont nextFont(nextRun->font);
                line->width += max_c(curFont.Spacing() * curFont.Size(), nextFont.Spacing() * nextFont.Size());
            }

            curFont.GetHeight(&fontHeight);
            line->height = max_c(line->height, fontHeight.ascent + fontHeight.descent);
            line->max_ascent = max_c(line->max_ascent, fontHeight.ascent);
        }
    }
}


void 
BTextView::ReScanLines()
{
    bhapi::text_line *line;
     __be_int32_t aOffset;
     __be_int32_t found;

    for(aOffset = 0; !(fRunArray == NULL || aOffset >= fRunArray->count - 1);)
    {
        if(memcmp((char*)&(fRunArray->runs[aOffset]) + sizeof(__be_int32_t),
              (char*)&(fRunArray->runs[aOffset + 1]) + sizeof(__be_int32_t),
              sizeof(bhapi::text_run) - sizeof(__be_int32_t)) == 0)
        {
            if(aOffset < fRunArray->count - 2)
                memmove(&(fRunArray->runs[aOffset + 1]),
                    &(fRunArray->runs[aOffset + 2]),
                    sizeof(bhapi::text_run) * (fRunArray->count - aOffset - 2));
            fRunArray->count -= 1;
        }
        else
        {
            aOffset++;
        }
    }

    while((line = (bhapi::text_line*)fLines.RemoveItem((__be_int32_t)0)) != NULL)
    {
        if(line->array) free(line->array);
        free(line);
    }

    if((line = (bhapi::text_line*)malloc(sizeof(bhapi::text_line))) == NULL) return;
    if(fLines.AddItem(line) == false) {free(line); return;}

    aOffset = 0;
    while((found = fText.FindFirst("\n", aOffset)) >= 0)
    {
        line->length = found - aOffset;
        line->array = NULL;
        aOffset = found + 1;

        bhapi::text_line *newLine = (bhapi::text_line*)malloc(sizeof(bhapi::text_line));
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


__be_int32_t
BTextView::CountLines() const
{
    return fLines.CountItems();
}


__be_int32_t
BTextView::CurrentLine() const
{
    return fCurrentLine;
}


void 
BTextView::GoToLine(__be_int32_t index)
{
    if(index < 0 || index >= fLines.CountItems()) return;

    if(fCurrentLine != index)
    {
         __be_int32_t pos = OffsetAt(index, false);

        bhapi::text_line *oldLine = (bhapi::text_line*)fLines.ItemAt(fCurrentLine);
        bhapi::text_line *newLine = (bhapi::text_line*)fLines.ItemAt(index);

        if(fAlignment == B_ALIGN_RIGHT && oldLine)
        {
             __be_int32_t oldOffset = oldLine->length - min_c(oldLine->length, fCursor);
            pos += min_c(newLine->length, max_c(newLine->length - oldOffset, 0));
        }
        else if(fAlignment == B_ALIGN_CENTER && oldLine)
        {
             __be_int32_t oldOffset = oldLine->length / 2 - min_c(oldLine->length, fCursor);
            pos += min_c(newLine->length, max_c(newLine->length / 2 - oldOffset, 0));
        }
        else
        {
            pos += min_c(newLine->length, fCursor);
        }

        SetPosition(pos, true, false);
    }
}


__be_int32_t
BTextView::LineAt(__be_int32_t offset, bool utf8) const
{
    if(offset < 0 || offset > (utf8 ? fText.CountChars() : fText.Length())) return -1;
    if(utf8) offset = (offset == fText.CountChars() ? fText.Length() : (fText.CharAt(offset, NULL) - fText.String()));

    if(offset == 0) return 0;
    if(offset == fText.Length()) return max_c(fLines.CountItems() - 1, 0);

     __be_int32_t nextLineOffset = 0;
    for(__be_int32_t i = 0; i < fLines.CountItems(); i++)
    {
        bhapi::text_line *line = (bhapi::text_line*)fLines.ItemAt(i);
        nextLineOffset += line->length + 1;
        if(offset < nextLineOffset) return i;
    }

    // should not reach here
    return -1;
}


__be_int32_t
BTextView::LineAt(BPoint pt, bool visible) const
{
    BRect rect = TextRect();

    if(visible)
    {
        BRegion region = VisibleBoundsRegion();
        region &= rect;
        if(region.Contains(pt) == false) return -1;
    }

    if(pt.y <= rect.top) return 0;

    float yStart, yEnd;
    yStart = yEnd = rect.top;

    for(__be_int32_t i = 0; i < fLines.CountItems(); i++)
    {
        bhapi::text_line *line = (bhapi::text_line*)fLines.ItemAt(i);

        if(i > 0) yStart = yEnd + BHAPI_TEXT_VIEW_LINE_SPACING + UnitsPerPixel();
        yEnd = yStart + line->height;

        if(pt.y <= yEnd) return i;
    }

    return -1;
}


BPoint
BTextView::PointAt(__be_int32_t offset, float *height, bool max_height, bool utf8) const
{
    BRect rect = TextRect();

    if(height) *height = 0;

    if(!rect.IsValid() || offset < 0 || offset > (utf8 ? fText.CountChars() : fText.Length())) return BPoint(-1.f, -1.f);
    if(utf8) offset = (offset == fText.CountChars() ? fText.Length() : (fText.CharAt(offset, NULL) - fText.String()));

     __be_int32_t nextLineOffset = 0;

    float yStart, yEnd;
    yStart = yEnd = rect.top;

    for(__be_int32_t i = 0; i < fLines.CountItems(); i++)
    {
        bhapi::text_line *line = (bhapi::text_line*)fLines.ItemAt(i);

        if(i > 0) yStart = yEnd + BHAPI_TEXT_VIEW_LINE_SPACING + UnitsPerPixel();
        yEnd = yStart + line->height;

        if(yStart > rect.bottom) break;

         __be_int32_t curLineOffset = nextLineOffset;
        nextLineOffset += line->length + 1;

        if(offset < nextLineOffset)
        {
            const char *str = fText.String() + curLineOffset;
             __be_int32_t __offset = offset - curLineOffset;
            bhapi::font_height fontHeight;
            BPoint pt(rect.left, yStart);
            float h = 0;

            if(fAlignment == B_ALIGN_RIGHT) pt.x = rect.right - line->width - 1.f;
            else if(fAlignment == B_ALIGN_CENTER) pt.x = rect.left + (rect.Width() - line->width) / 2.f - 1.f;

            if(line->array == NULL || line->array->count <= 0)
            {
                pt.x += _StringWidth(*bhapi::plain_font, str, __offset);
                if(__offset != 0) pt.x += bhapi::plain_font->Spacing() * bhapi::plain_font->Size();

                bhapi::plain_font->GetHeight(&fontHeight);
                h = (max_height ? line->height : (fontHeight.ascent + fontHeight.descent));
                if(!max_height) pt.y += line->max_ascent - fontHeight.ascent;
            }
            else for(__be_int32_t k = 0; k < line->array->count; k++)
            {
                bhapi::text_run *curRun = &(line->array->runs[k]);
                bhapi::text_run *nextRun = (k < line->array->count - 1 ? &(line->array->runs[k + 1]) : NULL);

                BFont curFont(curRun->font);

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
                    BFont nextFont(nextRun->font);
                    pt.x += max_c(curFont.Spacing() * curFont.Size(), nextFont.Spacing() * nextFont.Size());
                }

                if(pt.x > rect.right) break;
            }

            if(!rect.Contains(pt)) break;

            if(height) *height = h;
            return pt;
        }
    }

    return BPoint(-1.f, -1.f);
}


__be_int32_t
BTextView::OffsetAt(BPoint pt, bool visible, bool utf8) const
{
    BRect rect = TextRect();

     __be_int32_t nextLineOffset = 0;

    float yStart, yEnd;
    yStart = yEnd = rect.top;

     __be_int32_t index = LineAt(pt, visible);

    for(__be_int32_t i = 0; i <= index; i++)
    {
        bhapi::text_line *line = (bhapi::text_line*)fLines.ItemAt(i);

        if(i > 0) yStart = yEnd + BHAPI_TEXT_VIEW_LINE_SPACING + UnitsPerPixel();
        yEnd = yStart + line->height;

         __be_int32_t curLineOffset = nextLineOffset;
        nextLineOffset += line->length + 1;

        if(i != index) continue;

         __be_int32_t retVal = curLineOffset;

        float start = rect.left;
        if(fAlignment == B_ALIGN_RIGHT) start = rect.right - line->width - 1.f;
        else if(fAlignment == B_ALIGN_CENTER) start = rect.left + (rect.Width() - line->width) / 2.f - 1.f;

        if(pt.x > start)
        {
             __be_uint8 nbytes;
            const char *str = bhapi::utf8_at(fText.String() + curLineOffset, 0, &nbytes);
            const char *tmp = NULL;

            float xStart, xEnd;
            xStart = xEnd = start;

            if(line->array == NULL || line->array->count <= 0)
            {
                for(tmp = str; !(tmp == NULL || tmp - str > line->length); tmp = bhapi::utf8_next(tmp, &nbytes))
                {
                    if(tmp != str) xStart = xEnd + bhapi::plain_font->Spacing() * bhapi::plain_font->Size();
                    xEnd = xStart + _StringWidth(*bhapi::plain_font, tmp, (__be_int32_t)nbytes);
                    if(pt.x <= xEnd) break;
                }
            }
            else for(__be_int32_t k = 0; k < line->array->count; k++)
            {
                bhapi::text_run *curRun = &(line->array->runs[k]);
                bhapi::text_run *nextRun = (k < line->array->count - 1 ? &(line->array->runs[k + 1]) : NULL);

                BFont curFont(curRun->font);

                bool found = false;
                const char *aStr = bhapi::utf8_at(str + curRun->offset, 0, &nbytes);

                for(tmp = aStr; !(tmp == NULL || tmp - str > line->length ||
                    (nextRun == NULL ? false : tmp - str >= nextRun->offset));
                    tmp = bhapi::utf8_next(tmp, &nbytes))
                {
                    if(tmp != aStr) xStart = xEnd + curFont.Spacing() * curFont.Size();
                    xEnd = xStart + _StringWidth(curFont, tmp, (__be_int32_t)nbytes);
                    if(pt.x <= xEnd) {found = true; break;}
                }

                if(found == true) break;
                else tmp = NULL;

                if(nextRun)
                {
                    BFont nextFont(nextRun->font);
                    xEnd += max_c(curFont.Spacing() * curFont.Size(), nextFont.Spacing() * nextFont.Size());
                    xStart = xEnd;
                }
            }

            retVal += ((tmp == NULL || tmp - str > line->length) ? line->length : tmp - str);
        }

        return(utf8 ? bhapi::utf8_strlen_etc(fText.String(), retVal) : retVal);
    }

    return -1;
}


__be_int32_t
BTextView::OffsetAt(__be_int32_t a_line, bool utf8) const
{
    if(a_line < 0 || a_line >= fLines.CountItems()) return -1;

     __be_int32_t lineOffset = 0;
    for(__be_int32_t i = 0; i < a_line; i++)
    {
        bhapi::text_line *lline = (bhapi::text_line*)fLines.ItemAt(i);
        lineOffset += lline->length + 1;
    }

    return(utf8 ? bhapi::utf8_strlen_etc(fText.String(), lineOffset) : lineOffset);
}


float
BTextView::LineWidth(__be_int32_t lineIndex) const
{
    bhapi::text_line *line = (bhapi::text_line*)fLines.ItemAt(lineIndex);
    return(line ? line->width : 0.f);
}


float
BTextView::LineHeight(__be_int32_t lineIndex) const
{
    bhapi::text_line *line = (bhapi::text_line*)fLines.ItemAt(lineIndex);
    return(line ? line->height : 0.f);
}


float
BTextView::TextHeight(__be_int32_t fromLineIndex,  __be_int32_t toLineIndex) const
{
    if(fromLineIndex < 0 || fromLineIndex >= fLines.CountItems()) return 0;
    if(toLineIndex < 0 || toLineIndex >= fLines.CountItems()) toLineIndex = fLines.CountItems() - 1;

    float height = 0;
    for(__be_int32_t i = fromLineIndex; i <= toLineIndex; i++)
    {
        bhapi::text_line *line = (bhapi::text_line*)fLines.ItemAt(i);
        height += line->height;
        if(i != toLineIndex) height += BHAPI_TEXT_VIEW_LINE_SPACING + UnitsPerPixel();
    }

    return height;
}


void 
BTextView::GetTextRegion(__be_int32_t startPos,  __be_int32_t endPos, BRegion *region, bool utf8) const
{
    if(region == NULL) return;

    region->MakeEmpty();

    BRect rect = TextRect();

    if(!rect.IsValid() || startPos < 0 || startPos >= (utf8 ? fText.CountChars() : fText.Length())) return;

    if(endPos < 0 || endPos > (utf8 ? fText.CountChars() : fText.Length())) endPos = (utf8 ? fText.CountChars() : fText.Length());
    if(utf8 && endPos > startPos)
    {
        startPos = fText.CharAt(startPos, NULL) - fText.String();
        endPos = (endPos == fText.CountChars() ? fText.Length() : (fText.CharAt(endPos, NULL) - fText.String()));
    }

    if(endPos <= startPos) return;

     __be_int32_t nextLineOffset = 0;

    float yStart, yEnd;
    yStart = yEnd = rect.top;

    for(__be_int32_t i = 0; i < fLines.CountItems(); i++)
    {
        bhapi::text_line *line = (bhapi::text_line*)fLines.ItemAt(i);

        if(i > 0) yStart = yEnd + BHAPI_TEXT_VIEW_LINE_SPACING + UnitsPerPixel();
        yEnd = yStart + line->height;

         __be_int32_t curLineOffset = nextLineOffset;
        nextLineOffset += line->length + 1;

        if(startPos >= nextLineOffset) continue;
        if(endPos <= curLineOffset) break;

        const char *str = fText.String() + curLineOffset;
         __be_int32_t __offset = max_c(startPos - curLineOffset, 0);

        BRect r;
        r.left = rect.left;
        r.top = yStart;
        r.bottom = yEnd;

        if(fAlignment == B_ALIGN_RIGHT) r.left = rect.right - line->width - 1.f;
        else if(fAlignment == B_ALIGN_CENTER) r.left = rect.left + (rect.Width() - line->width) / 2.f - 1.f;
        r.right = r.left - 1.f;

        if(line->array == NULL || line->array->count <= 0)
        {
            r.left += _StringWidth(*bhapi::plain_font, str, __offset);
            if(__offset != 0) r.left += bhapi::plain_font->Spacing() * bhapi::plain_font->Size();
            r.right = r.left + _StringWidth(*bhapi::plain_font, str + __offset,
                            min_c(endPos - curLineOffset, line->length) - __offset);
        }
        else for(__be_int32_t k = 0; k < line->array->count; k++)
        {
            bhapi::text_run *curRun = &(line->array->runs[k]);
            bhapi::text_run *nextRun = (k < line->array->count - 1 ? &(line->array->runs[k + 1]) : NULL);

            BFont curFont(curRun->font);

            if(__offset >= curRun->offset && (nextRun == NULL || __offset < nextRun->offset))
            {
                r.left += _StringWidth(curFont, str + curRun->offset, __offset - curRun->offset);
                if(__offset != curRun->offset) r.left += curFont.Spacing() * curFont.Size();

                 __be_int32_t curRunOffset = __offset;
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
                        BFont nextFont(nextRun->font);
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
                BFont nextFont(nextRun->font);
                r.left += max_c(curFont.Spacing() * curFont.Size(), nextFont.Spacing() * nextFont.Size());
            }
        }

        r &= rect;
        region->Include(r);
    }
}


void 
BTextView::ScrollToOffset(__be_int32_t offset, bool utf8)
{
    BScrollView *scrollView = cast_as(Parent(), BScrollView);
    if(scrollView == NULL || scrollView->Target() != this) return;

    float height = 0;
    BPoint pt = PointAt(offset, &height, true, utf8);
    BRect rect(pt, pt + BPoint(0, height));

    BRect validRect = ConvertFromParent(scrollView->TargetFrame());

    if(Frame().OffsetToSelf(B_ORIGIN).Contains(rect) == false || validRect.Contains(rect)) return;

    BPoint aPt = rect.LeftTop();
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
BTextView::ScrollToSelection()
{
    if(!IsSelected()) return;
    ScrollToOffset(fSelectStart, false);
}


void 
BTextView::FrameResized(float new_width, float new_height)
{
    // TODO
}


const char*
BTextView::Text() const
{
    return fText.String();
}


__be_int32_t
BTextView::TextLength() const
{
    return fText.Length();
}


__be_int32_t
BTextView::TextChars() const
{
    return fText.CountChars();
}


char
BTextView::ByteAt(__be_int32_t index) const
{
    return fText.ByteAt(index);
}


const char*
BTextView::CharAt(__be_int32_t index,  __be_uint8 *length) const
{
    return fText.CharAt(index, length);
}


void 
BTextView::GetText(__be_int32_t offset,  __be_int32_t length, char *buffer) const
{
    GetText(offset, length, buffer, length, false);
}


void 
BTextView::GetText(__be_int32_t offset,  __be_int32_t length, char *buffer,  __be_int32_t buffer_size_in_bytes, bool utf8) const
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
BTextView::SetRunArray(__be_int32_t startPos,  __be_int32_t endPos, const bhapi::text_run_array *runs, bool utf8)
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
                if((fRunArray = (bhapi::text_run_array*)malloc(sizeof(bhapi::text_run_array))) == NULL) return;

            memcpy(&(fRunArray->runs[0]), &(runs->runs[0]), sizeof(bhapi::text_run));
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

        for(__be_int32_t k = 0; !(runs == NULL || k >= runs->count); k++)
        {
            const bhapi::text_run *run = &(runs->runs[k]);
             __be_int32_t aOffset = run->offset;
            if(aOffset > 0 && utf8) aOffset = bhapi::utf8_at(start, aOffset, NULL) - start;
            if(aOffset < 0 || (aOffset += startPos) >= endPos) continue;

             __be_int32_t requestCount = (fRunArray ? fRunArray->count + 1 : 1);
            void *newPtr = realloc(fRunArray,
                           sizeof(bhapi::text_run_array) + (size_t)(requestCount - 1) * sizeof(bhapi::text_run));
            if(newPtr == NULL) break;

            fRunArray = (bhapi::text_run_array*)newPtr;
            fRunArray->count = requestCount;

             __be_int32_t i = (requestCount < 2 || fRunArray->runs[requestCount - 2].offset > aOffset) ? 0 : fRunArray->count - 1;
            for(; i < fRunArray->count; i++)
            {
                bhapi::text_run *curRun = &(fRunArray->runs[i]);

                if(i == fRunArray->count - 1 || curRun->offset == aOffset)
                {
                    *curRun = *run;
                    curRun->offset = aOffset;
                    break;
                }
                else if(curRun->offset > aOffset)
                {
                    bhapi::text_run *nextRun = &(fRunArray->runs[i + 1]);
                    memmove(nextRun, curRun, (size_t)(fRunArray->count - i - 1) * sizeof(bhapi::text_run));
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
bhapi::text_run_array*
BTextView::RunArray(__be_int32_t _startPos,  __be_int32_t endPos,  __be_int32_t *length, bool utf8) const
{
    if(length) *length = 0;

     __be_int32_t startPos = _startPos;
    if(fRunArray == NULL || fRunArray->count < 1 || startPos < 0) return NULL;
    if(endPos < 0 || endPos > (utf8 ? fText.CountChars() : fText.Length())) endPos = (utf8 ? fText.CountChars() : fText.Length());
    if(utf8 && endPos > startPos)
    {
        startPos = fText.CharAt(startPos, NULL) - fText.String();
        endPos = (endPos == fText.CountChars() ? fText.Length() : (fText.CharAt(endPos, NULL) - fText.String()));
    }

    if(fText.Length() <= 0 && startPos == 0 && fRunArray->runs[0].offset == 0)
    {
        bhapi::text_run_array *retRuns = (bhapi::text_run_array*)malloc(sizeof(bhapi::text_run_array));
        if(retRuns)
        {
            memcpy(retRuns, fRunArray, sizeof(bhapi::text_run_array));
            if(length) *length = (__be_int32_t)sizeof(bhapi::text_run_array);
        }
        return retRuns;
    }

    if(startPos >= fText.Length() || endPos <= startPos) return NULL;

    // enough for it needs
    bhapi::text_run_array *retRuns = (bhapi::text_run_array*)malloc(sizeof(bhapi::text_run_array) +
                                  (size_t)(fRunArray->count - 1) * sizeof(bhapi::text_run));
    if(retRuns == NULL) return NULL;

    retRuns->count = 0;

    for(__be_int32_t i = 0; i < fRunArray->count; i++)
    {
        bhapi::text_run *curRun = &(fRunArray->runs[i]);
        if(curRun->offset >= endPos) break;

         __be_int32_t nextOffset = (i < fRunArray->count - 1 ? fRunArray->runs[i + 1].offset : fText.Length());
        if(nextOffset <= startPos) continue;

        bhapi::text_run *destRun = &(retRuns->runs[retRuns->count++]);
        memcpy(destRun, curRun, sizeof(bhapi::text_run));
        if(utf8) destRun->offset = bhapi::utf8_strlen_etc(fText.String(), destRun->offset);
        destRun->offset = max_c(destRun->offset - _startPos, 0);
    }

    if(retRuns->count == 0)
    {
        free(retRuns);
        return NULL;
    }

    void *newPtr = realloc(retRuns, sizeof(bhapi::text_run_array) + (size_t)(retRuns->count - 1) * sizeof(bhapi::text_run));
    if(newPtr != NULL) retRuns = (bhapi::text_run_array*)newPtr;

    if(length) *length = (__be_int32_t)(sizeof(bhapi::text_run_array) + (size_t)(retRuns->count - 1) * sizeof(bhapi::text_run));

    return retRuns;
}


void 
BTextView::Insert(const char *text, const bhapi::text_run_array *runs, bool utf8)
{
    if(text != NULL) Insert(fSelectStart, text, (utf8 ? bhapi::utf8_strlen(text) : strlen(text)), runs, utf8);
}


void 
BTextView::Insert(const char *text,  __be_int32_t length, const bhapi::text_run_array *runs, bool utf8)
{
    Insert(fSelectStart, text, length, runs, utf8);
}


void 
BTextView::Insert(__be_int32_t offset, const char *text,  __be_int32_t length, const bhapi::text_run_array *runs, bool utf8)
{
     __be_int32_t oldStart = fSelectStart, oldEnd = fSelectEnd;

    InsertText(text, length, offset, (fStylable ? runs : NULL), utf8);

    if(oldStart != fSelectStart || oldEnd != fSelectEnd)
    {
        // call virtual function
        Select(fSelectStart, fSelectEnd, false);
    }
}


void 
BTextView::InsertText(const char *start,  __be_int32_t length,  __be_int32_t offset, const bhapi::text_run_array *runs, bool utf8)
{
    if(start == NULL || *start == 0 || length == 0 || offset > (utf8 ? fText.CountChars() : fText.Length())) return;
    if(offset < 0) offset = (utf8 ? fText.CountChars() : fText.Length());

    const char *end = NULL;
    if(length > 0) end = (utf8 ? bhapi::utf8_at(start, length, NULL) : ((size_t)length >= strlen(start) ? NULL : start + length));
    length = (end == NULL ? strlen(start) : (end - start));

    if(utf8) offset = (offset < fText.CountChars() ? (fText.CharAt(offset, NULL) - fText.String()) : fText.Length());

     __be_int32_t oldLength = fText.Length();
    if(oldLength + length > fMaxBytes) length = fMaxBytes - oldLength;
    if(length > 0) fText.Insert(start, length, offset);
    if(fText.Length() == oldLength) return;

    if(offset < oldLength)
    {
        for(__be_int32_t i = 0; !(fRunArray == NULL || i >= fRunArray->count); i++)
        {
            bhapi::text_run *curRun = &(fRunArray->runs[i]);
             __be_int32_t nextOffset = (i < fRunArray->count - 1 ? fRunArray->runs[i + 1].offset : fText.Length());
            if(curRun->offset <= offset)
            {
                if(runs == NULL || nextOffset <= offset) continue;

                void *newPtr = realloc(fRunArray,
                               sizeof(bhapi::text_run_array) + (size_t)fRunArray->count * sizeof(bhapi::text_run));
                if(newPtr == NULL) continue;

                fRunArray = (bhapi::text_run_array*)newPtr;
                fRunArray->count += 1;
                i += 1;

                if(i != fRunArray->count - 1)
                    memmove(&(fRunArray->runs[i + 1]), &(fRunArray->runs[i]),
                        (size_t)(fRunArray->count - i - 1) * sizeof(bhapi::text_run));
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
         __be_int32_t aOffset = (utf8 ? bhapi::utf8_strlen_etc(fText.String(), offset) : offset);
         __be_int32_t aLen = (utf8 ? bhapi::utf8_strlen_etc(fText.String() + offset, length) : length);
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
BTextView::SetText(const char *text, const bhapi::text_run_array *runs, bool utf8)
{
    Delete(0, -1, false);
    if(text != NULL) Insert(0, text, (utf8 ? bhapi::utf8_strlen(text) : strlen(text)), runs, utf8);
    Invalidate();
}


void 
BTextView::SetText(const char *text,  __be_int32_t length, const bhapi::text_run_array *runs, bool utf8)
{
    Delete(0, -1, false);
    Insert(0, text, length, runs, utf8);
    Invalidate();
}


void 
BTextView::SetText(BFile *file,  __be_int64 fileOffset,  __be_int32_t length, const bhapi::text_run_array *runs, bool utf8)
{
    Delete(0, -1, false);
    if(file == NULL || length <= 0) return;

    char *buffer = (char*)malloc((size_t)length + 1);
    if(buffer == NULL) return;
    bzero(buffer, (size_t)length + 1);

     ssize_t nRead = file->ReadAt(fileOffset, buffer, (size_t)length);
    if(nRead > 0) Insert(0, buffer, (__be_int32_t)nRead, runs, utf8);

    free(buffer);
    Invalidate();
}


void 
BTextView::Delete(__be_int32_t startPos,  __be_int32_t endPos, bool utf8)
{
     __be_int32_t oldStart = fSelectStart, oldEnd = fSelectEnd;

    DeleteText(startPos, endPos, utf8);

    if(oldStart != fSelectStart || oldEnd != fSelectEnd)
    {
        // call virtual function
        Select(fSelectStart, fSelectEnd, false);
    }
}


void 
BTextView::DeleteText(__be_int32_t startPos,  __be_int32_t endPos, bool utf8)
{
    if(fText.Length() <= 0 || startPos < 0) return;

    if(endPos < 0 || endPos > (utf8 ? fText.CountChars() : fText.Length())) endPos = (utf8 ? fText.CountChars() : fText.Length());

    if(utf8 && endPos > startPos)
    {
        startPos = fText.CharAt(startPos, NULL) - fText.String();
        endPos = (endPos == fText.CountChars() ? fText.Length() : (fText.CharAt(endPos, NULL) - fText.String()));
    }

     __be_int32_t length = endPos - startPos;
    if(length <= 0) return;

     __be_int32_t oldLength = fText.Length();
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

            void *newPtr = realloc(fRunArray, sizeof(bhapi::text_run_array));
            if(newPtr) fRunArray = (bhapi::text_run_array*)newPtr;
        }
        else if((fRunArray = (bhapi::text_run_array*)malloc(sizeof(bhapi::text_run_array))) != NULL)
        {
            fRunArray->count = 1;
            fRunArray->runs[0].offset = 0;
            fRunArray->runs[0].font = *bhapi::plain_font;
            fRunArray->runs[0].color.set_to(0, 0, 0, 255);
            fRunArray->runs[0].background.set_to(0, 0, 0, 0);
            fRunArray->runs[0].underline = false;
        }
    }
    else for(__be_int32_t i = 0; !(fRunArray == NULL || i >= fRunArray->count); i++)
    {
        bhapi::text_run *curRun = &(fRunArray->runs[i]);
        bhapi::text_run *nextRun = (i < fRunArray->count - 1 ? &(fRunArray->runs[i + 1]) : NULL);

        if(curRun->offset < startPos) continue;
        if(curRun->offset >= endPos) {curRun->offset -= length; continue;}

        if(nextRun == NULL || nextRun->offset < endPos)
        {
            if(nextRun) memmove(curRun, nextRun, (size_t)(fRunArray->count - i - 1) * sizeof(bhapi::text_run));
            if(fRunArray->count > 1)
            {
                fRunArray->count -= 1; i--;
                void *newPtr = realloc(fRunArray,
                               sizeof(bhapi::text_run_array) + (size_t)(fRunArray->count - 1) * sizeof(bhapi::text_run));
                if(newPtr) fRunArray = (bhapi::text_run_array*)newPtr;
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
BTextView::MakeEditable(bool editable)
{
    if(fEditable != editable)
    {
        fEditable = editable;
        // TODO
    }
}


bool 
BTextView::IsEditable() const
{
    return fEditable;
}


void 
BTextView::MakeSelectable(bool selectable)
{
    if(fSelectable != selectable)
    {
        fSelectable = selectable;
        fSelectTracking = -1;
    }
}


bool 
BTextView::IsSelectable() const
{
    return fSelectable;
}


void 
BTextView::SetStylable(bool stylable)
{
    if(fStylable != stylable)
    {
        if((fStylable = stylable) == false)
        {
            if(!(fRunArray == NULL || fRunArray->count <= 1))
            {
                fRunArray->count = 1;
                void *newPtr = realloc(fRunArray, sizeof(bhapi::text_run_array));
                if(newPtr != NULL) fRunArray = (bhapi::text_run_array*)newPtr;
            }

            if(fRunArray) fRunArray->runs[0].offset = 0;
        }

        ReScanLines();
        Invalidate();
    }
}


bool 
BTextView::IsStylable() const
{
    return fStylable;
}


void 
BTextView::SetTabWidth(float width)
{
    if(fTabWidth != width)
    {
        fTabWidth = width;
        ReScanSize(0, -1);
        Invalidate();
    }
}


float
BTextView::TabWidth() const
{
    return fTabWidth;
}


void 
BTextView::SetAutoindent(bool flag)
{
    fAutoindent = flag;
}


bool 
BTextView::DoesAutoindent() const
{
    return fAutoindent;
}


void 
BTextView::HideTyping(__be_uint8 flag)
{
    if((flag != 0x00 && flag < 0x20) || flag > 0x7e) flag = 0x01;

    if(fTypingHidden != flag)
    {
        fTypingHidden = flag;
        ReScanSize(0, -1);
        Invalidate();
    }
}


__be_uint8
BTextView::IsTypingHidden() const
{
    return fTypingHidden;
}


void 
BTextView::SetAlignment(bhapi::alignment alignment)
{
    if(fAlignment != alignment)
    {
        fAlignment = alignment;
        Invalidate();
    }
}


bhapi::alignment
BTextView::Alignment() const
{
    return fAlignment;
}


void 
BTextView::SetMaxBytes(__be_int32_t max)
{
    if(max < 0) max = B_MAXINT32;
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


__be_int32_t
BTextView::MaxBytes() const
{
    return fMaxBytes;
}


void 
BTextView::Select(__be_int32_t startPos,  __be_int32_t endPos, bool utf8)
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
BTextView::GetSelection(__be_int32_t *startPos,  __be_int32_t *endPos, bool utf8) const
{
    bool isSelected = (fSelectEnd > fSelectStart && fSelectStart >= 0 && fSelectEnd <= fText.Length());

    if(isSelected)
    {
        if(startPos) *startPos = (utf8 ? bhapi::utf8_strlen_etc(fText.String(), fSelectStart) : fSelectStart);
        if(endPos) *endPos = (utf8 ? bhapi::utf8_strlen_etc(fText.String(), fSelectEnd) : fSelectEnd);
    }
    else
    {
        if(startPos) *startPos = 0;
        if(endPos) *endPos = 0;
    }

    return isSelected;
}


void 
BTextView::Draw(BRect updateRect)
{
    BRect rect = TextRect();
    if(!rect.IsValid()) return;

    BRegion clipping;
    GetClippingRegion(&clipping);
    if(clipping.CountRects() > 0) clipping &= (rect & updateRect);
    else clipping = (rect & updateRect);
    if(clipping.CountRects() <= 0) return;

    PushState();

    SetDrawingMode(B_OP_COPY);
    SetPenSize(0);
    ConstrainClippingRegion(&clipping);

    bhapi::rgb_color bkColor = fTextBkColor;
    if(!IsEnabled()) bkColor.disable(ViewColor());
    SetLowColor(bkColor);
    FillRect(rect & updateRect, B_SOLID_LOW);

    if(fTypingHidden == 0x01)
    {
        PopState();
        return;
    }

    BRegion selectRegion;
    GetTextRegion(fSelectStart, fSelectEnd, &selectRegion, false);
    selectRegion &= updateRect;
    if(selectRegion.CountRects() > 0)
    {
        bhapi::rgb_color color = bhapi::ui_color(B_DOCUMENT_HIGHLIGHT_COLOR);
        if(!IsEnabled()) color.disable(ViewColor());
        SetHighColor(color);
        FillRegion(&selectRegion, B_SOLID_HIGH);
    }

     __be_int32_t nextLineOffset = 0;

    BRect r = rect;
    r.bottom = r.top;

    for(__be_int32_t i = 0; i < fLines.CountItems(); i++)
    {
        bhapi::text_line *line = (bhapi::text_line*)fLines.ItemAt(i);

         __be_int32_t curLineOffset = nextLineOffset;
        const char *str = fText.String() + curLineOffset;

        nextLineOffset += line->length + 1;

        if(i > 0) r.top = r.bottom + BHAPI_TEXT_VIEW_LINE_SPACING + UnitsPerPixel();
        r.bottom = r.top + line->height;

        if(!clipping.Intersects(r)) continue;

        BPoint penLocation(r.left, r.top + line->max_ascent + 1);
         __be_int32_t k = 0;
         __be_int32_t cursorPos = -1;
        if(fEditable && i == fCurrentLine) cursorPos = ((fCursor >= 0 && fCursor <= line->length) ? fCursor : line->length);
        if(line->length == 0 && cursorPos < 0) continue;

        if(fAlignment == B_ALIGN_RIGHT) penLocation.x = r.right - line->width - 1.f;
        else if(fAlignment == B_ALIGN_CENTER) penLocation.x = r.left + (r.Width() - line->width) / 2.f - 1.f;

        do {
            if(line->array == NULL || line->array->count <= 0)
            {
                bhapi::rgb_color fgColor = bhapi::make_rgb_color(0, 0, 0, 255);
                if(!IsEnabled()) fgColor.disable(ViewColor());

                BView::SetHighColor(fgColor);
                _DrawString(*bhapi::plain_font, str, penLocation, line->length);

                if(cursorPos >= 0 && IsFocus())
                {
                    bhapi::rgb_color color = bhapi::ui_color(B_DOCUMENT_CURSOR_COLOR);
                    if(!IsEnabled()) color.disable(ViewColor());
                    BView::SetHighColor(color);
                    BRect aRect;
                    aRect.left = r.left + _StringWidth(*bhapi::plain_font, str, cursorPos);
                    aRect.right = aRect.left;
                    aRect.top = r.top;
                    aRect.bottom = r.top + line->height;
                    FillRect(aRect, B_SOLID_HIGH);
                }
            }
            else
            {
                bhapi::text_run *curRun = &(line->array->runs[k]);
                bhapi::text_run *nextRun = (k < line->array->count - 1 ? &(line->array->runs[k + 1]) : NULL);

                BFont curFont(curRun->font);

                bhapi::rgb_color fgColor = curRun->color;
                bkColor = curRun->background;
                 __be_int32_t len = (nextRun == NULL ? line->length : min_c(line->length, nextRun->offset)) - curRun->offset;
                float strWidth = _StringWidth(curFont, str + curRun->offset, len);

                if(!IsEnabled())
                {
                    fgColor.disable(ViewColor());
                    if(bkColor.alpha > 0) {bkColor.alpha = 0xff; bkColor.disable(ViewColor());}
                }

                if(bkColor.alpha > 0)
                {
                    BView::SetHighColor(bkColor);
                    BRect aRect;
                    aRect.left = penLocation.x;
                    aRect.right = penLocation.x + strWidth;
                    aRect.top = r.top;
                    aRect.bottom = r.top + line->height;

                    if(IsSelected() == false)
                    {
                        FillRect(aRect, B_SOLID_HIGH);
                    }
                    else
                    {
                         __be_int32_t pos0, pos1;
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
                                FillRect(aRect, B_SOLID_HIGH);
                                break;
                            }

                            if(pos0 > curRun->offset)
                            {
                                aRect.right = penLocation.x +
                                          _StringWidth(curFont,
                                               str + curRun->offset,
                                               pos0 - curRun->offset);
                                FillRect(aRect, B_SOLID_HIGH);
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
                                FillRect(aRect, B_SOLID_HIGH);
                            }
                        } while(false);
                    }
                }

                BView::SetHighColor(fgColor);
                _DrawString(curFont, str + curRun->offset, penLocation, len);
                if(curRun->underline) StrokeLine(penLocation, penLocation + BPoint(strWidth, 0), B_SOLID_HIGH);

                if(cursorPos >= curRun->offset && (nextRun == NULL || cursorPos < nextRun->offset) && IsFocus())
                {
                    bhapi::rgb_color color = bhapi::ui_color(B_DOCUMENT_CURSOR_COLOR);
                    if(!IsEnabled()) color.disable(ViewColor());
                    BView::SetHighColor(color);

                    bhapi::font_height fontHeight;
                    curFont.GetHeight(&fontHeight);

                    BRect aRect;
                    aRect.left = penLocation.x +
                             _StringWidth(curFont, str + curRun->offset, cursorPos - curRun->offset);
                    aRect.right = aRect.left;
                    aRect.top = penLocation.y - fontHeight.ascent - 1;
                    aRect.bottom = aRect.top + fontHeight.ascent + fontHeight.descent;
                    FillRect(aRect, B_SOLID_HIGH);
                }
                penLocation.x += strWidth;

                if(nextRun)
                {
                    BFont nextFont(nextRun->font);
                    penLocation.x += max_c(curFont.Spacing() * curFont.Size(), nextFont.Spacing() * nextFont.Size());
                }
            }
        } while(!(line->length == 0 || line->array == NULL || ++k >= line->array->count || penLocation.x > r.right));
    }

    if(fLines.CountItems() <= 0 && fEditable && IsFocus())
    {
        BFont font(*bhapi::plain_font);
        bhapi::font_height fontHeight;
        if(!(fRunArray == NULL || fRunArray->count <= 0)) font = fRunArray->runs[0].font;
        font.GetHeight(&fontHeight);

        bhapi::rgb_color color = bhapi::ui_color(B_DOCUMENT_CURSOR_COLOR);
        if(!IsEnabled()) color.disable(ViewColor());
        BView::SetHighColor(color);
        StrokeLine(rect.LeftTop(), rect.LeftTop() + BPoint(0, fontHeight.ascent + fontHeight.descent), B_SOLID_HIGH);
    }

    PopState();
}


void 
BTextView::GetPreferredSize(float *width, float *height)
{
    if(width)
    {
        *width = 0;
        for(__be_int32_t i = 0; i < fLines.CountItems(); i++)
        {
            bhapi::text_line *line = (bhapi::text_line*)fLines.ItemAt(i);
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
BTextView::SetTextRect(BRect textRect)
{
    BRect rect = Frame().OffsetToSelf(B_ORIGIN);
    if(!textRect.IsValid()) textRect = Frame().OffsetToSelf(B_ORIGIN);

    fMargins.left = textRect.left;
    fMargins.top = textRect.top;
    fMargins.right = rect.right - textRect.right;
    fMargins.bottom = rect.bottom - textRect.bottom;

    Invalidate();
}


BRect
BTextView::TextRect() const
{
    BRect rect = Frame().OffsetToSelf(B_ORIGIN);

    rect.left += fMargins.left;
    rect.top += fMargins.top;
    rect.right -= fMargins.right;
    rect.bottom -= fMargins.bottom;

    return rect;
}


void 
BTextView::SetTextBackground(bhapi::rgb_color color)
{
    if(fTextBkColor != color)
    {
        fTextBkColor = color;
        Invalidate();
    }
}


bhapi::rgb_color
BTextView::TextBackground() const
{
    return fTextBkColor;
}


void 
BTextView::FloorPosition(__be_int32_t *pos)
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
        if(bhapi::utf8_is_token(tmp--)) break;
        (*pos) -= 1;
    }
}


void 
BTextView::CeilPosition(__be_int32_t *pos)
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
        if(bhapi::utf8_is_token(tmp++)) break;
        (*pos) += 1;
    }
}


void 
BTextView::SetPosition(__be_int32_t pos, bool response, bool utf8)
{
    if(IsEditable() == false) return;

     __be_int32_t lineIndex = LineAt(pos, utf8);
     __be_int32_t currentLine = fCurrentLine;
     __be_int32_t currentCursor = fCursor;
     __be_int32_t lineOffset = 0;

    do {
        if(lineIndex < 0) break;

        currentLine = lineIndex;
        bhapi::text_line *line = (bhapi::text_line*)fLines.ItemAt(currentLine);

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
                        fText.CharAt(bhapi::utf8_strlen_etc(fText.String(), lineOffset), NULL);
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


__be_int32_t
BTextView::Position(bool utf8,  __be_int32_t *lineOffset) const
{
     __be_int32_t pos = 0;
    if(lineOffset) *lineOffset = 0;

    for(__be_int32_t i = 0; i <= fCurrentLine && i < fLines.CountItems(); i++)
    {
        bhapi::text_line *line = (bhapi::text_line*)fLines.ItemAt(i);
        if(i == fCurrentLine)
        {
            if(lineOffset) *lineOffset = (utf8 ? bhapi::utf8_strlen_etc(fText.String(), pos) : pos);
            pos += min_c(line->length, fCursor);
            return(utf8 ? bhapi::utf8_strlen_etc(fText.String(), pos) : pos);
        }
        pos += line->length + 1;
    }

    return 0;
}


void 
BTextView::MouseDown(BPoint where)
{
    if(!IsEnabled() || !TextRect().Contains(where) || !QueryCurrentMouse(true, B_PRIMARY_MOUSE_BUTTON)) return;
    if(!(IsEditable() || IsSelectable())) return;
    if(!IsFocus()) MakeFocus(true);

     __be_int32_t pos = OffsetAt(where, true, false);
    if(pos < 0) return;

    bool redraw = IsSelected();

    if(IsFocus() && fSelectTracking < 0)
    {
        if(!(!IsSelectable() || SetPrivateEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS) != B_OK))
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
BTextView::MouseUp(BPoint where)
{
    fSelectTracking = -1;
    if(TextRect().Contains(where)) bhapi::app->ObscureCursor();
}


void 
BTextView::MouseMoved(BPoint where,  __be_uint32_t code, const BMessage *a_message)
{
    if(TextRect().Contains(where) == false || code == B_EXITED_VIEW)
    {
        bhapi::app->SetCursor(B_CURSOR_SYSTEM_DEFAULT, false);
        return;
    }

    bhapi::app->SetCursor(B_CURSOR_I_BEAM, false);

    if(!IsEnabled() || !IsSelectable() || fSelectTracking < 0) return;

    BWindow *win = Window();
    if(!win) return;

    if(!VisibleBounds().Contains(where)) return;
    if(!(IsEditable() || IsSelectable())) return;

     __be_int32_t pos = OffsetAt(where, true, false);
    if(pos < 0) return;

    bool redraw = false;

     __be_int32_t oldStart = fSelectStart;
     __be_int32_t oldEnd = fSelectEnd;
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
BTextView::Cut(BClipboard *clipboard)
{
    if(clipboard == NULL || IsSelected() == false) return;

    BTextView::Copy(clipboard);

     __be_int32_t oldPos = fSelectStart;
    Delete();
    SetPosition(oldPos, true, false);

    Invalidate();
}


void 
BTextView::Copy(BClipboard *clipboard) const
{
    BMessage *clipMsg = NULL;

    if(clipboard == NULL || IsSelected() == false || clipboard->Lock() == false) return;
    if((clipMsg = clipboard->Data()) != NULL)
    {
        clipboard->Clear();

         __be_int32_t runsBytes = 0;
        bhapi::text_run_array *runs = RunArray(fSelectStart, fSelectEnd, &runsBytes, false);
        clipMsg->AddData("text/plain", B_MIME_TYPE,
                 fText.String() + fSelectStart, (ssize_t)(fSelectEnd - fSelectStart));
        if(runs != NULL)
        {
            clipMsg->AddData("text/runs", B_MIME_TYPE, runs, (ssize_t)runsBytes);
            free(runs);
        }

        clipboard->Commit();
    }

    clipboard->Unlock();
}


bool 
BTextView::AcceptsPaste(BClipboard *clipboard)
{
    return(fEditable);
}


void 
BTextView::Paste(BClipboard *clipboard)
{
    BString str;
    BMessage *clipMsg = NULL;
    bhapi::text_run_array *runs = NULL;

    if(AcceptsPaste(clipboard) == false || clipboard == NULL || clipboard->Lock() == false) return;
    if((clipMsg = clipboard->Data()) != NULL)
    {
        const char *text = NULL;
         ssize_t len = 0;
        if(clipMsg->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &len)) str.SetTo(text, (__be_int32_t)len);

        void *tmp = NULL;
        if(!(fStylable == false ||
             clipMsg->FindData("text/runs", B_MIME_TYPE, (const void**)&tmp, &len) == false ||
             len < (ssize_t)sizeof(bhapi::text_run_array) ||
             ((size_t)len - sizeof(bhapi::text_run_array)) % sizeof(bhapi::text_run) != 0 ||
             (runs = (bhapi::text_run_array*)malloc((size_t)len)) == NULL))
        {
            memcpy((void*)runs, tmp, (size_t)len);
        }
    }
    clipboard->Unlock();

    if(str.Length() > 0)
    {
         __be_int32_t curPos = Position(false, NULL);
        if(IsSelected()) {curPos = fSelectStart; Delete();}

         __be_int32_t oldLength = fText.Length();
        Insert(curPos, str.String(), -1, runs, false);
        SetPosition(curPos + (fText.Length() - oldLength), true, false);
    }

    if(runs) free(runs);

    Invalidate();
}


void 
BTextView::KeyDown(const char *bytes,  __be_int32_t numBytes)
{
    if(!IsEnabled() || !(IsEditable() || IsSelectable()) || !IsFocus() || numBytes < 1) return;

    BWindow *win = Window();
    if(!win) return;

    BMessage *msg = win->CurrentMessage();
    if(!msg || !(msg->what == B_KEY_DOWN || msg->what == B_UNMAPPED_KEY_DOWN)) return;

     __be_int32_t modifiers = 0;
    msg->FindInt32("modifiers", &modifiers);

    if(!(numBytes != 1 || msg->what != B_KEY_DOWN || !IsEnabled() || !IsEditable() || !(modifiers & B_CONTROL_KEY)))
    {
        if(*bytes == 'c' || *bytes == 'C') {Copy(&bhapi::__be_clipboard); return;}
        else if(*bytes == 'x' || *bytes == 'X') {Cut(&bhapi::__be_clipboard); return;}
        else if(*bytes == 'v' || *bytes == 'V') {Paste(&bhapi::__be_clipboard); return;}
    }

    if((modifiers & B_CONTROL_KEY) || (modifiers & B_COMMAND_KEY) ||
       (modifiers & B_MENU_KEY) || (modifiers & B_OPTION_KEY)) return;

    bool shift_only = false;
    if(IsSelectable())
    {
        modifiers &= ~(B_CAPS_LOCK | B_SCROLL_LOCK | B_NUM_LOCK | B_LEFT_SHIFT_KEY | B_RIGHT_SHIFT_KEY);
        if(modifiers == B_SHIFT_KEY) shift_only = true;
    }

    if(numBytes == 1)
    {
        // TODO: region update
        bhapi::text_line *line = (bhapi::text_line*)fLines.ItemAt(fCurrentLine);
        switch(bytes[0])
        {
            case B_ESCAPE:
                if(IsSelectable() && (fSelectTracking > 0 || IsSelected()))
                {
                    fSelectTracking = -1;
                    fSelectStart = fSelectEnd = -1;

                    // call virtual function
                    Select(fSelectStart, fSelectEnd, false);

                    Invalidate();
                }
                break;

            case B_UP_ARROW:
                if(fCurrentLine <= 0) break;
            case B_LEFT_ARROW:
                {
                    if(line == NULL) break;

                     __be_int32_t fPosition = Position(false, NULL);
                     __be_int32_t oldStart = fSelectStart;
                     __be_int32_t oldEnd = fSelectEnd;
                    bool redraw = false;

                     __be_int32_t nPosition;
                    if(bytes[0] == B_UP_ARROW)
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

            case B_DOWN_ARROW:
                if(fCurrentLine >= fLines.CountItems()) break;
            case B_RIGHT_ARROW:
                {
                    if(line == NULL) break;

                     __be_int32_t fPosition = Position(false, NULL);
                     __be_int32_t oldStart = fSelectStart;
                     __be_int32_t oldEnd = fSelectEnd;
                    bool redraw = false;

                     __be_int32_t nPosition;
                    if(bytes[0] == B_DOWN_ARROW)
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

            case B_DELETE:
                if(line == NULL || IsEditable() == false) break;
                if(IsSelectable() && IsSelected())
                {
                     __be_int32_t oldPos = fSelectStart;
                    Delete();
                    SetPosition(oldPos, true, false);
                }
                else
                {
                     __be_int32_t pos = Position(false, NULL);
                     __be_int32_t nextPos = pos + 1;
                    CeilPosition(&nextPos);
                    Delete(pos, nextPos);
                }
                fSelectTracking = -1;
                Invalidate();
                break;

            case B_BACKSPACE:
                if(line == NULL || IsEditable() == false) break;
                if(IsSelectable() && IsSelected())
                {
                     __be_int32_t oldPos = fSelectStart;
                    Delete();
                    SetPosition(oldPos, true, false);
                }
                else
                {
                     __be_int32_t pos = Position(false, NULL);
                     __be_int32_t prevPos = pos - 1;
                    FloorPosition(&prevPos);
                    Delete(prevPos, pos);
                    SetPosition(prevPos, true, false);
                }
                fSelectTracking = -1;
                Invalidate();
                break;

            case B_HOME:
                {
                    if(line == NULL) break;

                     __be_int32_t lineOffset;
                     __be_int32_t fPosition = Position(false, &lineOffset);
                     __be_int32_t oldStart = fSelectStart;
                     __be_int32_t oldEnd = fSelectEnd;
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

            case B_END:
                {
                    if(line == NULL) break;

                     __be_int32_t lineOffset;
                     __be_int32_t fPosition = Position(false, &lineOffset);
                     __be_int32_t oldStart = fSelectStart;
                     __be_int32_t oldEnd = fSelectEnd;
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

            case B_PAGE_DOWN:
            case B_PAGE_UP:
                {
                    BScrollView *scrollView = cast_as(Parent(), BScrollView);
                    if(scrollView == NULL || scrollView->Target() != this) break;

                    BRect validRect = ConvertFromParent(scrollView->TargetFrame());
                    float yOffset = 0;
                    if(bytes[0] == B_PAGE_UP)
                    {
                        if(validRect.top <= 0) break;
                        yOffset = -min_c(validRect.top, validRect.Height());
                    }
                    else
                    {
                        BRect bounds = Frame().OffsetToSelf(B_ORIGIN);
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
                        BString aStr(bytes, 1);

                        if(bytes[0] == '\n' && fAutoindent)
                        {
                            for(__be_int32_t i = OffsetAt(fCurrentLine, false); i >= 0 && i < fText.Length(); i++)
                            {
                                if(!(fText[i] == ' ' || fText[i] == '\t')) break;
                                aStr << fText[i];
                            }
                        }

                        if(IsSelectable() && IsSelected())
                        {
                             __be_int32_t oldPos = fSelectStart;
                            Delete();

                             __be_int32_t oldLength = fText.Length();
                            Insert(oldPos, aStr.String(), aStr.Length(), NULL, false);
                            SetPosition(oldPos + (fText.Length() - oldLength), true, false);
                        }
                        else
                        {
                             __be_int32_t oldPos = Position(false, NULL);

                             __be_int32_t oldLength = fText.Length();
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
                 __be_int32_t oldPos = fSelectStart;
                Delete();

                 __be_int32_t oldLength = fText.Length();
                Insert(oldPos, bytes, numBytes, NULL, false);
                SetPosition(oldPos + (fText.Length() - oldLength), true, false);
            }
            else
            {
                 __be_int32_t oldPos = Position(false, NULL);

                 __be_int32_t oldLength = fText.Length();
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
BTextView::KeyUp(const char *bytes,  __be_int32_t numBytes)
{
}


void 
BTextView::MessageReceived(BMessage *msg)
{
    if(msg->what == B_MODIFIERS_CHANGED)
    {
         __be_int32_t modifiers = 0, old_modifiers = 0;
        msg->FindInt32("modifiers", &modifiers);
        msg->FindInt32("BHAPI:old_modifiers", &old_modifiers);
        if((old_modifiers & B_SHIFT_KEY) && !(modifiers & B_SHIFT_KEY)) fSelectTracking = -1;
    }
    BView::MessageReceived(msg);
}


void 
BTextView::WindowActivated(bool state)
{
    fSelectTracking = -1;
    // TODO
}


void 
BTextView::MakeFocus(bool focusState)
{
    if(!focusState) fSelectTracking = -1;
    BView::MakeFocus(focusState);
}


float
BTextView::_StringWidth(const BFont &font, const char *str,  __be_int32_t length) const
{
    if(fTypingHidden == 0x01 || str == NULL || *str == 0 || length == 0) return 0;
    if(fTypingHidden == 0x00) return font.StringWidth(str, length, fTabWidth);

    BString aStr;
    aStr.Append(*((char*)&fTypingHidden), bhapi::utf8_strlen_etc(str, length));
    return font.StringWidth(aStr.String(), aStr.Length(), 0);
}


void 
BTextView::_DrawString(const BFont &font, const char *str, BPoint location,  __be_int32_t length)
{
    if(fTypingHidden == 0x01 || str == NULL || *str == 0 || length == 0) return;

    BView::SetFont(&font, B_FONT_ALL);

    if(fTypingHidden == 0x00)
    {
        DrawString(str, location, length, fTabWidth);
    }
    else
    {
        BString aStr;
        aStr.Append(*((char*)&fTypingHidden), bhapi::utf8_strlen_etc(str, length));
        DrawString(aStr.String(), location, aStr.Length(), 0);
    }
}

