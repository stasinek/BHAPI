/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef OBJECT_ID_H
#define OBJECT_ID_H


#include <Referenceable.h>


class ObjectID : public BReferenceable {
public:
								ObjectID();
	virtual						~ObjectID();

	inline	uint32_t				HashValue() const;

	virtual	bool				operator==(const ObjectID& other) const = 0;
	inline	bool				operator!=(const ObjectID& other) const;

protected:
	virtual	uint32_t				ComputeHashValue() const = 0;

protected:
	mutable	uint32_t				fHashValue;
};


uint32_t ObjectID::HashValue() const
{
	if (fHashValue == 0)
		fHashValue = ComputeHashValue();
	return fHashValue;
}


bool ObjectID::operator!=(const ObjectID& other) const
{
	return !(*this == other);
}


#endif	// OBJECT_ID_H
