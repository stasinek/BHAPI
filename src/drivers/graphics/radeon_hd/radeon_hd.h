/*
 * Copyright 2006-2011, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Axel Dörfler, axeld@pinc-software.de
 *		Alexander von Gluck IV, kallisti5@unixzen.com
 */
#ifndef RADEON_HD_H
#define RADEON_HD_H


#include <kits/netlock.h>

#include <kits/netradeon_reg.h>

//#include <kits/netr500_reg.h>  // Not used atm. DCE 0
#include <kits/netavivo_reg.h>		// DCE 1
#include <kits/netr600_reg.h>		// DCE 2
#include <kits/netr700_reg.h>		// DCE 3
#include <kits/netevergreen_reg.h>	// DCE 4
#include <kits/netni_reg.h>			// DCE 5
#include <kits/netsi_reg.h>			// DCE 6
#include <kits/netsea_reg.h>		// DCE 8
#include <kits/netvol_reg.h>		// DCE 10
#include <kits/netcar_reg.h>		// DCE 11

#include <kits/netAccelerant.h>
#include <kits/netDrivers.h>
#include <kits/netedid.h>
#include <kits/netPCI.h>


#define VENDOR_ID_ATI	0x1002

// Card chipset flags
#define CHIP_STD		(1 << 0) // Standard chipset
#define CHIP_X2			(1 << 1) // Dual cpu
#define CHIP_IGP		(1 << 2) // IGP chipset
#define CHIP_MOBILE		(1 << 3) // Mobile chipset
#define CHIP_DISCREET	(1 << 4) // Discreet chipset
#define CHIP_APU		(1 << 5) // APU chipset

#define DEVICE_NAME				"radeon_hd"
#define RADEON_ACCELERANT_NAME	"radeon_hd.accelerant"

#define MAX_NAME_LENGTH		32

// Used to collect EDID from boot loader
#define EDID_BOOT_INFO "vesa_edid/v1"
#define MODES_BOOT_INFO "vesa_modes/v1"

#define RHD_POWER_ON       0
#define RHD_POWER_RESET    1   /* off temporarily */
#define RHD_POWER_SHUTDOWN 2   /* long term shutdown */
#define RHD_POWER_UNKNOWN  3   /* initial state */


// Radeon Chipsets
// !! Must match chipset names below
enum radeon_chipset {
	RADEON_R420 = 0,	//r400, Radeon X700-X850
	RADEON_R423,
	RADEON_RV410,
	RADEON_RS400,
	RADEON_RS480,
	RADEON_RS600,
	RADEON_RS690,
	RADEON_RS740,
	RADEON_RV515,
	RADEON_R520,		//r500, DCE 1.0
	RADEON_RV530,		// DCE 1.0
	RADEON_RV560,		// DCE 1.0
	RADEON_RV570,		// DCE 1.0
	RADEON_R580,		// DCE 1.0
	RADEON_R600,		//r600, DCE 2.0
	RADEON_RV610,		// DCE 2.0
	RADEON_RV630,		// DCE 2.0
	RADEON_RV670,		// DCE 2.0
	RADEON_RV620,		// DCE 3.0
	RADEON_RV635,		// DCE 3.0
	RADEON_RS780,		// DCE 3.0
	RADEON_RS880,		// DCE 3.0
	RADEON_RV770,		//r700, DCE 3.1
	RADEON_RV730,		// DCE 3.2
	RADEON_RV710,		// DCE 3.2
	RADEON_RV740,		// DCE 3.2
	RADEON_CEDAR,		//Evergreen, DCE 4.0
	RADEON_REDWOOD,		// DCE 4.0
	RADEON_JUNIPER,		// DCE 4.0
	RADEON_CYPRESS,		// DCE 4.0
	RADEON_HEMLOCK,		// DCE 4.0?
	RADEON_PALM,		//Fusion APU (NI), DCE 4.1
	RADEON_SUMO,		// DCE 4.1
	RADEON_SUMO2,		// DCE 4.1
	RADEON_CAICOS,		//Nothern Islands, DCE 5.0
	RADEON_TURKS,		// DCE 5.0
	RADEON_BARTS,		// DCE 5.0
	RADEON_CAYMAN,		// DCE 5.0
	RADEON_ANTILLES,	// DCE 5.0?
	RADEON_CAPEVERDE,	//Southern Islands, DCE 6.0
	RADEON_PITCAIRN,	// DCE 6.0
	RADEON_TAHITI,		// DCE 6.0
	RADEON_ARUBA,		// DCE 6.1 Trinity/Richland
	RADEON_OLAND,		// DCE 6.4
	RADEON_HAINAN,		// NO DCE, only compute
	RADEON_KAVERI,		//Sea Islands, DCE 8.1
	RADEON_BONAIRE,		// DCE 8.2
	RADEON_KABINI,		// DCE 8.3
	RADEON_MULLINS,		// DCE 8.3
	RADEON_HAWAII,		// DCE 8.5
	RADEON_TOPAZ,		//Volcanic Islands, NO DCE
	RADEON_TONGA,		// DCE 10.0
	RADEON_CARRIZO		// DCE 11.0
};

// !! Must match chipset families above
static const char radeon_chip_name[][MAX_NAME_LENGTH] = {
	"R420",
	"R423",
	"RV410",
	"RS400",
	"RS480",
	"RS600",
	"RS690",
	"RS740",
	"RV515",
	"R520",
	"RV530",
	"RV560",
	"RV570",
	"R580",
	"R600",
	"RV610",
	"RV630",
	"RV670",
	"RV620",
	"RV635",
	"RS780",
	"RS880",
	"RV770",
	"RV730",
	"RV710",
	"RV740",
	"Cedar",
	"Redwood",
	"Juniper",
	"Cypress",
	"Hemlock",
	"Palm",
	"Sumo",
	"Sumo2",
	"Caicos",
	"Turks",
	"Barts",
	"Cayman",
	"Antilles",
	"Cape Verde",
	"Pitcairn",
	"Tahiti",
	"Aruba",
	"Oland",
	"Hainan",
	"Kaveri",
	"Bonaire",
	"Kabini",
	"Mullins",
	"Hawaii",
	"Topaz",
	"Tonga",
	"Carrizo"
};


struct ring_buffer {
	struct lock		lock;
	uint32_t			register_base;
	uint32_t			offset;
	uint32_t			size;
	uint32_t			position;
	uint32_t			space_left;
	uint8*			base;
};


struct overlay_registers;


struct radeon_shared_info {
	uint32_t			deviceIndex;		// accelerant index
	uint32_t			pciID;				// device pciid
	area_id			mode_list_area;		// area containing display mode list
	uint32_t			mode_count;

	bool			has_rom;			// was rom mapped?
	area_id			rom_area;			// area of mapped rom
	uint32_t			rom_phys;			// rom base location
	uint32_t			rom_size;			// rom size
	uint8*			rom;				// cloned, memory mapped PCI ROM

	display_mode	current_mode;
	uint32_t			bytes_per_row;
	uint32_t			bits_per_pixel;
	uint32_t			dpms_mode;

	area_id			registers_area;			// area of memory mapped registers
	uint8*			status_page;
	addr_t			physical_status_page;
	uint32_t			graphics_memory_size;

	uint8*			frame_buffer;			// virtual memory mapped FB
	area_id			frame_buffer_area;		// area of memory mapped FB
	addr_t			frame_buffer_phys;		// card PCI BAR address of FB
	uint32_t			frame_buffer_size;		// FB size mapped

	bool			has_edid;
	edid1_info		edid_info;

	struct lock		accelerant_lock;
	struct lock		engine_lock;

	ring_buffer		primary_ring_buffer;

	int32_t			overlay_channel_used;
	bool			overlay_active;
	uint32_t			overlay_token;
	addr_t			physical_overlay_registers;
	uint32_t			overlay_offset;

	bool			hardware_cursor_enabled;
	sem_id			vblank_sem;

	uint8*			cursor_memory;
	addr_t			physical_cursor_memory;
	uint32_t			cursor_buffer_offset;
	uint32_t			cursor_format;
	bool			cursor_visible;
	uint16			cursor_hot_x;
	uint16			cursor_hot_y;

	char			deviceName[MAX_NAME_LENGTH];
	uint16			chipsetID;
	char			chipsetName[MAX_NAME_LENGTH];
	uint32_t			chipsetFlags;
	uint8			dceMajor;
	uint8			dceMinor;

	uint16			color_data[3 * 256];    // colour lookup table
};

//----------------- ioctl() interface ----------------

// magic code for ioctls
#define RADEON_PRIVATE_DATA_MAGIC		'rdhd'

// list ioctls
enum {
	RADEON_GET_PRIVATE_DATA = B_DEVICE_OP_CODES_END + 1,

	RADEON_GET_DEVICE_NAME,
	RADEON_ALLOCATE_GRAPHICS_MEMORY,
	RADEON_FREE_GRAPHICS_MEMORY
};

// retrieve the area_id of the kernel/accelerant shared info
struct radeon_get_private_data {
	uint32_t	magic;				// magic number
	area_id	shared_info_area;
};

// allocate graphics memory
struct radeon_allocate_graphics_memory {
	uint32_t	magic;
	uint32_t	size;
	uint32_t	alignment;
	uint32_t	flags;
	uint32_t	buffer_base;
};

// free graphics memory
struct radeon_free_graphics_memory {
	uint32_t 	magic;
	uint32_t	buffer_base;
};

// registers
#define R6XX_CONFIG_APER_SIZE			0x5430	// r600>
#define OLD_CONFIG_APER_SIZE			0x0108	// <r600
#define CONFIG_MEMSIZE                  0x5428	// r600>

// PCI bridge memory management

// overlay

#define RADEON_OVERLAY_UPDATE			0x30000
#define RADEON_OVERLAY_TEST				0x30004
#define RADEON_OVERLAY_STATUS			0x30008
#define RADEON_OVERLAY_EXTENDED_STATUS	0x3000c
#define RADEON_OVERLAY_GAMMA_5			0x30010
#define RADEON_OVERLAY_GAMMA_4			0x30014
#define RADEON_OVERLAY_GAMMA_3			0x30018
#define RADEON_OVERLAY_GAMMA_2			0x3001c
#define RADEON_OVERLAY_GAMMA_1			0x30020
#define RADEON_OVERLAY_GAMMA_0			0x30024

struct overlay_scale {
	uint32_t _reserved0 : 3;
	uint32_t horizontal_scale_fraction : 12;
	uint32_t _reserved1 : 1;
	uint32_t horizontal_downscale_factor : 3;
	uint32_t _reserved2 : 1;
	uint32_t vertical_scale_fraction : 12;
};

#define OVERLAY_FORMAT_RGB15			0x2
#define OVERLAY_FORMAT_RGB16			0x3
#define OVERLAY_FORMAT_RGB32			0x1
#define OVERLAY_FORMAT_YCbCr422			0x8
#define OVERLAY_FORMAT_YCbCr411			0x9
#define OVERLAY_FORMAT_YCbCr420			0xc

#define OVERLAY_MIRROR_NORMAL			0x0
#define OVERLAY_MIRROR_HORIZONTAL		0x1
#define OVERLAY_MIRROR_VERTICAL			0x2

// The real overlay registers are written to using an update buffer

struct overlay_registers {
	uint32_t buffer_rgb0;
	uint32_t buffer_rgb1;
	uint32_t buffer_u0;
	uint32_t buffer_v0;
	uint32_t buffer_u1;
	uint32_t buffer_v1;
	// (0x18) OSTRIDE - overlay stride
	uint16 stride_rgb;
	uint16 stride_uv;
	// (0x1c) YRGB_VPH - Y/RGB vertical phase
	uint16 vertical_phase0_rgb;
	uint16 vertical_phase1_rgb;
	// (0x20) UV_VPH - UV vertical phase
	uint16 vertical_phase0_uv;
	uint16 vertical_phase1_uv;
	// (0x24) HORZ_PH - horizontal phase
	uint16 horizontal_phase_rgb;
	uint16 horizontal_phase_uv;
	// (0x28) INIT_PHS - initial phase shift
	uint32_t initial_vertical_phase0_shift_rgb0 : 4;
	uint32_t initial_vertical_phase1_shift_rgb0 : 4;
	uint32_t initial_horizontal_phase_shift_rgb0 : 4;
	uint32_t initial_vertical_phase0_shift_uv : 4;
	uint32_t initial_vertical_phase1_shift_uv : 4;
	uint32_t initial_horizontal_phase_shift_uv : 4;
	uint32_t _reserved0 : 8;
	// (0x2c) DWINPOS - destination window position
	uint16 window_left;
	uint16 window_top;
	// (0x30) DWINSZ - destination window size
	uint16 window_width;
	uint16 window_height;
	// (0x34) SWIDTH - source width
	uint16 source_width_rgb;
	uint16 source_width_uv;
	// (0x38) SWITDHSW - source width in 8 byte steps
	uint16 source_bytes_per_row_rgb;
	uint16 source_bytes_per_row_uv;
	uint16 source_height_rgb;
	uint16 source_height_uv;
	overlay_scale scale_rgb;
	overlay_scale scale_uv;
	// (0x48) OCLRC0 - overlay color correction 0
	uint32_t brightness_correction : 8;		// signed, -128 to 127
	uint32_t _reserved1 : 10;
	uint32_t contrast_correction : 9;			// fixed point: 3.6 bits
	uint32_t _reserved2 : 5;
	// (0x4c) OCLRC1 - overlay color correction 1
	uint32_t saturation_cos_correction : 10;	// fixed point: 3.7 bits
	uint32_t _reserved3 : 6;
	uint32_t saturation_sin_correction : 11;	// signed fixed point: 3.7 bits
	uint32_t _reserved4 : 5;
	// (0x50) DCLRKV - destination color key value
	uint32_t color_key_blue : 8;
	uint32_t color_key_green : 8;
	uint32_t color_key_red : 8;
	uint32_t _reserved5 : 8;
	// (0x54) DCLRKM - destination color key mask
	uint32_t color_key_mask_blue : 8;
	uint32_t color_key_mask_green : 8;
	uint32_t color_key_mask_red : 8;
	uint32_t _reserved6 : 7;
	uint32_t color_key_enabled : 1;
	// (0x58) SCHRKVH - source chroma key high value
	uint32_t source_chroma_key_high_red : 8;
	uint32_t source_chroma_key_high_blue : 8;
	uint32_t source_chroma_key_high_green : 8;
	uint32_t _reserved7 : 8;
	// (0x5c) SCHRKVL - source chroma key low value
	uint32_t source_chroma_key_low_red : 8;
	uint32_t source_chroma_key_low_blue : 8;
	uint32_t source_chroma_key_low_green : 8;
	uint32_t _reserved8 : 8;
	// (0x60) SCHRKEN - source chroma key enable
	uint32_t _reserved9 : 24;
	uint32_t source_chroma_key_red_enabled : 1;
	uint32_t source_chroma_key_blue_enabled : 1;
	uint32_t source_chroma_key_green_enabled : 1;
	uint32_t _reserved10 : 5;
	// (0x64) OCONFIG - overlay configuration
	uint32_t _reserved11 : 3;
	uint32_t color_control_output_mode : 1;
	uint32_t yuv_to_rgb_bypass : 1;
	uint32_t _reserved12 : 11;
	uint32_t gamma2_enabled : 1;
	uint32_t _reserved13 : 1;
	uint32_t select_pipe : 1;
	uint32_t slot_time : 8;
	uint32_t _reserved14 : 5;
	// (0x68) OCOMD - overlay command
	uint32_t overlay_enabled : 1;
	uint32_t active_field : 1;
	uint32_t active_buffer : 2;
	uint32_t test_mode : 1;
	uint32_t buffer_field_mode : 1;
	uint32_t _reserved15 : 1;
	uint32_t tv_flip_field_enabled : 1;
	uint32_t _reserved16 : 1;
	uint32_t tv_flip_field_parity : 1;
	uint32_t source_format : 4;
	uint32_t ycbcr422_order : 2;
	uint32_t _reserved18 : 1;
	uint32_t mirroring_mode : 2;
	uint32_t _reserved19 : 13;

	uint32_t _reserved20;

	uint32_t start_0y;
	uint32_t start_1y;
	uint32_t start_0u;
	uint32_t start_0v;
	uint32_t start_1u;
	uint32_t start_1v;
	uint32_t _reserved21[6];
#if 0
	// (0x70) AWINPOS - alpha blend window position
	uint32_t awinpos;
	// (0x74) AWINSZ - alpha blend window size
	uint32_t awinsz;

	uint32_t _reserved21[10];
#endif

	// (0xa0) FASTHSCALE - fast horizontal downscale (strangely enough,
	// the next two registers switch the usual Y/RGB vs. UV order)
	uint16 horizontal_scale_uv;
	uint16 horizontal_scale_rgb;
	// (0xa4) UVSCALEV - vertical downscale
	uint16 vertical_scale_uv;
	uint16 vertical_scale_rgb;

	uint32_t _reserved22[86];

	// (0x200) polyphase filter coefficients
	uint16 vertical_coefficients_rgb[128];
	uint16 horizontal_coefficients_rgb[128];

	uint32_t	_reserved23[64];

	// (0x500)
	uint16 vertical_coefficients_uv[128];
	uint16 horizontal_coefficients_uv[128];
};


struct hardware_status {
	uint32_t	interrupt_status_register;
	uint32_t	_reserved0[3];
	void*	primary_ring_head_storage;
	uint32_t	_reserved1[3];
	void*	secondary_ring_0_head_storage;
	void*	secondary_ring_1_head_storage;
	uint32_t	_reserved2[2];
	void*	binning_head_storage;
	uint32_t	_reserved3[3];
	uint32_t	store[1008];
};

#endif	/* RADEON_HD_H */
