/*
 * Copyright 2007-2009, Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stefano Ceccherini <stefano.ceccherini@gmail.com>
 */
#ifndef _INTERFACE_PRIVATE_H
#define _INTERFACE_PRIVATE_H


#include <GraphicsDefs.h>
#include <Haiku.h>


void _init_global_fonts_();
extern "C" status_t _fini_interface_kit_();


namespace BPrivate {

bool		get_mode_parameter(uint32_t mode, int32_t& width, int32_t& height,
				uint32_t& colorSpace);
int32_t		get_bytes_per_row(color_space colorSpace, int32_t width);

void		get_workspaces_layout(uint32_t* _columns, uint32_t* _rows);
void		set_workspaces_layout(uint32_t columns, uint32_t rows);

}	// namespace BPrivate


#endif	// _INTERFACE_PRIVATE_H
