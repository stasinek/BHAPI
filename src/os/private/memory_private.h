/*
 * Copyright 2011, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef BEOS_MEMORY_PRIVATE_H
#define BEOS_MEMORY_PRIVATE_H
#include <kernel.h>
//-------------------------------------------------------------------------------------------------
extern status_t get_memory_properties(bhapi::team_id teamID, const void* address, uint32* _protected, uint32* _lock);
//-------------------------------------------------------------------------------------------------
#endif
