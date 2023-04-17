/*
 * Copyright 2014, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef LINE_DATA_SOURCE_H
#define LINE_DATA_SOURCE_H


#include <Referenceable.h>


class LineDataSource : public BReferenceable {
public:
	virtual						~LineDataSource();

	virtual	int32_t				CountLines() const = 0;
	virtual	const char*			LineAt(int32_t index) const = 0;
	virtual int32_t				LineLengthAt(int32_t index) const = 0;
};


#endif	// LINE_DATA_SOURCE_H
