/*
 * Copyright 2005-2007, Ingo Weinhold, bonefish@users.sf.net.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef _FSSH_AUTO_LOCKER_H
#define _FSSH_AUTO_LOCKER_H


#include <fssh_types.h>


namespace FSShell {

// AutoLockerStandardLocking
template<typename Lockable>
class AutoLockerStandardLocking {
public:
	inline bool Lock(Lockable *lockable)
	{
		return lockable->Lock();
	}

	inline void Unlock(Lockable *lockable)
	{
		lockable->Unlock();
	}
};

// AutoLockerReadLocking
template<typename Lockable>
class AutoLockerReadLocking {
public:
	inline bool Lock(Lockable *lockable)
	{
		return lockable->ReadLock();
	}

	inline void Unlock(Lockable *lockable)
	{
		lockable->ReadUnlock();
	}
};

// AutoLockerWriteLocking
template<typename Lockable>
class AutoLockerWriteLocking {
public:
	inline bool Lock(Lockable *lockable)
	{
		return lockable->WriteLock();
	}

	inline void Unlock(Lockable *lockable)
	{
		lockable->WriteUnlock();
	}
};

// AutoLocker
template<typename Lockable,
		 typename Locking = AutoLockerStandardLocking<Lockable> >
class AutoLocker {
private:
	typedef AutoLocker<Lockable, Locking>	ThisClass;
public:
	inline AutoLocker()
		: fLockable(NULL),
		  fLocked(false)
	{
	}

	inline AutoLocker(Lockable *lockable, bool alreadyLocked = false,
		bool lockIfNotLocked = true)
		: fLockable(lockable),
		  fLocked(fLockable && alreadyLocked)
	{
		if (!alreadyLocked && lockIfNotLocked)
			Lock();
	}

	inline AutoLocker(Lockable &lockable, bool alreadyLocked = false,
		bool lockIfNotLocked = true)
		: fLockable(&lockable),
		  fLocked(fLockable && alreadyLocked)
	{
		if (!alreadyLocked && lockIfNotLocked)
			Lock();
	}

	inline ~AutoLocker()
	{
		Unlock();
	}

	inline void SetTo(Lockable *lockable, bool alreadyLocked,
		bool lockIfNotLocked = true)
	{
		Unlock();
		fLockable = lockable;
		fLocked = alreadyLocked;
		if (!alreadyLocked && lockIfNotLocked)
			Lock();
	}

	inline void SetTo(Lockable &lockable, bool alreadyLocked,
		bool lockIfNotLocked = true)
	{
		SetTo(&lockable, alreadyLocked, lockIfNotLocked);
	}

	inline void Unset()
	{
		Unlock();
		Detach();
	}

	inline bool Lock()
	{
		if (fLockable && !fLocked)
			fLocked = fLocking.Lock(fLockable);
		return fLocked;
	}

	inline void Unlock()
	{
		if (fLockable && fLocked) {
			fLocking.Unlock(fLockable);
			fLocked = false;
		}
	}

	inline void Detach()
	{
		fLockable = NULL;
		fLocked = false;
	}

	inline AutoLocker<Lockable, Locking> &operator=(Lockable *lockable)
	{
		SetTo(lockable);
		return *this;
	}

	inline AutoLocker<Lockable, Locking> &operator=(Lockable &lockable)
	{
		SetTo(&lockable);
		return *this;
	}

	inline bool IsLocked() const	{ return fLocked; }

	inline operator bool() const	{ return fLocked; }

private:
	Lockable	*fLockable;
	bool		fLocked;
	Locking		fLocking;
};


}	// namespace FSShell

using FSShell::AutoLocker;

#endif	// _FSSH_AUTO_LOCKER_H
