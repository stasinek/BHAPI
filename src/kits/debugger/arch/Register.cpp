/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Copyright 2014, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */


#include <Register.h>

#include <HaikuConstants.h>


Register::Register(int32_t index, const char* name, uint32_t bitSize,
	uint32_t valueType, register_type type, bool calleePreserved)
	:
	fIndex(index),
	fName(name),
	fBitSize(bitSize),
	fValueType(valueType),
	fType(type),
	fCalleePreserved(calleePreserved)
{
	switch (fValueType) {
		case B_INT8_TYPE:
		case B_UINT8_TYPE:
		case B_INT16_TYPE:
		case B_UINT16_TYPE:
		case B_INT32_TYPE:
		case B_UINT32_TYPE:
		case B_INT64_TYPE:
		case B_UINT64_TYPE:
			fFormat = REGISTER_FORMAT_INTEGER;
			break;
		case B_FLOAT_TYPE:
		case B_DOUBLE_TYPE:
			fFormat = REGISTER_FORMAT_FLOAT;
			break;
		case B_RAW_TYPE:
			fFormat = REGISTER_FORMAT_SIMD;
			break;
		default:
			fFormat = REGISTER_FORMAT_INTEGER;
			break;
	}
}


Register::Register(const Register& other)
	:
	fIndex(other.fIndex),
	fName(other.fName),
	fBitSize(other.fBitSize),
	fValueType(other.fValueType),
	fFormat(other.fFormat),
	fType(other.fType),
	fCalleePreserved(other.fCalleePreserved)
{
}
