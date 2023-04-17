/*
 * Copyright 2009-2010, Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#ifndef _PROPERTY_INFO_H
#define _PROPERTY_INFO_H

#ifdef __cplusplus
namespace bhapi {
#endif

struct compound_type {
    struct field_pair {
        const char*	name;
        type_code	type;
    };
    field_pair	pairs[5];
};

struct property_info {
    const char*		name;
    uint32_t			commands[10];
    uint32_t			specifiers[10];
    const char*		usage;
    uint32_t			extra_data;
    uint32_t			types[10];
    compound_type	ctypes[3];
    uint32_t			_reserved[10];
};

enum value_kind {
    B_COMMAND_KIND = 0,
    B_TYPE_CODE_KIND = 1
};

struct value_info {
    const char*		name;
    uint32_t			value;
    value_kind		kind;
    const char*		usage;
    uint32_t			extra_data;
    uint32_t			_reserved[10];
};
#ifdef __cplusplus
}
#endif

#ifndef BMESSAGE_I
#define BMESSAGE_I
class BMessage;
#endif
#include <kits/support/Flattenable.h>
#include <HaikuConstants.h>
#include <Haiku.h>
class BHAPI_IMPEXP BPropertyInfo : public BFlattenable {
public:
                                BPropertyInfo(property_info* prop = NULL,
                                    value_info* value = NULL,
                                    bool freeOnDelete = false);
    virtual						~BPropertyInfo();

    virtual	int32_t				FindMatch(BMessage* msg, int32_t index,
                                    BMessage* specifier, int32_t form,
                                    const char* prop, void* data = NULL) const;

    virtual	bool				IsFixedSize() const;
    virtual	type_code			TypeCode() const;
    virtual	ssize_t				FlattenedSize() const;
    virtual	status_t			Flatten(void* buffer, ssize_t size) const;
    virtual	bool				AllowsTypeCode(type_code code) const;
    virtual	status_t			Unflatten(type_code code, const void* buffer,
                                    ssize_t size);

        const property_info*	Properties() const;
        const value_info*		Values() const;
        int32_t					CountProperties() const;
        int32_t					CountValues() const;

        void					PrintToStream() const;

protected:
    static	bool				FindCommand(uint32_t what, int32_t index,
                                    property_info* info);
    static	bool				FindSpecifier(uint32_t form, property_info* info);

private:
    virtual	void				_ReservedPropertyInfo1();
    virtual	void				_ReservedPropertyInfo2();
    virtual	void				_ReservedPropertyInfo3();
    virtual	void				_ReservedPropertyInfo4();

                                BPropertyInfo(const BPropertyInfo& other);
            BPropertyInfo&		operator=(const BPropertyInfo& other);
            void				FreeMem();

            property_info*		fPropInfo;
            value_info*			fValueInfo;
            int32_t				fPropCount;
            bool				fInHeap;
            uint16				fValueCount;
            uint32_t				_reserved[4];
};


#endif	/* _PROPERTY_INFO_H */
