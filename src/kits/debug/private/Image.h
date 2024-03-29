﻿/*
 * Copyright 2005-2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */

#ifndef BEOS_DEBUG_IMAGE_H
#define BEOS_DEBUG_IMAGE_H

#include <kits/kernel/OS.h>
#include <util/DoublyLinkedList.h>
#include <elf_private.h>

#ifdef __cplusplus
namespace bhapi {
struct image_t;
struct runtime_loader_debug_area;
}
#endif
#ifdef __cplusplus
namespace BPrivate { namespace Debug {
class Image : public DoublyLinkedListLinkImpl<Image> {
public:
                                Image();
    virtual					~Image();

            const bhapi::image_info&	Info() const		{ return fInfo; }
            bhapi::image_id			ID() const			{ return fInfo.id; }
            const char*			Name() const		{ return fInfo.name; }
            addr_t				TextAddress() const
                { return (addr_t)fInfo.text; }
            size_t				TextSize() const	{ return fInfo.text_size; }

    virtual	const elf_sym*		LookupSymbol(addr_t address,
                                    addr_t* _baseAddress,
                                    const char** _symbolName,
                                    size_t *_symbolNameLen,
                                    bool *_exactMatch) const = 0;
    virtual	status_t			NextSymbol(int32_t& iterator,
                                    const char** _symbolName,
                                    size_t* _symbolNameLen,
                                    addr_t* _symbolAddress, size_t* _symbolSize,
                                    int32_t* _symbolType) const = 0;
    virtual	status_t			GetSymbol(const char* name, int32_t symbolType,
                                    void** _symbolLocation, size_t* _symbolSize,
                                    int32_t* _symbolType) const;

protected:
            bhapi::image_info			fInfo;
};

class SymbolTableBasedImage : public Image {
public:
                                SymbolTableBasedImage();
    virtual						~SymbolTableBasedImage();

    virtual	const elf_sym*		LookupSymbol(addr_t address,
                                    addr_t* _baseAddress,
                                    const char** _symbolName,
                                    size_t *_symbolNameLen,
                                    bool *_exactMatch) const;
    virtual	status_t			NextSymbol(int32_t& iterator,
                                    const char** _symbolName,
                                    size_t* _symbolNameLen,
                                    addr_t* _symbolAddress, size_t* _symbolSize,
                                    int32_t* _symbolType) const;

protected:
            size_t				_SymbolNameLen(const char* symbolName) const;

protected:
            addr_t				fLoadDelta;
            elf_sym*			fSymbolTable;
            char*				fStringTable;
            int32_t				fSymbolCount;
            size_t				fStringTableSize;
};


class ImageFile : public SymbolTableBasedImage {
public:
                                ImageFile();
    virtual						~ImageFile();

            status_t			Init(const bhapi::image_info& info);
            status_t			Init(const char* path);

private:
            status_t			_LoadFile(const char* path,
                                    addr_t* _textAddress, size_t* _textSize,
                                    addr_t* _dataAddress, size_t* _dataSize);

            status_t			_FindTableInSection(elf_ehdr* elfHeader,
                                    uint16 sectionType);

private:
            int					fFD;
            off_t				fFileSize;
            uint8*				fMappedFile;
};


class KernelImage : public SymbolTableBasedImage {
public:
                                KernelImage();
    virtual						~KernelImage();

            status_t			Init(const bhapi::image_info& info);
};


class CommPageImage : public SymbolTableBasedImage {
public:
                                CommPageImage();
    virtual						~CommPageImage();

            status_t			Init(const bhapi::image_info& info);
};

}	// namespace Debug
}	// namespace BPrivate
using BPrivate::Debug::ImageFile;
#endif  // C++
#endif	// BEOS_DEBUG_IMAGE_H
