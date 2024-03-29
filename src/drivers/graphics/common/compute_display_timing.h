/*
 * Copyright 2011, Axel Dörfler, axeld@pinc-software.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef _COMPUTE_DISPLAY_TIMING_H
#define _COMPUTE_DISPLAY_TIMING_H


#include <kits/netAccelerant.h>
#include <kits/netGraphicsDefs.h>


#ifdef __cplusplus
extern "C" {
#endif

status_t compute_display_timing(uint32_t width, uint32_t height, float refresh,
	bool interlaced, display_timing* timing);

#ifdef __cplusplus
}
#endif


#endif	/* _COMPUTE_DISPLAY_TIMING_H */
