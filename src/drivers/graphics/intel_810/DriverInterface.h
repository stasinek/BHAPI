/*
 * Copyright 2007-2012 Haiku, Inc.  All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *		Gerald Zajac
 */
#ifndef DRIVERINTERFACE_H
#define DRIVERINTERFACE_H


#include <kits/netAccelerant.h>
#include <kits/netGraphicsDefs.h>
#include <kits/netDrivers.h>
#include <kits/netedid.h>


// This file contains info that is shared between the kernel driver and the
// accelerant, and info that is shared among the source files of the
// accelerant.


#define ENABLE_DEBUG_TRACE		// if defined, turns on debug output to syslog


struct Benaphore {
	sem_id	sem;
	int32_t	count;

	status_t Init(const char* name)
	{
		count = 0;
		sem = create_sem(0, name);
		return sem < 0 ? sem : B_OK;
	}

	status_t Acquire()
	{
		if (atomic_add(&count, 1) > 0)
			return acquire_sem(sem);
		return B_OK;
	}

	status_t Release()
	{
		if (atomic_add(&count, -1) > 1)
			return release_sem(sem);
		return B_OK;
	}

	void Delete()	{ delete_sem(sem); }
};



enum {
	INTEL_GET_SHARED_DATA = B_DEVICE_OP_CODES_END + 234,
	INTEL_DEVICE_NAME,
	INTEL_GET_EDID,
};


struct DisplayModeEx : display_mode {
	uint8	bitsPerPixel;
	uint8	bytesPerPixel;
	uint16	bytesPerRow;		// number of bytes in one line/row
};


struct SharedInfo {
	// Device ID info.
	uint16	vendorID;			// PCI vendor ID, from pci_info
	uint16	deviceID;			// PCI device ID, from pci_info
	uint8	revision;			// PCI device revsion, from pci_info
	char	chipName[32];		// user recognizable name of chip

	bool	bAccelerantInUse;	// true = accelerant has been initialized

	// Memory mappings.
	area_id regsArea;			// area_id for the memory mapped registers. It
								// will be cloned into accelerant address space.
	area_id videoMemArea;		// addr shared with all teams.
	addr_t	videoMemAddr;		// virtual video memory addr
	phys_addr_t	videoMemPCI;	// physical video memory addr
	uint32_t	videoMemSize; 		// video memory size in bytes (for frame buffer)

	uint32_t	maxFrameBufferSize;	// max available video memory for frame buffer

	// Color spaces supported by current video chip/driver.
	color_space	colorSpaces[6];
	uint32_t	colorSpaceCount;	// number of color spaces in array colorSpaces

	// List of screen modes.
	area_id modeArea;			// area containing list of display modes
	uint32_t	modeCount;			// number of display modes in the list

	DisplayModeEx displayMode;	// current display mode configuration

	edid1_info	edidInfo;
	bool		bHaveEDID;		// true = EDID info from device is in edidInfo

	Benaphore	engineLock;		// serializing access to the acceleration engine
};


#endif	// DRIVERINTERFACE_H
