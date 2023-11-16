/*
 * Copyright 2004-2007, Ingo Weinhold, bonefish@users.sf.net. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef HASH_STRING_H
#define HASH_STRING_H

#include <Haiku.h>

// string_hash
//
// from the Dragon Book: a slightly modified hashpjw()
static inline
uint32_t string_hash(const char *name)
{
	uint32_t h = 0;
	if (name) {
		for (; *name; name++) {
			uint32_t g = h & 0xf0000000;
			if (g)
				h ^= g >> 24;
			h = (h << 4) + *name;
		}
	}
	return h;
}

#ifdef __cplusplus

namespace BPrivate {

// HashString
class HashString {
public:
	HashString();
	HashString(const HashString &string);
	HashString(const char *string, int32_t length = -1);
	~HashString();

	bool SetTo(const char *string, int32_t maxLength = -1);
	void Unset();

	void Truncate(int32_t newLength);

	const char *GetString() const;
	int32_t GetLength() const	{ return fLength; }

	uint32_t GetHashCode() const	{ return string_hash(GetString()); }

	HashString &operator=(const HashString &string);
	bool operator==(const HashString &string) const;
	bool operator!=(const HashString &string) const { return !(*this == string); }

private:
	bool _SetTo(const char *string, int32_t length);

private:
	int32_t	fLength;
	char	*fString;
};

}	// namespace BPrivate

using BPrivate::HashString;

#endif	// __cplusplus

#endif	// HASH_STRING_H
