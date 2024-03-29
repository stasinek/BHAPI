/*
 * Copyright 2007-2012, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ARGUMENT_VECTOR_H
#define _ARGUMENT_VECTOR_H


#include <Haiku.h>


namespace BPrivate {


class ArgumentVector {
public:
	enum ParseError {
		NO_ERROR,
		NO_MEMORY,
		UNTERMINATED_QUOTED_STRING,
		TRAILING_BACKSPACE
	};

public:
								ArgumentVector();
								~ArgumentVector();

			int32_t				ArgumentCount() const	{ return fCount; }
			const char* const*	Arguments() const		{ return fArguments; }

			char**				DetachArguments();
				// Caller must free() -- it's all one big allocation at the
				// returned pointer.

			ParseError			Parse(const char* commandLine,
									const char** _errorLocation = NULL);

private:
			struct Parser;

private:
			char**				fArguments;
			int32_t				fCount;
};


} // namespace BPrivate


using BPrivate::ArgumentVector;


#endif	// _ARGUMENT_VECTOR_H
