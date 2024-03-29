/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Copyright 2010-2013, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef IMAGE_DEBUG_INFO_H
#define IMAGE_DEBUG_INFO_H


#include <kits/support/String.h>

#include <ObjectList.h>
#include <Referenceable.h>

#include <AddressSectionTypes.h>
#include <ImageInfo.h>
#include <Types.h>


class Architecture;
class DebuggerInterface;
class FileSourceCode;
class FunctionDebugInfo;
class FunctionInstance;
class GlobalTypeCache;
class LocatableFile;
class SpecificImageDebugInfo;
class SymbolInfo;
class Type;
class TypeLookupConstraints;


class ImageDebugInfo : public BReferenceable {
public:
								ImageDebugInfo(const ImageInfo& imageInfo);
								~ImageDebugInfo();

			const ImageInfo&	GetImageInfo() const	{ return fImageInfo; }

			bool				AddSpecificInfo(SpecificImageDebugInfo* info);
			status_t			FinishInit(DebuggerInterface* interface);

			status_t			GetType(GlobalTypeCache* cache,
									const BString& name,
									const TypeLookupConstraints& constraints,
									Type*& _type);
									// returns a reference

			bool				HasType(const BString& name,
									const TypeLookupConstraints& constraints)
									const;

			AddressSectionType	GetAddressSectionType(target_addr_t address)
									const;

			int32_t				CountFunctions() const;
			FunctionInstance*	FunctionAt(int32_t index) const;
			FunctionInstance*	FunctionAtAddress(target_addr_t address) const;
			FunctionInstance*	FunctionByName(const char* name) const;

			FunctionInstance*	MainFunction() const
									{ return fMainFunction; }

			status_t			AddSourceCodeInfo(LocatableFile* file,
									FileSourceCode* sourceCode) const;

private:
			typedef BObjectList<SpecificImageDebugInfo> SpecificInfoList;
			typedef BObjectList<FunctionInstance> FunctionList;

private:
	static	int					_CompareFunctions(const FunctionInstance* a,
									const FunctionInstance* b);
	static	int					_CompareAddressFunction(
									const target_addr_t* address,
									const FunctionInstance* function);
	static	int					_CompareSymbols(const SymbolInfo* a,
									const SymbolInfo* b);

private:
			ImageInfo			fImageInfo;
			SpecificInfoList	fSpecificInfos;
			FunctionList		fFunctions;
			FunctionInstance*	fMainFunction;
};


#endif	// IMAGE_DEBUG_INFO_H
