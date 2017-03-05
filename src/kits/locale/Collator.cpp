/*
* Copyright 2003, Axel Dörfler, axeld@pinc-software.de. All rights reserved.
* Copyright 2010, Adrien Destugues <pulkomandy@pulkomandy.ath.cx>
* Distributed under the terms of the MIT License.
*/


#include <unicode/uversion.h>
#include <Collator.h>

#include <ctype.h>
#include <stdlib.h>

#include <new>
#include <typeinfo>

#include <UnicodeChar.h>
#include <String.h>
#include <Message.h>

#include <unicode/coll.h>
#include <unicode/tblcoll.h>


BCollator::BCollator()
	:
	fDefaultStrength(B_COLLATE_PRIMARY),
	fIgnorePunctuation(true)
{
	// TODO: the collator construction will have to change; the default
	//	collator should be constructed by the Locale/LocaleRoster, so we
	//	only need a constructor where you specify all details

	UErrorCode error = U_ZERO_ERROR;
	fICUCollator = Collator::createInstance(error);
}


BCollator::BCollator(const char* locale, int8 strength, bool ignorePunctuation)
	:
	fDefaultStrength(strength),
	fIgnorePunctuation(ignorePunctuation)
{
	UErrorCode error = U_ZERO_ERROR;
	fICUCollator = Collator::createInstance(locale, error);
}


BCollator::BCollator(BMessage* archive)
	:
	BArchivable(archive),
	fICUCollator(NULL),
	fDefaultStrength(B_COLLATE_PRIMARY),
	fIgnorePunctuation(true)
{
	int32 data;
	if (archive->FindInt32("loc:strength", &data) == B_OK)
		fDefaultStrength = (uint8)data;
	else
		fDefaultStrength = B_COLLATE_PRIMARY;

	archive->FindBool("loc:punctuation", &fIgnorePunctuation);

	UErrorCode error = U_ZERO_ERROR;
	RuleBasedCollator* fallbackICUCollator
		= static_cast<RuleBasedCollator*>(Collator::createInstance(error));

	ssize_t size;
	const void* buffer = NULL;
	if (archive->FindData("loc:collator", B_RAW_TYPE, &buffer, &size) == B_OK) {
		fICUCollator = new RuleBasedCollator((const uint8_t*)buffer, (int)size,
			fallbackICUCollator, error);
		if (fICUCollator == NULL) {
			fICUCollator = fallbackICUCollator;
				// Unarchiving failed, so we revert to the fallback collator
				// TODO: when can this happen, can it be avoided?
		}
	}
}


BCollator::BCollator(const BCollator& other)
	:
	fICUCollator(NULL)
{
	*this = other;
}


BCollator::~BCollator()
{
	delete fICUCollator;
}


BCollator& BCollator::operator=(const BCollator& source)
{
	if (&source != this) {
		delete fICUCollator;

		fICUCollator = source.fICUCollator != NULL
			? source.fICUCollator->clone()
			: NULL;
		fDefaultStrength = source.fDefaultStrength;
		fIgnorePunctuation = source.fIgnorePunctuation;
	}

	return *this;
}


void
BCollator::SetDefaultStrength(int8 strength)
{
	fDefaultStrength = strength;
}


int8
BCollator::DefaultStrength() const
{
	return fDefaultStrength;
}


void
BCollator::SetIgnorePunctuation(bool ignore)
{
	fIgnorePunctuation = ignore;
}


bool
BCollator::IgnorePunctuation() const
{
	return fIgnorePunctuation;
}


status_t
BCollator::GetSortKey(const char* string, BString* key, int8 strength) const
{
	_SetStrength(strength);

	// TODO : handle fIgnorePunctuation

	int length = strlen(string);

	uint8_t* buffer = (uint8_t*)malloc(length * 2);
		// According to ICU documentation this should be enough in "most cases"
	if (buffer == NULL)
		return B_NO_MEMORY;

	UErrorCode error = U_ZERO_ERROR;
	int requiredSize = fICUCollator->getSortKey(UnicodeString(string, length,
		NULL, error), buffer, length * 2);
	if (requiredSize > length * 2) {
		buffer = (uint8_t*)realloc(buffer, requiredSize);
		if (buffer == NULL)
			return B_NO_MEMORY;

		error = U_ZERO_ERROR;
		fICUCollator->getSortKey(UnicodeString(string, length, NULL, error),
			buffer,	requiredSize);
	}

	key->SetTo((char*)buffer);
	free(buffer);

	if (error == U_ZERO_ERROR)
		return B_OK;

	return B_ERROR;
}


int
BCollator::Compare(const char* s1, const char* s2, int8 strength) const
{
	_SetStrength(strength);

	// TODO : handle fIgnorePunctuation

	UErrorCode error = U_ZERO_ERROR;
	return fICUCollator->compare(s1, s2, error);
}


status_t
BCollator::Archive(BMessage* archive, bool deep) const
{
	status_t status = BArchivable::Archive(archive, deep);
	if (status < B_OK)
		return status;

	if (status == B_OK)
		status = archive->AddInt32("loc:strength", fDefaultStrength);
	if (status == B_OK)
		status = archive->AddBool("loc:punctuation", fIgnorePunctuation);

	UErrorCode error = U_ZERO_ERROR;
	int size = static_cast<RuleBasedCollator*>(fICUCollator)->cloneBinary(NULL,
		0, error);
		// This WILL fail with U_BUFFER_OVERFLOW_ERROR. But we get the needed
		// size.
	error = U_ZERO_ERROR;
	uint8_t* buffer = (uint8_t*)malloc(size);
	static_cast<RuleBasedCollator*>(fICUCollator)->cloneBinary(buffer, size,
		error);

	if (status == B_OK && error == U_ZERO_ERROR)
		status = archive->AddData("loc:collator", B_RAW_TYPE, buffer, size);
	free(buffer);

	if (error == U_ZERO_ERROR)
		return status;
	return B_ERROR;
}


BArchivable*
BCollator::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "BCollator"))
		return new(std::nothrow) BCollator(archive);

	return NULL;
}


status_t
BCollator::_SetStrength(int8 strength) const
{
	if (strength == B_COLLATE_DEFAULT)
		strength = fDefaultStrength;

	Collator::ECollationStrength icuStrength;
	switch (strength) {
		case B_COLLATE_PRIMARY:
			icuStrength = Collator::PRIMARY;
			break;
		case B_COLLATE_SECONDARY:
			icuStrength = Collator::SECONDARY;
			break;
		case B_COLLATE_TERTIARY:
		default:
			icuStrength = Collator::TERTIARY;
			break;
		case B_COLLATE_QUATERNARY:
			icuStrength = Collator::QUATERNARY;
			break;
		case B_COLLATE_IDENTICAL:
			icuStrength = Collator::IDENTICAL;
			break;
	}
	fICUCollator->setStrength(icuStrength);

	return B_OK;
}
