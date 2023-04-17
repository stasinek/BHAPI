/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Copyright 2014, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef REGISTER_H
#define REGISTER_H


#include <Haiku.h>


enum register_format {
	REGISTER_FORMAT_INTEGER,
	REGISTER_FORMAT_FLOAT,
	REGISTER_FORMAT_SIMD
};

enum register_type {
	REGISTER_TYPE_INSTRUCTION_POINTER,
	REGISTER_TYPE_STACK_POINTER,
	REGISTER_TYPE_RETURN_ADDRESS,
	REGISTER_TYPE_GENERAL_PURPOSE,
	REGISTER_TYPE_SPECIAL_PURPOSE,
	REGISTER_TYPE_EXTENDED
};


class Register {
public:
								Register(int32_t index, const char* name,
									uint32_t bitSize, uint32_t valueType,
									register_type type, bool calleePreserved);
										// name will not be cloned
								Register(const Register& other);

			int32_t				Index() const		{ return fIndex; }
			const char*			Name() const		{ return fName; }
			uint32_t				ValueType() const	{ return fValueType; }
			register_format		Format() const		{ return fFormat; }
			uint32_t				BitSize() const		{ return fBitSize; }
			register_type		Type() const		{ return fType; }
			bool				IsCalleePreserved() const
									{ return fCalleePreserved; }

private:
			int32_t				fIndex;
			const char*			fName;
			uint32_t				fBitSize;
			uint32_t				fValueType;
			register_format		fFormat;
			register_type		fType;
			bool				fCalleePreserved;

};


#endif	// REGISTER_H
