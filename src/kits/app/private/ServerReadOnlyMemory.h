/*
 * Copyright 2006, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Axel Dörfler, axeld@pinc-software.de
 */
#ifndef SERVER_READ_ONLY_MEMORY_H
#define SERVER_READ_ONLY_MEMORY_H


#include <kits/netGraphicsDefs.h>
#include <kits/netInterfaceDefs.h>


// Update this constant with the largest color constant excluding
// B_SUCCESS_COLOR and B_FAILURE_COLOR.
// If you add a constant with index greater than 100 you'll have to add
// to the second operand.
static const int32_t kColorWhichCount = B_SCROLL_BAR_THUMB_COLOR + 3;


struct server_read_only_memory {
	rgb_color	colors[kColorWhichCount];
};


static inline int32_t color_which_to_index(color_which which)
{
	if (which <= kColorWhichCount - 3)
		return which - 1;
	if (which >= B_SUCCESS_COLOR && which <= B_FAILURE_COLOR)
		return which - B_SUCCESS_COLOR + kColorWhichCount - 3;

	return -1;
}


static inline color_which
index_to_color_which(int32_t index)
{
	if (index >= 0 && index < kColorWhichCount) {
		if ((color_which)index < kColorWhichCount - 3)
			return (color_which)(index + 1);
		else {
			return (color_which)(index + B_SUCCESS_COLOR
				- kColorWhichCount - 3);
		}
	}

	return (color_which)-1;
}

#endif	/* SERVER_READ_ONLY_MEMORY_H */
