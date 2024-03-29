/*
 * Copyright 2005-2009, Axel Dörfler, axeld@pinc-software.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef VESA_INFO_H
#define VESA_INFO_H


#include <kits/netDrivers.h>
#include <kits/netAccelerant.h>
#include <kits/netPCI.h>

#include <kits/netedid.h>


#define VESA_EDID_BOOT_INFO "vesa_edid/v1"
#define VESA_MODES_BOOT_INFO "vesa_modes/v1"

struct vesa_mode {
	uint16			mode;
	uint16			width;
	uint16			height;
	uint8			bits_per_pixel;
};

struct vesa_shared_info {
	int32_t			type;
	area_id			mode_list_area;		// area containing display mode list
	uint32_t			mode_count;
	display_mode	current_mode;
	uint32_t			bytes_per_row;

	area_id			frame_buffer_area;	// area of frame buffer
	uint8*			frame_buffer;
		// pointer to frame buffer (visible by all apps!)
	uint8*			physical_frame_buffer;

	uint32_t			vesa_mode_offset;
	uint32_t			vesa_mode_count;

	edid1_info		edid_info;
	bool			has_edid;
	uint32_t			dpms_capabilities;
};

//----------------- ioctl() interface ----------------

// list ioctls
enum {
	VESA_GET_PRIVATE_DATA = B_DEVICE_OP_CODES_END + 1,
	VESA_GET_DEVICE_NAME,
	VESA_SET_DISPLAY_MODE,
	VESA_GET_DPMS_MODE,
	VESA_SET_DPMS_MODE,
	VESA_SET_INDEXED_COLORS,

	VGA_PLANAR_BLIT,
};

struct vesa_set_indexed_colors_args {
	uint8			first;
	uint16			count;
	uint8*			colors;
};

struct vga_planar_blit_args {
	uint8*			source;
	int32_t			source_bytes_per_row;
	int32_t			left;
	int32_t			top;
	int32_t			right;
	int32_t			bottom;
};

#endif	/* VESA_INFO_H */
