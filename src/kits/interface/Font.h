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
 * File: Font.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_FONT_H
#define BHAPI_FONT_H

#include "../support/SupportDefs.h"
#include "../support/String.h"

#define B_FONT_FAMILY_LENGTH	127
#define B_FONT_STYLE_LENGTH	63
#define B_FONT_MIN_TAB_WIDTH	(-16.f)

namespace bhapi {
typedef char font_family[B_FONT_FAMILY_LENGTH + 1];
typedef char font_style[B_FONT_STYLE_LENGTH + 1];

typedef struct font_height {
	float ascent;
	float descent;
	float leading;
} font_height;

typedef struct font_desc font_desc;
}

#ifdef __cplusplus /* Just for C++ */

class BView;
class BFontEngine;

class IMPEXPBHAPI BFont {
public:
	BFont();
	BFont(const BFont &font);
	BFont(const BFont *font);
    BFont(const bhapi::font_desc &fontDesc);
	virtual ~BFont();

    b_status_t	SetFamilyAndStyle(const bhapi::font_family family, const bhapi::font_style style);
    b_status_t	SetFamilyAndStyle(b_uint32 code);

    b_status_t	GetFamilyAndStyle(bhapi::font_family *family, bhapi::font_style *style) const;
    b_uint32		FamilyAndStyle() const;

	void		SetSize(float size);
	void		SetSpacing(float spacing);
	void		SetShear(float shear);
	void		SetBoldStyle(bool bold);

	float		Size() const;
	float		Spacing() const;
	float		Shear() const;
	bool		IsBoldStyle() const;

	bool		IsScalable() const;
    bool		HasFixedSize(b_int32 *count = NULL) const;
    bool		GetFixedSize(float *size, b_int32 index = 0) const;

	// tabWidth:
	// 	positive --- fixed size
	// 	0        --- decided on the font
	// 	negative --- multiple of space
    float		StringWidth(const char *string, b_int32 length = -1, float tabWidth = 0) const;
    float		StringWidth(const BString &str, b_int32 length = -1, float tabWidth = 0) const;
    void		GetHeight(bhapi::font_height *height) const;

	// CharWidths(): return value must free by "delete[]"
    float		*CharWidths(const char *string, b_int32 *nChars, float tabWidth = 0) const;
    float		*CharWidths(const BString &str, b_int32 *nChars, float tabWidth = 0) const;
    float		*CharWidths(const char *string, b_int32 length, b_int32 *nChars, float tabWidth = 0) const;
    float		*CharWidths(const BString &str, b_int32 length, b_int32 *nChars, float tabWidth = 0) const;

	BFont		&operator=(const BFont &font);
    BFont		&operator=(const bhapi::font_desc &fontDesc);

	bool		operator==(const BFont &font);
	bool		operator!=(const BFont &font);

	void		PrintToStream() const;

private:
	friend class BView;

	void *fInfo;

	BFontEngine *Engine() const;
};

namespace bhapi {
extern IMPEXPBHAPI const BFont* plain_font;
extern IMPEXPBHAPI const BFont* bold_font;
extern IMPEXPBHAPI const BFont* fixed_font;
IMPEXPBHAPI b_int32	count_font_families(void);
IMPEXPBHAPI b_status_t	get_font_family(b_int32 index, const char **name);
IMPEXPBHAPI b_int32	get_font_family_index(const char *name);
IMPEXPBHAPI b_int32	count_font_styles(const char *family);
IMPEXPBHAPI b_int32	count_font_styles(b_int32 index);
IMPEXPBHAPI b_status_t	get_font_style(const char *family, b_int32 index, const char **name);
IMPEXPBHAPI b_int32	get_font_style_index(const char *family, const char *name);
IMPEXPBHAPI bool	update_font_families(bool check_only);
}
#endif /* __cplusplus */

namespace bhapi {

struct font_desc {
    bhapi::font_family	family;
    bhapi::font_style	style;
	float		size;
	float		spacing;
	float		shear;
	bool		bold;
#ifdef __cplusplus
    inline font_desc()
	{
        bzero(this, sizeof(*this));
	}

    inline font_desc &operator=(const BFont &from)
	{
		from.GetFamilyAndStyle(&family, &style);
		size = from.Size();
		spacing = from.Spacing();
		shear = from.Shear();
		bold = from.IsBoldStyle();
		return *this;
	}

    inline void SetFamilyAndStyle(const bhapi::font_family f, const bhapi::font_style s)
	{
        bzero(family, sizeof(bhapi::font_family));
        bzero(style, sizeof(bhapi::font_style));
        memcpy(family, f, min_c(B_FONT_FAMILY_LENGTH, strlen(f)));
        memcpy(style, s, min_c(B_FONT_STYLE_LENGTH, strlen(s)));
	}

	inline void SetSize(float val) {size = val;}
	inline void SetSpacing(float val) {spacing = val;}
	inline void SetShear(float val) {shear = val;}
	inline void SetBoldStyle(bool val) {bold = val;}

    inline void GetFamilyAndStyle(bhapi::font_family *f, bhapi::font_style *s)
	{
		if(f) memcpy(*f, family, B_FONT_FAMILY_LENGTH + 1);
		if(s) memcpy(*s, style, B_FONT_STYLE_LENGTH + 1);
	}

	inline float Size() const {return size;}
	inline float Spacing() const {return spacing;}
	inline float Shear() const {return shear;}
	inline bool IsBoldStyle() const {return bold;}
#endif /* __cplusplus */
};
} /* namespace bhapi */
#endif /* BHAPI_FONT_H */

