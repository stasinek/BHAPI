/*
 * Copyright 2013 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ithamar R. Adema, ithamar@upgrade-android.com
 */

#include <KernelExport.h>

#include <kernel.h>
#include <user_atomic.h>

#include <util/AutoLock.h>

#ifdef ATOMIC_FUNCS_ARE_SYSCALLS

/*
 * NOTE: These functions are _intentionally_ not using spinlocks, unlike
 * the 64 bit versions. The reason for this is that they are used by the
 * spinlock code itself, and therefore would deadlock.
 *
 * Since these are only really needed for ARMv5, which is not SMP anyway,
 * this is an acceptable compromise.
 */

void
atomic_set(int32_t *value, int32_t newValue)
{
	InterruptsLocker locker;
	*value = newValue;
}

int32_t
atomic_get_and_set(int32_t *value, int32_t newValue)
{
	InterruptsLocker locker;
	int32_t oldValue = *value;
	atomic_set(value, newValue);
	return oldValue;
}

int32_t
atomic_test_and_set(int32_t *value, int32_t newValue, int32_t testAgainst)
{
	InterruptsLocker locker;

	int32_t oldValue = *value;
	if (oldValue == testAgainst)
		*value = newValue;
	return oldValue;
}

int32_t
atomic_add(int32_t *value, int32_t addValue)
{
	InterruptsLocker locker;

	int32_t oldValue = *value;
	*value += addValue;
	return oldValue;
}

int32_t
atomic_and(int32_t *value, int32_t andValue)
{
	InterruptsLocker locker;

	int32_t oldValue = *value;
	*value &= andValue;
	return oldValue;
}

int32_t
atomic_or(int32_t *value, int32_t orValue)
{
	InterruptsLocker locker;

	int32_t oldValue = *value;
	*value |= orValue;
	return oldValue;
}

int32_t
atomic_get(int32_t *value)
{
	InterruptsLocker locker;

	int32_t oldValue = *value;
	return oldValue;
}

void
_user_atomic_set(int32_t *value, int32_t newValue)
{
	if (IS_USER_ADDRESS(value)
		&& lock_memory((void *)value, sizeof(int32_t), B_READ_DEVICE) == B_OK) {
		atomic_set(value, newValue);
		unlock_memory((void *)value, sizeof(int32_t), B_READ_DEVICE);
		return;
	}

access_violation:
	// XXX kill application
	return;
}

int32_t
_user_atomic_get_and_set(int32_t *value, int32_t newValue)
{
	if (IS_USER_ADDRESS(value)
		&& lock_memory((void *)value, sizeof(int32_t), B_READ_DEVICE) == B_OK) {
		int32_t oldValue = atomic_get_and_set(value, newValue);
		unlock_memory((void *)value, sizeof(int32_t), B_READ_DEVICE);
		return oldValue;
	}

access_violation:
	// XXX kill application
	return -1;
}

int32_t
_user_atomic_test_and_set(int32_t *value, int32_t newValue, int32_t testAgainst)
{
	if (IS_USER_ADDRESS(value)
		&& lock_memory((void *)value, sizeof(int32_t), B_READ_DEVICE) == B_OK) {
		int32_t oldValue = atomic_test_and_set((int32_t*)value, newValue, testAgainst);
		unlock_memory((void *)value, sizeof(int32_t), B_READ_DEVICE);
		return oldValue;
	}

access_violation:
	// XXX kill application
	return -1;
}

int32_t
_user_atomic_add(int32_t *value, int32_t addValue)
{
	if (IS_USER_ADDRESS(value)
		&& lock_memory((void *)value, sizeof(int32_t), B_READ_DEVICE) == B_OK) {
		int32_t oldValue = atomic_add(value, addValue);
		unlock_memory((void *)value, sizeof(int32_t), B_READ_DEVICE);
		return oldValue;
	}

access_violation:
	// XXX kill application
	return -1;
}

int32_t
_user_atomic_and(int32_t *value, int32_t andValue)
{
	if (IS_USER_ADDRESS(value)
		&& lock_memory((void *)value, sizeof(int32_t), B_READ_DEVICE) == B_OK) {
		int32_t oldValue = atomic_and(value, andValue);
		unlock_memory((void *)value, sizeof(int32_t), B_READ_DEVICE);
		return oldValue;
	}

access_violation:
	// XXX kill application
	return -1;
}

int32_t
_user_atomic_or(int32_t *value, int32_t orValue)
{
	if (IS_USER_ADDRESS(value)
		&& lock_memory((void *)value, sizeof(int32_t), B_READ_DEVICE) == B_OK) {
		int32_t oldValue = atomic_or(value, orValue);
		unlock_memory((void *)value, sizeof(int32_t), B_READ_DEVICE);
		return oldValue;
	}

access_violation:
	// XXX kill application
	return -1;
}

int32_t
_user_atomic_get(int32_t *value)
{
	if (IS_USER_ADDRESS(value)
		&& lock_memory((void *)value, sizeof(int32_t), B_READ_DEVICE) == B_OK) {
		int32_t oldValue = atomic_get(value);
		unlock_memory((void *)value, sizeof(int32_t), B_READ_DEVICE);
		return oldValue;
	}

access_violation:
	// XXX kill application
	return -1;
}

#endif /* ATOMIC_FUNCS_ARE_SYSCALLS */
