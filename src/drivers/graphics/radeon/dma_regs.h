/*
	Copyright (c) 2002, Thomas Kurschel


	Part of Radeon driver
		
	DMA registers
*/

#ifndef _DMA_REGS_H
#define _DMA_REGS_H

typedef struct DMA_descriptor {
	uint32_t	src_address;
	uint32_t	dest_address;
	uint32_t	command;
	uint32_t	res;
} DMA_descriptor;

#define RADEON_DMA_COMMAND_EOL		(1 << 31)
#define RADEON_DMA_COMMAND_INTDIS	(1 << 30)
#define RADEON_DMA_COMMAND_DAIC		(1 << 29)
#define RADEON_DMA_COMMAND_SAIC		(1 << 28)
#define RADEON_DMA_COMMAND_DAS		(1 << 27)
#define RADEON_DMA_COMMAND_SAS		(1 << 26)
#define RADEON_DMA_COMMAND_DST_SWAP_SHIFT	24
#define RADEON_DMA_COMMAND_SRC_SWAP_SHIFT	24
#define RADEON_DMA_COMMAND_BYTE_COUNT_SHIFT	0

#define	RADEON_DMA_DESC_MAX_SIZE	0x1fffff

#define	RADEON_DMA_GUI_TABLE_ADDR		0x0780
#define	RADEON_DMA_GUI_STATUS			0x0790
#define	RADEON_DMA_STATUS_ACTIVE		(1 << 21)

#define	RADEON_DMA_VID_TABLE_ADDR		0x07a0
#define	RADEON_DMA_VID_STATUS			0x07b0

#endif
