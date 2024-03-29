/*
 * Copyright 2010 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef STRING_FOR_SIZE_H
#define STRING_FOR_SIZE_H

#include <Haiku.h>


namespace BPrivate {


const char* string_for_size(double size, char* string, size_t stringSize);


}	// namespace BPrivate


using BPrivate::string_for_size;


#endif // STRING_FOR_SIZE_H
