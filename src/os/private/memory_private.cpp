/*
 * Copyright 2011, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
//-------------------------------------------------------------------------------------------------
#include <memory_private.h>
#include <syscalls.h>
//-------------------------------------------------------------------------------------------------

status_t get_memory_properties(bhapi::team_id a_teamID, const void* a_address, uint32_t* a_protected, uint32_t* a_lock)
{
return _kern_get_memory_properties(a_teamID, a_address, a_protected, a_lock);
}
//-------------------------------------------------------------------------------------------------
