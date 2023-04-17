/*
** Copyright 2003, Marcus Overhagen. All rights reserved.
** Distributed under the terms of the Haiku License.
*/


#include <kits/support/SupportDefs.h>

#include <syscalls.h>


#ifdef ATOMIC_FUNCS_ARE_SYSCALLS

void
atomic_set(int32_t *value, int32_t newValue)
{
    _kern_atomic_set(value, newValue);
}


int32_t atomic_get_and_set(int32_t *value, int32_t newValue)
{
    return _kern_atomic_get_and_set(value, newValue);
}


int32_t atomic_test_and_set(int32_t *value, int32_t newValue, int32_t testAgainst)
{
    return _kern_atomic_test_and_set(value, newValue, testAgainst);
}


int32_t atomic_add(int32_t *value, int32_t addValue)
{
    return _kern_atomic_add(value, addValue);
}


int32_t atomic_and(int32_t *value, int32_t andValue)
{
    return _kern_atomic_and(value, andValue);
}


int32_t atomic_or(int32_t *value, int32_t orValue)
{
    return _kern_atomic_or(value, orValue);
}


int32_t atomic_get(int32_t *value)
{
    return _kern_atomic_get(value);
}


#endif	/* ATOMIC_FUNCS_ARE_SYSCALLS */

#ifdef ATOMIC64_FUNCS_ARE_SYSCALLS

void
atomic_set64(int64 *value, int64 newValue)
{
    _kern_atomic_set64(value, newValue);
}


int64 atomic_test_and_set64(int64 *value, int64 newValue, int64 testAgainst)
{
    return _kern_atomic_test_and_set64(value, newValue, testAgainst);
}


int64 atomic_add64(int64 *value, int64 addValue)
{
    return _kern_atomic_add64(value, addValue);
}


int64 atomic_and64(int64 *value, int64 andValue)
{
    return _kern_atomic_and64(value, andValue);
}


int64 atomic_or64(int64 *value, int64 orValue)
{
    return _kern_atomic_or64(value, orValue);
}


int64 atomic_get64(int64 *value)
{
    return _kern_atomic_get64(value);
}


#endif	/* ATOMIC64_FUNCS_ARE_SYSCALLS */

#ifdef __ARM__

/* GCC compatibility: libstdc++ needs this one.
 * TODO: Update libstdc++ and drop this.
 * cf. http://fedoraproject.org/wiki/Architectures/ARM/GCCBuiltInAtomicOperations
 */
extern int32_t_t __sync_fetch_and_add_4(int32_t_t *value, int32_t_t addValue);

extern int32_t_t __sync_fetch_and_add_4(int32_t_t *value, int32_t_t addValue)
{
    return atomic_add((int32_t *)value, addValue);
}

#endif

#include <BeOSBuildCompatibility.h>

#include <kits/support/String.h>

#include <OS.h>
#include <Haiku.h>


#if (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 7)) \
    && !defined(__clang__)


void
atomic_set(int32_t *value, int32_t newValue)
{
    *value = newValue;
}


int32_t atomic_get_and_set(int32_t *value, int32_t newValue)
{
    int32_t oldValue = *value;
    *value = newValue;
    return oldValue;
}


int32_t atomic_test_and_set(int32_t *value, int32_t newValue, int32_t testAgainst)
{
    int32_t oldValue = *value;
    if (oldValue == testAgainst)
        *value = newValue;
    return oldValue;
}


int32_t atomic_add(int32_t *value, int32_t addValue)
{
    int32_t oldValue = *value;
    *value += addValue;
    return oldValue;
}


int32_t atomic_and(int32_t *value, int32_t andValue)
{
    int32_t oldValue = *value;
    *value &= andValue;
    return oldValue;
}


int32_t atomic_or(int32_t *value, int32_t orValue)
{
    int32_t oldValue = *value;
    *value |= orValue;
    return oldValue;
}


int32_t atomic_get(int32_t *value)
{
    return *value;
}


void
atomic_set64(int64 *value, int64 newValue)
{
    *value = newValue;
}


int64 atomic_get_and_set64(int64 *value, int64 newValue)
{
    int64 oldValue = *value;
    *value = newValue;
    return oldValue;
}

int64 atomic_test_and_set64(int64 *value, int64 newValue, int64 testAgainst)
{
    int64 oldValue = *value;
    if (oldValue == testAgainst)
        *value = newValue;
    return oldValue;
}

int64 atomic_add64(int64 *value, int64 addValue)
{
    int64 oldValue = *value;
    *value += addValue;
    return oldValue;
}

int64 atomic_and64(int64 *value, int64 andValue)
{
    int64 oldValue = *value;
    *value &= andValue;
    return oldValue;
}

int64 atomic_or64(int64 *value, int64 orValue)
{
    int64 oldValue = *value;
    *value |= orValue;
    return oldValue;
}

int64 atomic_get64(int64 *value)
{
    return *value;
}


#endif	// __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 7)

