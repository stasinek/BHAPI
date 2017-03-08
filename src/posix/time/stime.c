/*
** Copyright 2004, Jérôme Duval. All rights reserved.
** Distributed under the terms of the Haiku License.
*/

#include <time.h>
#include <errno.h>
#include "syscalls.h"

#include <errno_private.h>


int
stime(const time_t *tp)
{
	status_t status;

	if (tp == NULL) {
		__set_errno(EINVAL);
		return -1;
	}

	status = _kern_set_real_time_clock((bigtime_t)*tp * 1000000);
	if (status < B_OK) {
		__set_errno(status);
		return -1;
	}
	return 0;
}

