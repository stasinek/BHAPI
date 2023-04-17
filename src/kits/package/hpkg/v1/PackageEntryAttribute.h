/*
 * Copyright 2009,2011, Haiku, Inc.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PACKAGE__HPKG__V1__PACKAGE_ENTRY_ATTRIBUTE_H_
#define _PACKAGE__HPKG__V1__PACKAGE_ENTRY_ATTRIBUTE_H_


#include <package/hpkg/v1/PackageData.h>


namespace BPackageKit {

namespace BHPKG {

namespace V1 {


class BPackageEntryAttribute {
public:
								BPackageEntryAttribute(const char* name);

			const char*			Name() const			{ return fName; }
			uint32_t				Type() const			{ return fType; }

			BPackageData&		Data()	{ return fData; }

			void				SetType(uint32_t type)	{ fType = type; }

private:
			const char*			fName;
			uint32_t				fType;
			BPackageData			fData;
};


}	// namespace V1

}	// namespace BHPKG

}	// namespace BPackageKit


#endif	// _PACKAGE__HPKG__V1__PACKAGE_ENTRY_ATTRIBUTE_H_
