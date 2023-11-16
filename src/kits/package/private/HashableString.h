/*
 * Copyright 2011, Oliver Tappe <zooey@hirschkaefer.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef _PACKAGE__PRIVATE__HASHABLE_STRING_H_
#define _PACKAGE__PRIVATE__HASHABLE_STRING_H_


#include <kits/support/String.h>

#include <HashStringClass.h>


namespace BPackageKit {

namespace BPrivate {


class HashableString : public BString {
public:
	inline						HashableString();

	inline						HashableString(const BString& string);

	inline	uint32_t				GetHashCode() const;

	inline	bool				operator!= (const HashableString& other) const;

private:
			uint32_t				fHashCode;
};


inline
HashableString::HashableString()
	:
	fHashCode(0)
{
}


inline
HashableString::HashableString(const BString& string)
	:
	BString(string),
	fHashCode(string_hash(String()))
{
}


inline uint32_t HashableString::GetHashCode() const
{
	return fHashCode;
}


inline bool HashableString::operator!= (const HashableString& other) const
{
	return Compare(other) != 0 || fHashCode != other.fHashCode;
}


}	// namespace BPrivate

}	// namespace BPackageKit


#endif // _PACKAGE__PRIVATE__HASHABLE_STRING_H_
