/*
 * Copyright 2002-2005, Axel Dörfler, axeld@pinc-software.de. All rights reserved.
 * Distributed under the terms of the MIT License.
 */


#include <OS.h>
#include <syscalls.h>


sem_id
create_sem(int32_t count, const char *name)
{
    return _kern_create_sem(count, name);
}


status_t
delete_sem(sem_id id)
{
    return _kern_delete_sem(id);
}


status_t
acquire_sem(sem_id id)
{
    return _kern_acquire_sem(id);
}


status_t
acquire_sem_etc(sem_id id, int32_t count, uint32_t flags, bigtime_t timeout)
{
    return _kern_acquire_sem_etc(id, count, flags, timeout);
}


// ToDo: the next two calls (switch_sem()) are not yet public API; no decision
//	has been made yet, so they may get changed or removed until R1

status_t
switch_sem(sem_id releaseSem, sem_id id)
{
    return _kern_switch_sem(releaseSem, id);
}


status_t
switch_sem_etc(sem_id releaseSem, sem_id id, int32_t count, uint32_t flags, bigtime_t timeout)
{
    return _kern_switch_sem_etc(releaseSem, id, count, flags, timeout);
}


status_t
release_sem(sem_id id)
{
    return _kern_release_sem(id);
}


status_t
release_sem_etc(sem_id id, int32_t count, uint32_t flags)
{
    return _kern_release_sem_etc(id, count, flags);
}


status_t
get_sem_count(sem_id sem, int32_t *count)
{
    return _kern_get_sem_count(sem, count);
}


status_t
set_sem_owner(sem_id sem, team_id team)
{
    return _kern_set_sem_owner(sem, team);
}


status_t
_get_sem_info(sem_id sem, sem_info *info, size_t size)
{
    return _kern_get_sem_info(sem, info, size);
}


status_t
_get_next_sem_info(team_id team, int32_t *cookie, sem_info *info, size_t size)
{
    return _kern_get_next_sem_info(team, cookie, info, size);
}



#include <BeOSBuildCompatibility.h>

#include <stdlib.h>
#include <kits/support/String.h>

#include <OS.h>
#include <Haiku.h>

// We assume that everything is single-threaded, so we don't need real
// semaphores. Simple fakes are sufficient.

struct semaphore {
    char*	name;
    int32_t	count;
    bool	inUse;
};

static const int kSemaphoreCount = 40960;
static semaphore sSemaphores[kSemaphoreCount];


static bool
check_sem(sem_id id)
{
    if (id < 0 || id >= kSemaphoreCount)
        return false;
    return sSemaphores[id].inUse;
}

// create_sem
sem_id
create_sem(int32_t count, const char *name)
{
    for (int i = 0; i < kSemaphoreCount; i++) {
        semaphore &sem = sSemaphores[i];
        if (!sem.inUse) {
            sem.name = strdup(name ? name : "unnamed sem");
            if (!sem.name)
                return B_NO_MEMORY;

            sem.inUse = true;
            sem.count = count;

            return i;
        }
    }

    return B_NO_MORE_SEMS;
}

// delete_sem
status_t
delete_sem(sem_id id)
{
    if (!check_sem(id))
        return B_BAD_SEM_ID;

    sSemaphores[id].inUse = false;
    free(sSemaphores[id].name);
    sSemaphores[id].name = NULL;

    return B_OK;
}

// acquire_sem
status_t
acquire_sem(sem_id id)
{
    return acquire_sem_etc(id, 1, 0, 0);
}

// acquire_sem_etc
status_t
acquire_sem_etc(sem_id id, int32_t count, uint32_t flags, bigtime_t timeout)
{
    if (!check_sem(id))
        return B_BAD_SEM_ID;

    if (count <= 0)
        return B_BAD_VALUE;

    semaphore &sem = sSemaphores[id];
    if (sem.count >= count) {
        sem.count -= count;
        return B_OK;
    }

    if (timeout < 0)
        timeout = 0;

    bool noTimeout = false;
    if (flags & B_RELATIVE_TIMEOUT) {
        // relative timeout: get the absolute time when to time out

        // special case: on timeout == 0 we return B_WOULD_BLOCK
        if (timeout == 0)
            return B_WOULD_BLOCK;

        bigtime_t currentTime = system_time();
        if (timeout > B_INFINITE_TIMEOUT || currentTime >= B_INFINITE_TIMEOUT - timeout) {
            noTimeout = true;
        } else {
            timeout += currentTime;
        }

    } else if (flags & B_ABSOLUTE_TIMEOUT) {
        // absolute timeout
    } else {
        // no timeout given
        noTimeout = true;
    }

    // no timeout?
    if (noTimeout) {
        debugger("Would block on a semaphore without timeout in a "
            "single-threaded context!");
        return B_ERROR;
    }

    // wait for the time out time
    status_t error = snooze_until(timeout, B_SYSTEM_TIMEBASE);
    if (error != B_OK)
        return error;

    return B_TIMED_OUT;
}

// release_sem
status_t
release_sem(sem_id id)
{
    return release_sem_etc(id, 1, 0);
}

// release_sem_etc
status_t
release_sem_etc(sem_id id, int32_t count, uint32_t flags)
{
    if (!check_sem(id))
        return B_BAD_SEM_ID;

    if (count <= 0)
        return B_BAD_VALUE;

    semaphore &sem = sSemaphores[id];
    sem.count += count;

    return B_OK;
}

// get_sem_count
status_t
get_sem_count(sem_id id, int32_t *threadCount)
{
    if (!check_sem(id))
        return B_BAD_SEM_ID;

    if (!threadCount)
        return B_BAD_VALUE;

    *threadCount = sSemaphores[id].count;
    return B_OK;
}

// set_sem_owner
status_t
set_sem_owner(sem_id id, team_id team)
{
    if (!check_sem(id))
        return B_BAD_SEM_ID;

    return B_OK;
}

// _get_sem_info
status_t
_get_sem_info(sem_id id, struct sem_info *info, size_t infoSize)
{
    if (!check_sem(id))
        return B_BAD_SEM_ID;

    if (!info)
        return B_BAD_VALUE;

    info->sem = id;
    info->team = 1;
    strlcpy(info->name, sSemaphores[id].name, sizeof(info->name));
    info->count = sSemaphores[id].count;
    info->latest_holder = -1;

    return B_OK;
}

// _get_next_sem_info
status_t
_get_next_sem_info(team_id team, int32_t *cookie, struct sem_info *info,
    size_t infoSize)
{
    if (team < 0 || team > 2)
        return B_BAD_TEAM_ID;

    for (int i = *cookie; i < kSemaphoreCount; i++) {
        if (sSemaphores[i].inUse) {
            *cookie = i + 1;
            return _get_sem_info(i, info, infoSize);
        }
    }

    return B_ENTRY_NOT_FOUND;
}
