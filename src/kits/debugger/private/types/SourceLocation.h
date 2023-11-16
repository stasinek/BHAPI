/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef SOURCE_LOCATION_H
#define SOURCE_LOCATION_H

#include <Haiku.h>


class SourceLocation {
public:
	SourceLocation(int32_t line = 0, int32_t column = 0)
		:
		fLine(line),
		fColumn(column)
	{
	}

	SourceLocation(const SourceLocation& other)
		:
		fLine(other.fLine),
		fColumn(other.fColumn)
	{
	}

	SourceLocation& operator=(const SourceLocation& other)
	{
		fLine = other.fLine;
		fColumn = other.fColumn;
		return *this;
	}

	bool operator==(const SourceLocation& other) const
	{
		return fLine == other.fLine && fColumn == other.fColumn;
	}

	bool operator!=(const SourceLocation& other) const
	{
		return !(*this == other);
	}

	bool operator<(const SourceLocation& other) const
	{
		return fLine < other.fLine
			|| (fLine == other.fLine && fColumn < other.fColumn);
	}

	bool operator<=(const SourceLocation& other) const
	{
		return fLine < other.fLine
			|| (fLine == other.fLine && fColumn <= other.fColumn);
	}

	int32_t Line() const
	{
		return fLine;
	}

	int32_t Column() const
	{
		return fColumn;
	}

private:
	int32_t	fLine;
	int32_t	fColumn;
};


#endif	// SOURCE_LOCATION_H
