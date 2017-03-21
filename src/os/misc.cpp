include <../include/stdarg.h>


#include <fs_volume.h>

extern "C" void debug_printf(const char *format, ...)
{
	va_list list;

	va_start(list, format);
	vprintf(format, list);
	va_end(list);
}


extern "C" void ktrace_printf(const char *format, ...)
{
}


dev_t
fs_mount_volume(const char *where, const char *device, const char *filesystem,
	uint32 flags, const char *parameters)
{
	return B_ERROR;
}


status_t fs_unmount_volume(const char *path, uint32 flags)
{
	return B_ERROR;
}


int32 atomic_get(vint32 *value)
{
	return *value;
}


status_t _get_port_message_info_etc(port_id id, port_message_info *info,
	size_t infoSize, uint32 flags, bigtime_t timeout)
{
	return B_ERROR;
}



#include <BeOSBuildCompatibility.h>

#include <errno.h>


#include <unistd.h>
#include <../include/sys/stat.h>
#include <../include/sys/time.h>

#include <Debug.h>
#include <image.h>
#include <kernel/OS.h>

mode_t __gUmask = 022;

// debugger
void debugger(const char *message)
{
	fprintf(stderr, "debugger() called: %s\n", message);
	exit(1);
}

// _debuggerAssert
int
_debuggerAssert(const char *file, int line, const char *expression)
{
	char buffer[2048];
	snprintf(buffer, sizeof(buffer), "%s:%d: %s\n", file, line, expression);
	debugger(buffer);
	return 0;
}

// system_time
bigtime_t
system_time(void)
{
	struct timeval tm;
	gettimeofday(&tm, NULL);
	return (int64)tm.tv_sec * 1000000LL + (int64)tm.tv_usec;
}

// snooze
status_t snooze(bigtime_t amount)
{
	if (amount <= 0)
		return B_OK;

	int64 secs = amount / 1000000LL;
	int64 usecs = amount % 1000000LL;
	if (secs > 0) {
		if (sleep((unsigned)secs) < 0)
			return errno;
	}

	if (usecs > 0) {
		if (usleep((useconds_t)usecs) < 0)
			return errno;
	}

	return B_OK;
}

// snooze_until
status_t snooze_until(bigtime_t time, int timeBase)
{
	return snooze(time - system_time());
}
