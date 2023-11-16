/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef REGISTER_MAP_H
#define REGISTER_MAP_H


#include <Referenceable.h>


class RegisterMap : public BReferenceable {
public:
	virtual						~RegisterMap();

	virtual	int32_t				CountRegisters() const = 0;
	virtual	int32_t				MapRegisterIndex(int32_t index) const = 0;
};


#endif	// REGISTER_MAP_H
