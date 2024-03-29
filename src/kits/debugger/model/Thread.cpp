/*
 * Copyright 2013-2016, Rene Gollent, rene@gollent.com.
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */

#include <model/Thread.h>

#include <stdio.h>

#include <CpuState.h>
#include <StackTrace.h>
#include <Team.h>


Thread::Thread(Team* team, thread_id threadID)
	:
	fTeam(team),
	fID(threadID),
	fState(THREAD_STATE_UNKNOWN),
	fReturnValueInfos(NULL),
	fStopRequestPending(false),
	fStoppedReason(THREAD_STOPPED_UNKNOWN),
	fCpuState(NULL),
	fStackTrace(NULL)
{
}


Thread::~Thread()
{
	if (fCpuState != NULL)
		fCpuState->ReleaseReference();
	if (fStackTrace != NULL)
		fStackTrace->ReleaseReference();

	ClearReturnValueInfos();
	delete fReturnValueInfos;
}


status_t Thread::Init()
{
	fReturnValueInfos = new(std::nothrow) ReturnValueInfoList;
	if (fReturnValueInfos == NULL)
		return B_NO_MEMORY;

	return B_OK;
}


bool Thread::IsMainThread() const
{
	return fID == fTeam->ID();
}


void Thread::SetName(const BString& name)
{
	fName = name;
}


void Thread::SetState(uint32_t state, uint32_t reason, const BString& info)
{
	if (state == fState && reason == fStoppedReason)
		return;

	fState = state;
	fStoppedReason = reason;
	fStoppedReasonInfo = info;

	// unset CPU state and stack trace, if the thread isn't stopped
	if (fState != THREAD_STATE_STOPPED) {
		SetCpuState(NULL);
		SetStackTrace(NULL);
		ClearReturnValueInfos();
		fStopRequestPending = false;
	}

	fTeam->NotifyThreadStateChanged(this);
}


void Thread::SetCpuState(CpuState* state)
{
	if (state == fCpuState)
		return;

	if (fCpuState != NULL)
		fCpuState->ReleaseReference();

	fCpuState = state;

	if (fCpuState != NULL)
		fCpuState->AcquireReference();

	fTeam->NotifyThreadCpuStateChanged(this);
}


void Thread::SetStackTrace(StackTrace* trace)
{
	if (trace == fStackTrace)
		return;

	if (fStackTrace != NULL)
		fStackTrace->ReleaseReference();

	fStackTrace = trace;

	if (fStackTrace != NULL)
		fStackTrace->AcquireReference();

	fTeam->NotifyThreadStackTraceChanged(this);
}

void Thread::SetStopRequestPending()
{
	fStopRequestPending = true;
}


status_t Thread::AddReturnValueInfo(ReturnValueInfo* info)
{
	if (!fReturnValueInfos->AddItem(info))
		return B_NO_MEMORY;

	info->AcquireReference();
	return B_OK;
}


void Thread::ClearReturnValueInfos()
{
	for (int32_t i = 0; i < fReturnValueInfos->CountItems(); i++)
		fReturnValueInfos->ItemAt(i)->ReleaseReference();

	fReturnValueInfos->MakeEmpty();
}
