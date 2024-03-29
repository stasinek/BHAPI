/*
 * Copyright 2005, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Axel Dörfler, axeld@pinc-software.de
 */
#ifndef SHARED_CURSOR_AREA_H
#define SHARED_CURSOR_AREA_H


#include <Haiku.h>


struct shared_cursor {
	uint32_t	pos;
	int32_t	read;
};


#endif	/* SHARED_CURSOR_AREA_H */
