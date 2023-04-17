/*
 * Copyright 2007-2011, Axel Dörfler, axeld@pinc-software.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CREATE_DISPLAY_MODES_H
#define _CREATE_DISPLAY_MODES_H


#include <kits/netedid.h>

#include <kits/netAccelerant.h>
#include <kits/netGraphicsDefs.h>


typedef bool (*check_display_mode_hook)(display_mode* mode);

#ifdef __cplusplus
extern "C" {
#endif

area_id create_display_modes(const char* name, edid1_info* edid,
	const display_mode* initialModes, uint32_t initialModeCount,
	const color_space* spaces, uint32_t spacesCount,
	check_display_mode_hook hook, display_mode** _modes, uint32_t* _count);

void fill_display_mode(uint32_t width, uint32_t height, display_mode* mode);

#ifdef __cplusplus
}
#endif

#endif	/* _CREATE_DISPLAY_MODES_H */
