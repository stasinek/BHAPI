﻿//-----------------------------------------------------------------------------
#include <kernel.h>
#include <syscalls.h>
#include <memory_private.h>
#include <stdarg.h>
//-----------------------------------------------------------------------------

extern "C" void debug_printf(const char *format, ...)
{
    va_list list;
    va_start(list, format);
    vprintf(format, list);
    va_end(list);
}
//-----------------------------------------------------------------------------

extern "C" void ktrace_printf(const char *format, ...)
{
}
//-----------------------------------------------------------------------------

dev_t fs_mount_volume(const char *where, const char *device, const char *filesystem, uint32_t flags, const char *parameters)
{
    return B_ERROR;
}
//-----------------------------------------------------------------------------

status_t fs_unmount_volume(const char *path, uint32_t flags)
{
    return B_ERROR;
}
//-----------------------------------------------------------------------------

status_t __get_port_message_info_etc(port_id id, port_message_info *info, size_t infoSize, uint32_t flags, bigtime_t timeout)
{
    return B_ERROR;
}
//-----------------------------------------------------------------------------

int32_t atomic_get(vint32_t *value)
{
    return *value;
}
//-----------------------------------------------------------------------------
