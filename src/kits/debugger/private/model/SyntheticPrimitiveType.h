/*
 * Copyright 2014, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef SYNTHETIC_PRIMITIVE_TYPE_H
#define SYNTHETIC_PRIMITIVE_TYPE_H


#include <Type.h>

#include <kits/support/String.h>


class SyntheticPrimitiveType : public PrimitiveType {
public:
								SyntheticPrimitiveType(uint32_t typeConstant);
	virtual						~SyntheticPrimitiveType();

	virtual	uint32_t				TypeConstant() const;

	virtual	image_id			ImageID() const;
	virtual	const BString&		ID() const;
	virtual	const BString&		Name() const;
	virtual	type_kind			Kind() const;
	virtual	target_size_t		ByteSize() const;

	virtual	status_t			ResolveObjectDataLocation(
									const ValueLocation& objectLocation,
									ValueLocation*& _location);
	virtual	status_t			ResolveObjectDataLocation(
									target_addr_t objectAddress,
									ValueLocation*& _location);

private:
	void						_Init();

private:
	uint32_t						fTypeConstant;
	BString						fID;
	BString						fName;
};


#endif // SYNTHETIC_PRIMITIVE_TYPE
