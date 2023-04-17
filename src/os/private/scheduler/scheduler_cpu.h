/*
 * Copyright 2013, Paweł Dziepak, pdziepak@quarnos.org.
 * Distributed under the terms of the MIT License.
 */
#ifndef KERNEL_SCHEDULER_CPU_H
#define KERNEL_SCHEDULER_CPU_H


#include <OS.h>

#include <thread.h>
#include <util/AutoLock.h>
#include <util/Heap.h>
#include <util/MinMaxHeap.h>

#include <cpufreq.h>

#include <RunQueue.h>
#include <scheduler_common.h>
#include <scheduler_modes.h>
#include <scheduler_profiler.h>


namespace Scheduler {


class DebugDumper;

struct ThreadData;
class ThreadProcessing;

struct CPUEntry;
struct CoreEntry;
struct PackageEntry;

// The run queues. Holds the threads ready to run ordered by priority.
// One queue per schedulable target per core. Additionally, each
// logical processor has its sPinnedRunQueues used for scheduling
// pinned threads.
class ThreadRunQueue : public RunQueue<ThreadData, THREAD_MAX_SET_PRIORITY> {
public:
						void			Dump() const;
};

class CPUEntry : public HeapLinkImpl<CPUEntry, int32_t> {
public:
										CPUEntry();

						void			Init(int32_t id, CoreEntry* core);

	inline				int32_t			ID() const	{ return fCPUNumber; }
	inline				CoreEntry*		Core() const	{ return fCore; }

						void			Start();
						void			Stop();

	inline				void			EnterScheduler();
	inline				void			ExitScheduler();

	inline				void			LockScheduler();
	inline				void			UnlockScheduler();

	inline				void			LockRunQueue();
	inline				void			UnlockRunQueue();

						void			PushFront(ThreadData* thread,
											int32_t priority);
						void			PushBack(ThreadData* thread,
											int32_t priority);
						void			Remove(ThreadData* thread);
	inline				ThreadData*		PeekThread() const;
						ThreadData*		PeekIdleThread() const;

						void			UpdatePriority(int32_t priority);

	inline				int32_t			GetLoad() const	{ return fLoad; }
						void			ComputeLoad();

						ThreadData*		ChooseNextThread(ThreadData* oldThread,
											bool putAtBack);

						void			TrackActivity(ThreadData* oldThreadData,
											ThreadData* nextThreadData);

						void			StartQuantumTimer(ThreadData* thread,
											bool wasPreempted);

	static inline		CPUEntry*		GetCPU(int32_t cpu);

private:
						void			_RequestPerformanceLevel(
											ThreadData* threadData);

	static				int32_t			_RescheduleEvent(timer* /* unused */);
	static				int32_t			_UpdateLoadEvent(timer* /* unused */);

						int32_t			fCPUNumber;
						CoreEntry*		fCore;

						rw_spinlock 	fSchedulerModeLock;

						ThreadRunQueue	fRunQueue;
						spinlock		fQueueLock;

						int32_t			fLoad;

						bigtime_t		fMeasureActiveTime;
						bigtime_t		fMeasureTime;

						bool			fUpdateLoadEvent;

						friend class DebugDumper;
} CACHE_LINE_ALIGN;

class CPUPriorityHeap : public Heap<CPUEntry, int32_t> {
public:
										CPUPriorityHeap() { }
										CPUPriorityHeap(int32_t cpuCount);

						void			Dump();
};

class CoreEntry : public MinMaxHeapLinkImpl<CoreEntry, int32_t>,
	public DoublyLinkedListLinkImpl<CoreEntry> {
public:
										CoreEntry();

						void			Init(int32_t id, PackageEntry* package);

	inline				int32_t			ID() const	{ return fCoreID; }
	inline				PackageEntry*	Package() const	{ return fPackage; }
	inline				int32_t			CPUCount() const
											{ return fCPUCount; }

	inline				void			LockCPUHeap();
	inline				void			UnlockCPUHeap();

	inline				CPUPriorityHeap*	CPUHeap();

	inline				int32_t			ThreadCount() const;

	inline				void			LockRunQueue();
	inline				void			UnlockRunQueue();

						void			PushFront(ThreadData* thread,
											int32_t priority);
						void			PushBack(ThreadData* thread,
											int32_t priority);
						void			Remove(ThreadData* thread);
	inline				ThreadData*		PeekThread() const;

	inline				bigtime_t		GetActiveTime() const;
	inline				void			IncreaseActiveTime(
											bigtime_t activeTime);

	inline				int32_t			GetLoad() const;
	inline				uint32_t			LoadMeasurementEpoch() const
											{ return fLoadMeasurementEpoch; }

	inline				void			AddLoad(int32_t load, uint32_t epoch,
											bool updateLoad);
	inline				uint32_t			RemoveLoad(int32_t load, bool force);
	inline				void			ChangeLoad(int32_t delta);

	inline				void			CPUGoesIdle(CPUEntry* cpu);
	inline				void			CPUWakesUp(CPUEntry* cpu);

						void			AddCPU(CPUEntry* cpu);
						void			RemoveCPU(CPUEntry* cpu,
											ThreadProcessing&
												threadPostProcessing);

	static inline		CoreEntry*		GetCore(int32_t cpu);

private:
						void			_UpdateLoad(bool forceUpdate = false);

	static				void			_UnassignThread(Thread* thread,
											void* core);

						int32_t			fCoreID;
						PackageEntry*	fPackage;

						int32_t			fCPUCount;
						int32_t			fIdleCPUCount;
						CPUPriorityHeap	fCPUHeap;
						spinlock		fCPULock;

						int32_t			fThreadCount;
						ThreadRunQueue	fRunQueue;
						spinlock		fQueueLock;

						bigtime_t		fActiveTime;
	mutable				seqlock			fActiveTimeLock;

						int32_t			fLoad;
						int32_t			fCurrentLoad;
						uint32_t			fLoadMeasurementEpoch;
						bool			fHighLoad;
						bigtime_t		fLastLoadUpdate;
						rw_spinlock		fLoadLock;

						friend class DebugDumper;
} CACHE_LINE_ALIGN;

class CoreLoadHeap : public MinMaxHeap<CoreEntry, int32_t> {
public:
										CoreLoadHeap() { }
										CoreLoadHeap(int32_t coreCount);

						void			Dump();
};

// gPackageEntries are used to decide which core should be woken up from the
// idle state. When aiming for performance we should use as many packages as
// possible with as little cores active in each package as possible (so that the
// package can enter any boost mode if it has one and the active core have more
// of the shared cache for themselves. If power saving is the main priority we
// should keep active cores on as little packages as possible (so that other
// packages can go to the deep state of sleep). The heap stores only packages
// with at least one core active and one core idle. The packages with all cores
// idle are stored in gPackageIdleList (in LIFO manner).
class PackageEntry : public DoublyLinkedListLinkImpl<PackageEntry> {
public:
											PackageEntry();

						void				Init(int32_t id);

	inline				void				CoreGoesIdle(CoreEntry* core);
	inline				void				CoreWakesUp(CoreEntry* core);

	inline				CoreEntry*			GetIdleCore() const;

						void				AddIdleCore(CoreEntry* core);
						void				RemoveIdleCore(CoreEntry* core);

	static inline		PackageEntry*		GetMostIdlePackage();
	static inline		PackageEntry*		GetLeastIdlePackage();

private:
						int32_t				fPackageID;

						DoublyLinkedList<CoreEntry>	fIdleCores;
						int32_t				fIdleCoreCount;
						int32_t				fCoreCount;
						rw_spinlock			fCoreLock;

						friend class DebugDumper;
} CACHE_LINE_ALIGN;
typedef DoublyLinkedList<PackageEntry> IdlePackageList;

extern CPUEntry* gCPUEntries;

extern CoreEntry* gCoreEntries;
extern CoreLoadHeap gCoreLoadHeap;
extern CoreLoadHeap gCoreHighLoadHeap;
extern rw_spinlock gCoreHeapsLock;
extern int32_t gCoreCount;

extern PackageEntry* gPackageEntries;
extern IdlePackageList gIdlePackageList;
extern rw_spinlock gIdlePackageLock;
extern int32_t gPackageCount;


inline void
CPUEntry::EnterScheduler()
{
	SCHEDULER_ENTER_FUNCTION();
	acquire_read_spinlock(&fSchedulerModeLock);
}


inline void
CPUEntry::ExitScheduler()
{
	SCHEDULER_ENTER_FUNCTION();
	release_read_spinlock(&fSchedulerModeLock);
}


inline void
CPUEntry::LockScheduler()
{
	SCHEDULER_ENTER_FUNCTION();
	acquire_write_spinlock(&fSchedulerModeLock);
}


inline void
CPUEntry::UnlockScheduler()
{
	SCHEDULER_ENTER_FUNCTION();
	release_write_spinlock(&fSchedulerModeLock);
}


inline void
CPUEntry::LockRunQueue()
{
	SCHEDULER_ENTER_FUNCTION();
	acquire_spinlock(&fQueueLock);
}


inline void
CPUEntry::UnlockRunQueue()
{
	SCHEDULER_ENTER_FUNCTION();
	release_spinlock(&fQueueLock);
}


/* static */ inline CPUEntry*
CPUEntry::GetCPU(int32_t cpu)
{
	SCHEDULER_ENTER_FUNCTION();
	return &gCPUEntries[cpu];
}


inline void
CoreEntry::LockCPUHeap()
{
	SCHEDULER_ENTER_FUNCTION();
	acquire_spinlock(&fCPULock);
}


inline void
CoreEntry::UnlockCPUHeap()
{
	SCHEDULER_ENTER_FUNCTION();
	release_spinlock(&fCPULock);
}


inline CPUPriorityHeap*
CoreEntry::CPUHeap()
{
	SCHEDULER_ENTER_FUNCTION();
	return &fCPUHeap;
}


inline int32_t
CoreEntry::ThreadCount() const
{
	SCHEDULER_ENTER_FUNCTION();
	return fThreadCount + fCPUCount - fIdleCPUCount;
}


inline void
CoreEntry::LockRunQueue()
{
	SCHEDULER_ENTER_FUNCTION();
	acquire_spinlock(&fQueueLock);
}


inline void
CoreEntry::UnlockRunQueue()
{
	SCHEDULER_ENTER_FUNCTION();
	release_spinlock(&fQueueLock);
}


inline void
CoreEntry::IncreaseActiveTime(bigtime_t activeTime)
{
	SCHEDULER_ENTER_FUNCTION();
	WriteSequentialLocker _(fActiveTimeLock);
	fActiveTime += activeTime;
}


inline bigtime_t
CoreEntry::GetActiveTime() const
{
	SCHEDULER_ENTER_FUNCTION();

	bigtime_t activeTime;
	uint32_t count;
	do {
		count = acquire_read_seqlock(&fActiveTimeLock);
		activeTime = fActiveTime;
	} while (!release_read_seqlock(&fActiveTimeLock, count));
	return activeTime;
}


inline int32_t
CoreEntry::GetLoad() const
{
	SCHEDULER_ENTER_FUNCTION();

	ASSERT(fCPUCount > 0);
	return fLoad / fCPUCount;
}


inline void
CoreEntry::AddLoad(int32_t load, uint32_t epoch, bool updateLoad)
{
	SCHEDULER_ENTER_FUNCTION();

	ASSERT(gTrackCoreLoad);
	ASSERT(load >= 0 && load <= kMaxLoad);

	ReadSpinLocker locker(fLoadLock);
	atomic_add(&fCurrentLoad, load);
	if (fLoadMeasurementEpoch != epoch)
		atomic_add(&fLoad, load);
	locker.Unlock();

	if (updateLoad)
		_UpdateLoad(true);
}


inline uint32_t
CoreEntry::RemoveLoad(int32_t load, bool force)
{
	SCHEDULER_ENTER_FUNCTION();

	ASSERT(gTrackCoreLoad);
	ASSERT(load >= 0 && load <= kMaxLoad);

	ReadSpinLocker locker(fLoadLock);
	atomic_add(&fCurrentLoad, -load);
	if (force) {
		atomic_add(&fLoad, -load);
		locker.Unlock();

		_UpdateLoad(true);
	}
	return fLoadMeasurementEpoch;
}


inline void
CoreEntry::ChangeLoad(int32_t delta)
{
	SCHEDULER_ENTER_FUNCTION();

	ASSERT(gTrackCoreLoad);
	ASSERT(delta >= -kMaxLoad && delta <= kMaxLoad);

	if (delta != 0) {
		ReadSpinLocker locker(fLoadLock);
		atomic_add(&fCurrentLoad, delta);
		atomic_add(&fLoad, delta);
	}

	_UpdateLoad();
}


/* PackageEntry::CoreGoesIdle and PackageEntry::CoreWakesUp have to be defined
   before CoreEntry::CPUGoesIdle and CoreEntry::CPUWakesUp. If they weren't
   GCC2 wouldn't inline them as, apparently, it doesn't do enough optimization
   passes.
*/
inline void
PackageEntry::CoreGoesIdle(CoreEntry* core)
{
	SCHEDULER_ENTER_FUNCTION();

	WriteSpinLocker _(fCoreLock);

	ASSERT(fIdleCoreCount >= 0);
	ASSERT(fIdleCoreCount < fCoreCount);

	fIdleCoreCount++;
	fIdleCores.Add(core);

	if (fIdleCoreCount == fCoreCount) {
		// package goes idle
		WriteSpinLocker _(gIdlePackageLock);
		gIdlePackageList.Add(this);
	}
}


inline void
PackageEntry::CoreWakesUp(CoreEntry* core)
{
	SCHEDULER_ENTER_FUNCTION();

	WriteSpinLocker _(fCoreLock);

	ASSERT(fIdleCoreCount > 0);
	ASSERT(fIdleCoreCount <= fCoreCount);

	fIdleCoreCount--;
	fIdleCores.Remove(core);

	if (fIdleCoreCount + 1 == fCoreCount) {
		// package wakes up
		WriteSpinLocker _(gIdlePackageLock);
		gIdlePackageList.Remove(this);
	}
}


inline void
CoreEntry::CPUGoesIdle(CPUEntry* /* cpu */)
{
	if (gSingleCore)
		return;

	ASSERT(fIdleCPUCount < fCPUCount);
	if (++fIdleCPUCount == fCPUCount)
		fPackage->CoreGoesIdle(this);
}


inline void
CoreEntry::CPUWakesUp(CPUEntry* /* cpu */)
{
	if (gSingleCore)
		return;

	ASSERT(fIdleCPUCount > 0);
	if (fIdleCPUCount-- == fCPUCount)
		fPackage->CoreWakesUp(this);
}


/* static */ inline CoreEntry*
CoreEntry::GetCore(int32_t cpu)
{
	SCHEDULER_ENTER_FUNCTION();
	return gCPUEntries[cpu].Core();
}


inline CoreEntry*
PackageEntry::GetIdleCore() const
{
	SCHEDULER_ENTER_FUNCTION();
	return fIdleCores.Last();
}


/* static */ inline PackageEntry*
PackageEntry::GetMostIdlePackage()
{
	SCHEDULER_ENTER_FUNCTION();

	PackageEntry* current = &gPackageEntries[0];
	for (int32_t i = 1; i < gPackageCount; i++) {
		if (gPackageEntries[i].fIdleCoreCount > current->fIdleCoreCount)
			current = &gPackageEntries[i];
	}

	if (current->fIdleCoreCount == 0)
		return NULL;

	return current;
}


/* static */ inline PackageEntry*
PackageEntry::GetLeastIdlePackage()
{
	SCHEDULER_ENTER_FUNCTION();

	PackageEntry* package = NULL;

	for (int32_t i = 0; i < gPackageCount; i++) {
		PackageEntry* current = &gPackageEntries[i];

		int32_t currentIdleCoreCount = current->fIdleCoreCount;
		if (currentIdleCoreCount != 0 && (package == NULL
				|| currentIdleCoreCount < package->fIdleCoreCount)) {
			package = current;
		}
	}

	return package;
}


}	// namespace Scheduler


#endif	// KERNEL_SCHEDULER_CPU_H

