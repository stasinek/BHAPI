#ifndef _LINUX_TIME_H
#define _LINUX_TIME_H

struct timezone {
	int     tz_minuteswest; /* minutes west of Greenwich */
	int     tz_dsttime;     /* type of dst correction */
};

struct timezone sys_tz;

#endif