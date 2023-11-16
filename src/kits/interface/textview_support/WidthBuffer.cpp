/*
 * Copyright 2003-2008, Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stefano Ceccherini (stefano.ceccherini@gmail.com)
 */

//!	Caches string widths in a hash table, to avoid a trip to the app server.


#include <utf8_functions.h>
#include <TextGapBuffer.h>
#include <WidthBuffer.h>

#include <Autolock.h>
#include <kits/debug/Debug.h>
#include <Font.h>
#include <Locker.h>




//! NetPositive binary compatibility support
class _BWidthBuffer_;


namespace BPrivate {


const static uint32_t kTableCount = 128;
const static uint32_t kInvalidCode = 0xFFFFFFFF;
WidthBuffer* gWidthBuffer = NULL;
	// initialized in InterfaceDefs.cpp


struct hashed_escapement {
	uint32_t code;
	float escapement;

	hashed_escapement()
	{
		code = kInvalidCode;
		escapement = 0;
	}
};


/*! \brief Convert a UTF8 char to a code, which will be used
		to uniquely identify the character in the hash table.
	\param text A pointer to the character to examine.
	\param charLen the length of the character to examine.
	\return The code for the given character,
*/
static inline uint32_t CharToCode(const char* text, const int32_t charLen)
{
	uint32_t value = 0;
	int32_t shiftVal = 24;
	for (int32_t c = 0; c < charLen; c++) {
		value |= ((unsigned char)text[c] << shiftVal);
		shiftVal -= 8;
	}
	return value;
}


/*! \brief Initializes the object.
*/
WidthBuffer::WidthBuffer()
	:
	_BTextViewSupportBuffer_<_width_table_>(1, 0),
	fLock("width buffer")
{
}


/*! \brief Frees the allocated resources.
*/
WidthBuffer::~WidthBuffer()
{
	for (int32_t x = 0; x < fItemCount; x++)
		delete[] (hashed_escapement*)fBuffer[x].widths;
}


/*! \brief Returns how much room is required to draw a string in the font.
	\param inText The string to be examined.
	\param fromOffset The offset in the string where to begin the examination.
	\param lenght The amount of bytes to be examined.
	\param inStyle The font.
	\return The space (in pixels) required to draw the given string.
*/
float
WidthBuffer::StringWidth(const char* inText, int32_t fromOffset, int32_t length,
	const BFont* inStyle)
{
	if (inText == NULL || length <= 0)
		return 0;

	BAutolock _(fLock);

	int32_t index = 0;
	if (!FindTable(inStyle, &index))
		index = InsertTable(inStyle);

	char* text = NULL;
	int32_t numChars = 0;
	int32_t textLen = 0;

	const char* sourceText = inText + fromOffset;
	const float fontSize = inStyle->Size();
	float stringWidth = 0;

	for (int32_t charLen = 0; length > 0;
			sourceText += charLen, length -= charLen) {
		charLen = UTF8NextCharLen(sourceText, length);

		// End of string, bail out
		if (charLen <= 0)
			break;

		// Some magic, to uniquely identify this character
		const uint32_t value = CharToCode(sourceText, charLen);

		float escapement;
		if (GetEscapement(value, index, &escapement)) {
			// Well, we've got a match for this character
			stringWidth += escapement;
		} else {
			// Store this character into an array, which we'll
			// pass to HashEscapements() later
			int32_t offset = textLen;
			textLen += charLen;
			numChars++;
			char* newText = (char*)realloc(text, textLen + 1);
			if (newText == NULL) {
				free(text);
				return 0;
			}

			text = newText;
			memcpy(&text[offset], sourceText, charLen);
		}
	}

	if (text != NULL) {
		// We've found some characters which aren't yet in the hash table.
		// Get their width via HashEscapements()
		text[textLen] = 0;
		stringWidth += HashEscapements(text, numChars, textLen, index, inStyle);
		free(text);
	}

	return stringWidth * fontSize;
}


/*! \brief Returns how much room is required to draw a string in the font.
	\param inBuffer The TextGapBuffer to be examined.
	\param fromOffset The offset in the TextGapBuffer where to begin the
	examination.
	\param lenght The amount of bytes to be examined.
	\param inStyle The font.
	\return The space (in pixels) required to draw the given string.
*/
float
WidthBuffer::StringWidth(TextGapBuffer &inBuffer, int32_t fromOffset,
	int32_t length, const BFont* inStyle)
{
	const char* text = inBuffer.GetString(fromOffset, &length);
	return StringWidth(text, 0, length, inStyle);
}


/*! \brief Searches for the table for the given font.
	\param inStyle The font to search for.
	\param outIndex a pointer to an int32_t, where the function will store
	the index of the table, if found, or -1, if not.
	\return \c true if the function founds the table,
		\c false if not.
*/
bool WidthBuffer::FindTable(const BFont* inStyle, int32_t* outIndex)
{
	if (inStyle == NULL)
		return false;

	int32_t tableIndex = -1;

	for (int32_t i = 0; i < fItemCount; i++) {
		if (*inStyle == fBuffer[i].font) {
			tableIndex = i;
			break;
		}
	}
	if (outIndex != NULL)
		*outIndex = tableIndex;

	return tableIndex != -1;
}


/*!	\brief Creates and insert an empty table for the given font.
	\param font The font to create the table for.
	\return The index of the newly created table.
*/
int32_t WidthBuffer::InsertTable(const BFont* font)
{
	_width_table_ table;

	table.font = *font;
	table.hashCount = 0;
	table.tableCount = kTableCount;
	table.widths = new hashed_escapement[kTableCount];

	uint32_t position = fItemCount;
	InsertItemsAt(1, position, &table);

	return position;
}


/*! \brief Gets the escapement for the given character.
	\param value An integer which uniquely identifies a character.
	\param index The index of the table to search.
	\param escapement A pointer to a float, where the function will
	store the escapement.
	\return \c true if the function could find the escapement
		for the given character, \c false if not.
*/
bool WidthBuffer::GetEscapement(uint32_t value, int32_t index, float* escapement)
{
	const _width_table_ &table = fBuffer[index];
	const hashed_escapement* widths
		= static_cast<hashed_escapement*>(table.widths);
	uint32_t hashed = Hash(value) & (table.tableCount - 1);

	uint32_t found;
	while ((found = widths[hashed].code) != kInvalidCode) {
		if (found == value)
			break;

		if (++hashed >= (uint32_t)table.tableCount)
			hashed = 0;
	}

	if (found == kInvalidCode)
		return false;

	if (escapement != NULL)
		*escapement = widths[hashed].escapement;

	return true;
}


uint32_t WidthBuffer::Hash(uint32_t val)
{
	uint32_t shifted = val >> 24;
	uint32_t result = (val >> 15) + (shifted * 3);

	result ^= (val >> 6) - (shifted * 22);
	result ^= (val << 3);

	return result;
}


/*! \brief Gets the escapements for the given string, and put them into
	the hash table.
	\param inText The string to be examined.
	\param numChars The amount of characters contained in the string.
	\param textLen the amount of bytes contained in the string.
	\param tableIndex the index of the table where the escapements
		should be put.
	\param inStyle the font.
	\return The width of the supplied string (which should be multiplied by
		the size of the font).
*/
float
WidthBuffer::HashEscapements(const char* inText, int32_t numChars, int32_t textLen,
	int32_t tableIndex, const BFont* inStyle)
{
	ASSERT(inText != NULL);
	ASSERT(numChars > 0);
	ASSERT(textLen > 0);

	float* escapements = new float[numChars];
	inStyle->GetEscapements(inText, numChars, escapements);

	_width_table_ &table = fBuffer[tableIndex];
	hashed_escapement* widths = static_cast<hashed_escapement*>(table.widths);

	int32_t charCount = 0;
	char* text = (char*)inText;
	const char* textEnd = inText + textLen;
	// Insert the escapements into the hash table
	do {
		// Using this variant is safe as the handed in string is guaranteed to
		// be 0 terminated.
		const int32_t charLen = UTF8NextCharLen(text);
		if (charLen == 0)
			break;

		const uint32_t value = CharToCode(text, charLen);

		uint32_t hashed = Hash(value) & (table.tableCount - 1);
		uint32_t found;
		while ((found = widths[hashed].code) != kInvalidCode) {
			if (found == value)
				break;
			if (++hashed >= (uint32_t)table.tableCount)
				hashed = 0;
		}

		if (found == kInvalidCode) {
			// The value is not in the table. Add it.
			widths[hashed].code = value;
			widths[hashed].escapement = escapements[charCount];
			table.hashCount++;

			// We always keep some free space in the hash table:
			// we double the current size when hashCount is 2/3 of
			// the total size.
			if (table.tableCount * 2 / 3 <= table.hashCount) {
				const int32_t newSize = table.tableCount * 2;

				// Create and initialize a new hash table
				hashed_escapement* newWidths = new hashed_escapement[newSize];

				// Rehash the values, and put them into the new table
				for (uint32_t oldPos = 0; oldPos < (uint32_t)table.tableCount;
						oldPos++) {
					if (widths[oldPos].code != kInvalidCode) {
						uint32_t newPos
							= Hash(widths[oldPos].code) & (newSize - 1);
						while (newWidths[newPos].code != kInvalidCode) {
							if (++newPos >= (uint32_t)newSize)
								newPos = 0;
						}
						newWidths[newPos] = widths[oldPos];
					}
				}

				// Delete the old table, and put the new pointer into the
				// _width_table_
				delete[] widths;
				table.tableCount = newSize;
				table.widths = widths = newWidths;
			}
		}
		charCount++;
		text += charLen;
	} while (text < textEnd);

	// Calculate the width of the string
	float width = 0;
	for (int32_t x = 0; x < numChars; x++)
		width += escapements[x];

	delete[] escapements;

	return width;
}

} // namespace BPrivate


#if __GNUC__ < 3
//! NetPositive binary compatibility support

_BWidthBuffer_::_BWidthBuffer_()
{
}

_BWidthBuffer_::~_BWidthBuffer_()
{
}

_BWidthBuffer_* gCompatibilityWidthBuffer = NULL;

extern "C"
float
StringWidth__14_BWidthBuffer_PCcllPC5BFont(_BWidthBuffer_* widthBuffer,
	const char* inText, int32_t fromOffset, int32_t length, const BFont* inStyle)
{
	return BPrivate::gWidthBuffer->StringWidth(inText, fromOffset, length,
		inStyle);
}

#endif // __GNUC__ < 3

