#ifndef _UNICODE_CHAR_H_
#define _UNICODE_CHAR_H_

#include <Haiku.h>

enum unicode_char_category
{
	// Non-category for unassigned and non-character code points.
	B_UNICODE_UNASSIGNED				= 0,

	B_UNICODE_GENERAL_OTHER_TYPES		= 0,	// Cn
	B_UNICODE_UPPERCASE_LETTER			= 1,	// Lu
	B_UNICODE_LOWERCASE_LETTER			= 2,	// Ll
	B_UNICODE_TITLECASE_LETTER			= 3,	// Lt
	B_UNICODE_MODIFIER_LETTER			= 4,	// Lm
	B_UNICODE_OTHER_LETTER				= 5,	// Lo
	B_UNICODE_NON_SPACING_MARK			= 6,	// Mn
	B_UNICODE_ENCLOSING_MARK			= 7,	// Me
	B_UNICODE_COMBINING_SPACING_MARK	= 8,	// Mc
	B_UNICODE_DECIMAL_DIGIT_NUMBER		= 9,	// Nd
	B_UNICODE_LETTER_NUMBER				= 10,	// Nl
	B_UNICODE_OTHER_NUMBER				= 11,	// No
	B_UNICODE_SPACE_SEPARATOR			= 12,	// Zs
	B_UNICODE_LINE_SEPARATOR			= 13,	// Zl
	B_UNICODE_PARAGRAPH_SEPARATOR		= 14,	// Zp
	B_UNICODE_CONTROL_CHAR				= 15,	// Cc
	B_UNICODE_FORMAT_CHAR				= 16,	// Cf
	B_UNICODE_PRIVATE_USE_CHAR			= 17,	// Co
	B_UNICODE_SURROGATE					= 18,	// Cs
	B_UNICODE_DASH_PUNCTUATION			= 19,	// Pd
	B_UNICODE_START_PUNCTUATION			= 20,	// Ps
	B_UNICODE_END_PUNCTUATION			= 21,	// Pe
	B_UNICODE_CONNECTOR_PUNCTUATION		= 22,	// Pc
	B_UNICODE_OTHER_PUNCTUATION			= 23,	// Po
	B_UNICODE_MATH_SYMBOL				= 24,	// Sm
	B_UNICODE_CURRENCY_SYMBOL			= 25,	// Sc
	B_UNICODE_MODIFIER_SYMBOL			= 26,	// Sk
	B_UNICODE_OTHER_SYMBOL				= 27,	// So
	B_UNICODE_INITIAL_PUNCTUATION		= 28,	// Pi
	B_UNICODE_FINAL_PUNCTUATION			= 29,	// Pf

	B_UNICODE_CATEGORY_COUNT
};


// This specifies the language directional property of a character set.

enum unicode_char_direction {
	B_UNICODE_LEFT_TO_RIGHT					= 0,
	B_UNICODE_RIGHT_TO_LEFT					= 1,
	B_UNICODE_EUROPEAN_NUMBER				= 2,
	B_UNICODE_EUROPEAN_NUMBER_SEPARATOR		= 3,
	B_UNICODE_EUROPEAN_NUMBER_TERMINATOR	= 4,
	B_UNICODE_ARABIC_NUMBER					= 5,
	B_UNICODE_COMMON_NUMBER_SEPARATOR		= 6,
	B_UNICODE_BLOCK_SEPARATOR				= 7,
	B_UNICODE_SEGMENT_SEPARATOR				= 8,
	B_UNICODE_WHITE_SPACE_NEUTRAL			= 9,
	B_UNICODE_OTHER_NEUTRAL					= 10,
	B_UNICODE_LEFT_TO_RIGHT_EMBEDDING		= 11,
	B_UNICODE_LEFT_TO_RIGHT_OVERRIDE		= 12,
	B_UNICODE_RIGHT_TO_LEFT_ARABIC			= 13,
	B_UNICODE_RIGHT_TO_LEFT_EMBEDDING		= 14,
	B_UNICODE_RIGHT_TO_LEFT_OVERRIDE		= 15,
	B_UNICODE_POP_DIRECTIONAL_FORMAT		= 16,
	B_UNICODE_DIR_NON_SPACING_MARK			= 17,
	B_UNICODE_BOUNDARY_NEUTRAL				= 18,

	B_UNICODE_DIRECTION_COUNT
};


// Script range as defined in the Unicode standard.

enum unicode_char_script {
	// New No_Block value in Unicode 4.
	B_UNICODE_NO_BLOCK								= 0, // [none] Special range
	B_UNICODE_BASIC_LATIN							= 1, // [0000]
	B_UNICODE_LATIN_1_SUPPLEMENT					= 2, // [0080]
	B_UNICODE_LATIN_EXTENDED_A						= 3, // [0100]
	B_UNICODE_LATIN_EXTENDED_B						= 4, // [0180]
	B_UNICODE_IPA_EXTENSIONS						= 5, // [0250]
	B_UNICODE_SPACING_MODIFIER_LETTERS				= 6, // [02B0]
	B_UNICODE_COMBINING_DIACRITICAL_MARKS			= 7, // [0300]
	B_UNICODE_GREEK									= 8, // [0370]
	B_UNICODE_CYRILLIC								= 9, // [0400]
	B_UNICODE_ARMENIAN								= 10, // [0530]
	B_UNICODE_HEBREW								= 11, // [0590]
	B_UNICODE_ARABIC								= 12, // [0600]
	B_UNICODE_SYRIAC								= 13, // [0700]
	B_UNICODE_THAANA								= 14, // [0780]
	B_UNICODE_DEVANAGARI							= 15, // [0900]
	B_UNICODE_BENGALI								= 16, // [0980]
	B_UNICODE_GURMUKHI								= 17, // [0A00]
	B_UNICODE_GUJARATI								= 18, // [0A80]
	B_UNICODE_ORIYA									= 19, // [0B00]
	B_UNICODE_TAMIL									= 20, // [0B80]
	B_UNICODE_TELUGU								= 21, // [0C00]
	B_UNICODE_KANNADA								= 22, // [0C80]
	B_UNICODE_MALAYALAM								= 23, // [0D00]
	B_UNICODE_SINHALA								= 24, // [0D80]
	B_UNICODE_THAI									= 25, // [0E00]
	B_UNICODE_LAO									= 26, // [0E80]
	B_UNICODE_TIBETAN								= 27, // [0F00]
	B_UNICODE_MYANMAR								= 28, // [1000]
	B_UNICODE_GEORGIAN								= 29, // [10A0]
	B_UNICODE_HANGUL_JAMO							= 30, // [1100]
	B_UNICODE_ETHIOPIC								= 31, // [1200]
	B_UNICODE_CHEROKEE								= 32, // [13A0]
	B_UNICODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS	= 33, // [1400]
	B_UNICODE_OGHAM									= 34, // [1680]
	B_UNICODE_RUNIC									= 35, // [16A0]
	B_UNICODE_KHMER									= 36, // [1780]
	B_UNICODE_MONGOLIAN								= 37, // [1800]
	B_UNICODE_LATIN_EXTENDED_ADDITIONAL				= 38, // [1E00]
	B_UNICODE_GREEK_EXTENDED						= 39, // [1F00]
	B_UNICODE_GENERAL_PUNCTUATION					= 40, // [2000]
	B_UNICODE_SUPERSCRIPTS_AND_SUBSCRIPTS			= 41, // [2070]
	B_UNICODE_CURRENCY_SYMBOLS						= 42, // [20A0]
	B_UNICODE_COMBINING_MARKS_FOR_SYMBOLS			= 43, // [20D0]
	B_UNICODE_LETTERLIKE_SYMBOLS					= 44, // [2100]
	B_UNICODE_NUMBER_FORMS							= 45, // [2150]
	B_UNICODE_ARROWS								= 46, // [2190]
	B_UNICODE_MATHEMATICAL_OPERATORS				= 47, // [2200]
	B_UNICODE_MISCELLANEOUS_TECHNICAL				= 48, // [2300]
	B_UNICODE_CONTROL_PICTURES						= 49, // [2400]
	B_UNICODE_OPTICAL_CHARACTER_RECOGNITION			= 50, // [2440]
	B_UNICODE_ENCLOSED_ALPHANUMERICS				= 51, // [2460]
	B_UNICODE_BOX_DRAWING							= 52, // [2500]
	B_UNICODE_BLOCK_ELEMENTS						= 53, // [2580]
	B_UNICODE_GEOMETRIC_SHAPES						= 54, // [25A0]
	B_UNICODE_MISCELLANEOUS_SYMBOLS					= 55, // [2600]
	B_UNICODE_DINGBATS								= 56, // [2700]
	B_UNICODE_BRAILLE_PATTERNS						= 57, // [2800]
	B_UNICODE_CJK_RADICALS_SUPPLEMENT				= 58, // [2E80]
	B_UNICODE_KANGXI_RADICALS						= 59, // [2F00]
	B_UNICODE_IDEOGRAPHIC_DESCRIPTION_CHARACTERS	= 60, // [2FF0]
	B_UNICODE_CJK_SYMBOLS_AND_PUNCTUATION			= 61, // [3000]
	B_UNICODE_HIRAGANA								= 62, // [3040]
	B_UNICODE_KATAKANA								= 63, // [30A0]
	B_UNICODE_BOPOMOFO								= 64, // [3100]
	B_UNICODE_HANGUL_COMPATIBILITY_JAMO				= 65, // [3130]
	B_UNICODE_KANBUN								= 66, // [3190]
	B_UNICODE_BOPOMOFO_EXTENDED						= 67, // [31A0]
	B_UNICODE_ENCLOSED_CJK_LETTERS_AND_MONTHS		= 68, // [3200]
	B_UNICODE_CJK_COMPATIBILITY						= 69, // [3300]
	B_UNICODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A	= 70, // [3400]
	B_UNICODE_CJK_UNIFIED_IDEOGRAPHS				= 71, // [4E00]
	B_UNICODE_YI_SYLLABLES							= 72, // [A000]
	B_UNICODE_YI_RADICALS							= 73, // [A490]
	B_UNICODE_HANGUL_SYLLABLES						= 74, // [AC00]
	B_UNICODE_HIGH_SURROGATES						= 75, // [D800]
	B_UNICODE_HIGH_PRIVATE_USE_SURROGATES			= 76, // [DB80]
	B_UNICODE_LOW_SURROGATES						= 77, // [DC00]
	B_UNICODE_PRIVATE_USE							= 78,
	B_UNICODE_PRIVATE_USE_AREA = B_UNICODE_PRIVATE_USE, // [E000]
	B_UNICODE_CJK_COMPATIBILITY_IDEOGRAPHS			= 79, // [F900]
	B_UNICODE_ALPHABETIC_PRESENTATION_FORMS			= 80, // [FB00]
	B_UNICODE_ARABIC_PRESENTATION_FORMS_A			= 81, // [FB50]
	B_UNICODE_COMBINING_HALF_MARKS					= 82, // [FE20]
	B_UNICODE_CJK_COMPATIBILITY_FORMS				= 83, // [FE30]
	B_UNICODE_SMALL_FORM_VARIANTS					= 84, // [FE50]
	B_UNICODE_ARABIC_PRESENTATION_FORMS_B			= 85, // [FE70]
	B_UNICODE_SPECIALS								= 86, // [FFF0]
	B_UNICODE_HALFWIDTH_AND_FULLWIDTH_FORMS			= 87, // [FF00]

	// New blocks in Unicode 3.1
	B_UNICODE_OLD_ITALIC							= 88, // [10300]
	B_UNICODE_GOTHIC								= 89, // [10330]
	B_UNICODE_DESERET								= 90, // [10400]
	B_UNICODE_BYZANTINE_MUSICAL_SYMBOLS				= 91, // [1D000]
	B_UNICODE_MUSICAL_SYMBOLS						= 92, // [1D100]
	B_UNICODE_MATHEMATICAL_ALPHANUMERIC_SYMBOLS		= 93, // [1D400]
	B_UNICODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B	= 94, // [20000]
	B_UNICODE_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT = 95, // [2F800]
	B_UNICODE_TAGS									= 96, // [E0000]

	// New blocks in Unicode
	B_UNICODE_CYRILLIC_SUPPLEMENTARY				= 97,
	B_UNICODE_CYRILLIC_SUPPLEMENT = B_UNICODE_CYRILLIC_SUPPLEMENTARY, // [0500]
	B_UNICODE_TAGALOG								= 98, // [1700]
	B_UNICODE_HANUNOO								= 99, // [1720]
	B_UNICODE_BUHID									= 100, // [1740]
	B_UNICODE_TAGBANWA								= 101, // [1760]
	B_UNICODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A	= 102, // [27C0]
	B_UNICODE_SUPPLEMENTAL_ARROWS_A					= 103, // [27F0]
	B_UNICODE_SUPPLEMENTAL_ARROWS_B					= 104, // [2900]
	B_UNICODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B	= 105, // [2980]
	B_UNICODE_SUPPLEMENTAL_MATHEMATICAL_OPERATORS	= 106, // [2A00]
	B_UNICODE_KATAKANA_PHONETIC_EXTENSIONS			= 107, // [31F0]
	B_UNICODE_VARIATION_SELECTORS					= 108, // [FE00]
	B_UNICODE_SUPPLEMENTARY_PRIVATE_USE_AREA_A		= 109, // [F0000]
	B_UNICODE_SUPPLEMENTARY_PRIVATE_USE_AREA_B		= 110, // [100000]

	// New blocks in Unicode 4
	B_UNICODE_LIMBU									= 111, // [1900]
	B_UNICODE_TAI_LE								= 112, // [1950]
	B_UNICODE_KHMER_SYMBOLS							= 113, // [19E0]
	B_UNICODE_PHONETIC_EXTENSIONS					= 114, // [1D00]
	B_UNICODE_MISCELLANEOUS_SYMBOLS_AND_ARROWS		= 115, // [2B00]
	B_UNICODE_YIJING_HEXAGRAM_SYMBOLS				= 116, // [4DC0]
	B_UNICODE_LINEAR_B_SYLLABARY					= 117, // [10000]
	B_UNICODE_LINEAR_B_IDEOGRAMS					= 118, // [10080]
	B_UNICODE_AEGEAN_NUMBERS						= 119, // [10100]
	B_UNICODE_UGARITIC								= 120, // [10380]
	B_UNICODE_SHAVIAN								= 121, // [10450]
	B_UNICODE_OSMANYA								= 122, // [10480]
	B_UNICODE_CYPRIOT_SYLLABARY						= 123, // [10800]
	B_UNICODE_TAI_XUAN_JING_SYMBOLS					= 124, // [1D300]
	B_UNICODE_VARIATION_SELECTORS_SUPPLEMENT		= 125, // [E0100]

	// New blocks in Unicode 4.1
	B_UNICODE_ANCIENT_GREEK_MUSICAL_NOTATION		= 126, // [1D200]
	B_UNICODE_ANCIENT_GREEK_NUMBERS					= 127, // [10140]
	B_UNICODE_ARABIC_SUPPLEMENT						= 128, // [0750]
	B_UNICODE_BUGINESE								= 129, // [1A00]
	B_UNICODE_CJK_STROKES							= 130, // [31C0]
	B_UNICODE_COMBINING_DIACRITICAL_MARKS_SUPPLEMENT = 131, // [1DC0]
	B_UNICODE_COPTIC								= 132, // [2C80]
	B_UNICODE_ETHIOPIC_EXTENDED						= 133, // [2D80]
	B_UNICODE_ETHIOPIC_SUPPLEMENT					= 134, // [1380]
	B_UNICODE_GEORGIAN_SUPPLEMENT					= 135, // [2D00]
	B_UNICODE_GLAGOLITIC							= 136, // [2C00]
	B_UNICODE_KHAROSHTHI							= 137, // [10A00]
	B_UNICODE_MODIFIER_TONE_LETTERS					= 138, // [A700]
	B_UNICODE_NEW_TAI_LUE							= 139, // [1980]
	B_UNICODE_OLD_PERSIAN							= 140, // [103A0]
	B_UNICODE_PHONETIC_EXTENSIONS_SUPPLEMENT		= 141, // [1D80]
	B_UNICODE_SUPPLEMENTAL_PUNCTUATION				= 142, // [2E00]
	B_UNICODE_SYLOTI_NAGRI							= 143, // [A800]
	B_UNICODE_TIFINAGH								= 144, // [2D30]
	B_UNICODE_VERTICAL_FORMS						= 145, // [FE10]

	// New blocks in Unicode 5.0
	B_UNICODE_NKO									= 146, // [07C0]
	B_UNICODE_BALINESE								= 147, // [1B00]
	B_UNICODE_LATIN_EXTENDED_C						= 148, // [2C60]
	B_UNICODE_LATIN_EXTENDED_D						= 149, // [A720]
	B_UNICODE_PHAGS_PA								= 150, // [A840]
	B_UNICODE_PHOENICIAN							= 151, // [10900]
	B_UNICODE_CUNEIFORM								= 152, // [12000]
	B_UNICODE_CUNEIFORM_NUMBERS_AND_PUNCTUATION		= 153, // [12400]
	B_UNICODE_COUNTING_ROD_NUMERALS					= 154, // [1D360]

	//  New blocks in Unicode 5.1
	B_UNICODE_SUNDANESE								= 155, // [1B80]
	B_UNICODE_LEPCHA								= 156, // [1C00]
	B_UNICODE_OL_CHIKI								= 157, // [1C50]
	B_UNICODE_CYRILLIC_EXTENDED_A					= 158, // [2DE0]
	B_UNICODE_VAI									= 159, // [A500]
	B_UNICODE_CYRILLIC_EXTENDED_B					= 160, // [A640]
	B_UNICODE_SAURASHTRA							= 161, // [A880]
	B_UNICODE_KAYAH_LI								= 162, // [A900]
	B_UNICODE_REJANG								= 163, // [A930]
	B_UNICODE_CHAM									= 164, // [AA00]
	B_UNICODE_ANCIENT_SYMBOLS						= 165, // [10190]
	B_UNICODE_PHAISTOS_DISC							= 166, // [101D0]
	B_UNICODE_LYCIAN								= 167, // [10280]
	B_UNICODE_CARIAN								= 168, // [102A0]
	B_UNICODE_LYDIAN								= 169, // [10920]
	B_UNICODE_MAHJONG_TILES							= 170, // [1F000]
	B_UNICODE_DOMINO_TILES							= 171, // [1F030]

	//  New blocks in Unicode 5.2
	B_UNICODE_SAMARITAN								= 172, // [0800]
	B_UNICODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED = 173, // [18B0]
	B_UNICODE_TAI_THAM								= 174, // [1A20]
	B_UNICODE_VEDIC_EXTENSIONS						= 175, // [1CD0]
	B_UNICODE_LISU									= 176, // [A4D0]
	B_UNICODE_BAMUM									= 177, // [A6A0]
	B_UNICODE_COMMON_INDIC_NUMBER_FORMS				= 178, // [A830]
	B_UNICODE_DEVANAGARI_EXTENDED					= 179, // [A8E0]
	B_UNICODE_HANGUL_JAMO_EXTENDED_A				= 180, // [A960]
	B_UNICODE_JAVANESE								= 181, // [A980]
	B_UNICODE_MYANMAR_EXTENDED_A					= 182, // [AA60]
	B_UNICODE_TAI_VIET								= 183, // [AA80]
	B_UNICODE_MEETEI_MAYEK							= 184, // [ABC0]
	B_UNICODE_HANGUL_JAMO_EXTENDED_B				= 185, // [D7B0]
	B_UNICODE_IMPERIAL_ARAMAIC						= 186, // [10840]
	B_UNICODE_OLD_SOUTH_ARABIAN						= 187, // [10A60]
	B_UNICODE_AVESTAN								= 188, // [10B00]
	B_UNICODE_INSCRIPTIONAL_PARTHIAN				= 189, // [10B40]
	B_UNICODE_INSCRIPTIONAL_PAHLAVI					= 190, // [10B60]
	B_UNICODE_OLD_TURKIC							= 191, // [10C00]
	B_UNICODE_RUMI_NUMERAL_SYMBOLS					= 192, // [10E60]
	B_UNICODE_KAITHI								= 193, // [11080]
	B_UNICODE_EGYPTIAN_HIEROGLYPHS					= 194, // [13000]
	B_UNICODE_ENCLOSED_ALPHANUMERIC_SUPPLEMENT		= 195, // [1F100]
	B_UNICODE_ENCLOSED_IDEOGRAPHIC_SUPPLEMENT		= 196, // [1F200]
	B_UNICODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C	= 197, // [2A700]

	//  New blocks in Unicode 6.0
	B_UNICODE_MANDAIC								= 198, // [0840]
	B_UNICODE_BATAK									= 199, // [1BC0]
	B_UNICODE_ETHIOPIC_EXTENDED_A					= 200, // [AB00]
	B_UNICODE_BRAHMI								= 201, // [11000]
	B_UNICODE_BAMUM_SUPPLEMENT						= 202, // [16800]
	B_UNICODE_KANA_SUPPLEMENT						= 203, // [1B000]
	B_UNICODE_PLAYING_CARDS							= 204, // [1F0A0]
	B_UNICODE_MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS	= 205, // [1F300]
	B_UNICODE_EMOTICONS								= 206, // [1F600]
	B_UNICODE_TRANSPORT_AND_MAP_SYMBOLS				= 207, // [1F680]
	B_UNICODE_ALCHEMICAL_SYMBOLS					= 208, // [1F700]
	B_UNICODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D	= 209, // [2B740]

	B_UNICODE_SCRIPT_COUNT							= 210,
	B_UNICODE_NO_SCRIPT = B_UNICODE_SCRIPT_COUNT,

	B_UNICODE_INVALID_CODE							= -1
};


// East Asian Width constants.

enum unicode_east_asian_width
{
	B_UNICODE_EA_NEUTRAL,   // [N]
	B_UNICODE_EA_AMBIGUOUS, // [A]
	B_UNICODE_EA_HALFWIDTH, // [H]
	B_UNICODE_EA_FULLWIDTH, // [F]
	B_UNICODE_EA_NARROW,	// [Na]
	B_UNICODE_EA_WIDE,		// [W]
	B_UNICODE_EA_COUNT
};


class BUnicodeChar {
	public:
		static bool IsAlpha(uint32_t c);
		static bool IsAlNum(uint32_t c);
		static bool IsDigit(uint32_t c);
		static bool IsHexDigit(uint32_t c);
		static bool IsUpper(uint32_t c);
		static bool IsLower(uint32_t c);
		static bool IsSpace(uint32_t c);
		static bool IsWhitespace(uint32_t c);
		static bool IsControl(uint32_t c);
		static bool IsPunctuation(uint32_t c);
		static bool IsPrintable(uint32_t c);
		static bool IsTitle(uint32_t c);
		static bool IsDefined(uint32_t c);
		static bool IsBase(uint32_t c);

		static int8 Type(uint32_t c);

		static uint32_t ToLower(uint32_t c);
		static uint32_t ToUpper(uint32_t c);
		static uint32_t ToTitle(uint32_t c);
		static int32_t DigitValue(uint32_t c);
		static unicode_east_asian_width EastAsianWidth(uint32_t c);

		static void ToUTF8(uint32_t c, char** out);
		static uint32_t FromUTF8(const char** in);
		static uint32_t FromUTF8(const char* in);

		static size_t UTF8StringLength(const char* string);
		static size_t UTF8StringLength(const char* string, size_t maxLength);

	private:
		BUnicodeChar();
};


inline uint32_t BUnicodeChar::FromUTF8(const char* in)
{
	const char* string = in;
	return FromUTF8(&string);
}


#endif	//  _UNICODE_CHAR_H_
