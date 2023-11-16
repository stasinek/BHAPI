/*
 * Copyright 2014, Paweł Dziepak, pdziepak@quarnos.org.
 * Copyright 2011, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef _KERNEL_USER_EVENT_H
#define _KERNEL_USER_EVENT_H


#include <signal.h>

#include <Haiku.h>

#include <DPC.h>
#include <thread.h>


namespace BKernel {


struct Team;
struct Thread;


struct UserEvent : public BReferenceable {
	virtual						~UserEvent();

	virtual	status_t			Fire() = 0;
};


struct SignalEvent : UserEvent, private DPCCallback {
	virtual						~SignalEvent();

			void				SetUserValue(union sigval userValue);

	virtual	status_t			Fire();

protected:
			struct EventSignal;

protected:
								SignalEvent(EventSignal* signal);

protected:
			EventSignal*		fSignal;
			int32_t				fPendingDPC;
};


struct TeamSignalEvent : SignalEvent {
	static	TeamSignalEvent*	Create(Team* team, uint32_t signalNumber,
									int32_t signalCode, int32_t errorCode);

	virtual	status_t			Fire();

protected:
	virtual	void				DoDPC(DPCQueue* queue);

private:
								TeamSignalEvent(Team* team,
									EventSignal* signal);

private:
			Team*				fTeam;
};


struct ThreadSignalEvent : SignalEvent {
	static	ThreadSignalEvent*	Create(Thread* thread, uint32_t signalNumber,
									int32_t signalCode, int32_t errorCode,
									pid_t sendingTeam);

	virtual	status_t			Fire();

protected:
	virtual	void				DoDPC(DPCQueue* queue);

private:
								ThreadSignalEvent(Thread* thread,
									EventSignal* signal);

private:
			Thread*				fThread;
};


struct CreateThreadEvent : UserEvent, private DPCCallback {
	static	CreateThreadEvent*	Create(
									const ThreadCreationAttributes& attributes);

	virtual	status_t			Fire();

private:
								CreateThreadEvent(
									const ThreadCreationAttributes& attributes);

	virtual	void				DoDPC(DPCQueue* queue);

private:
			ThreadCreationAttributes fCreationAttributes;
			char				fThreadName[B_OS_NAME_LENGTH];
			int32_t				fPendingDPC;
};


}	// namespace BKernel


using BKernel::CreateThreadEvent;
using BKernel::SignalEvent;
using BKernel::TeamSignalEvent;
using BKernel::ThreadSignalEvent;
using BKernel::UserEvent;


#endif	// _KERNEL_USER_EVENT_H
