/*
 * Copyright 2012 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *      Alexander von Gluck, kallisti5@unixzen.com
 */
#ifndef _DP_H
#define _DP_H


#include <Accelerant.h>
#include "dp_raw.h"
#include <GraphicsDefs.h>
#include <OS.h>


typedef struct {
	// Required configuration
	bool	valid;			// Is valid DP information
	uint32	auxPin;			// Normally GPIO pin on GPU

	uint8	config[DP_DPCD_SIZE]; // DP Configuration Data
	int		laneCount;

	uint32	linkRate;		// DP Link Speed 162000, 270000, 540000

	// Internal State information
	uint8	linkStatus[DP_LINK_STATUS_SIZE];

	bool    trainingUseEncoder;

	uint8   trainingAttempts;
	uint8   trainingSet[4];
	int     trainingReadInterval;

} dp_info;

typedef struct {
	uint16	address;
	uint8	request;
	uint8	reply;
	void*	buffer;
	size_t	size;
} dp_aux_msg;

uint32 dp_encode_link_rate(uint32 linkRate);
uint32 dp_decode_link_rate(uint32 rawLinkRate);

uint32 dp_get_lane_count_max(dp_info* dpInfo);
uint32 dp_get_link_rate_max(dp_info* dpInfo);

uint32 dp_get_pixel_clock_max(int linkRate, int laneCount, int bpp);


#endif /* _DP_H */
