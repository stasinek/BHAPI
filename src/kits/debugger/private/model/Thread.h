/*
 * Copyright 2013-2016, Rene Gollent, rene@gollent.com.
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef THREAD_H
#define THREAD_H

#include <kits/kernel/OS.h>
#include <kits/support/String.h>

#include <Referenceable.h>
#include <util/DoublyLinkedList.h>

#include <ReturnValueInfo.h>
#include <types/Types.h>


class CpuState;
class StackTrace;
class Team;


// general thread state
enum {
	THREAD_STATE_UNKNOWN,
	THREAD_STATE_RUNNING,
	THREAD_STATE_STOPPED
};

// reason why stopped
enum {
	THREAD_STOPPED_UNKNOWN,
	THREAD_STOPPED_DEBUGGED,
	THREAD_STOPPED_DEBUGGER_CALL,
	THREAD_STOPPED_BREAKPOINT,
	THREAD_STOPPED_WATCHPOINT,
	THREAD_STOPPED_SINGLE_STEP,
	THREAD_STOPPED_EXCEPTION
};


class Thread : public BReferenceable,
	public DoublyLinkedListLinkImpl< ::Thread> {
public:
								Thread(Team* team, thread_id threadID);
								~Thread();

			status_t			Init();

			Team*				GetTeam() const	{ return fTeam; }
			thread_id			ID() const		{ return fID; }

			bool				IsMainThread() const;

			const char*			Name() const	{ return fName.String(); }
			void				SetName(const BString& name);

			uint32_t				State() const	{ return fState; }
			void				SetState(uint32_t state,
									uint32_t reason = THREAD_STOPPED_UNKNOWN,
									const BString& info = BString());

			uint32_t				StoppedReason() const
									{ return fStoppedReason; }
			const BString&		StoppedReasonInfo() const
									{ return fStoppedReasonInfo; }

			CpuState*			GetCpuState() const	{ return fCpuState; }
			void				SetCpuState(CpuState* state);

			StackTrace*			GetStackTrace() const	{ return fStackTrace; }
			void				SetStackTrace(StackTrace* trace);

			bool				StopRequestPending() const
									{ return fStopRequestPending; }
			void				SetStopRequestPending();

			ReturnValueInfoList*
								ReturnValueInfos() const
								{ return fReturnValueInfos; }
			status_t			AddReturnValueInfo(ReturnValueInfo* info);
			void				ClearReturnValueInfos();

private:
			Team*				fTeam;
			thread_id			fID;
			BString				fName;
			uint32_t				fState;
			ReturnValueInfoList*
								fReturnValueInfos;
			bool				fStopRequestPending;
			uint32_t				fStoppedReason;
			BString				fStoppedReasonInfo;
			CpuState*			fCpuState;
			StackTrace*			fStackTrace;
};


typedef DoublyLinkedList< ::Thread> ThreadList;


#endif	// THREAD_H
