/*
 * Copyright 2011, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Copyright 2003-2008, Axel Dörfler, axeld@pinc-software.de.
 * All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_KERNEL_SIGNAL_H
#define BHAPI_KERNEL_SIGNAL_H
#include <signal.h>
//#include <KernelExport.h>
#include <signal_defs.h>

#include <heap.h>
#include <util/DoublyLinkedList.h>
#include <util/KernelReferenceable.h>
#include <os/kernel.h>
#include <syscalls.h>

namespace BKernel {
    struct ProcessGroup;
    struct Team;
    struct Thread;
}

using BKernel::ProcessGroup;
using BKernel::Team;
using BKernel::Thread;

#define KILL_SIGNALS	\
    (((sigset_t)1 << (SIGKILL - 1)) | ((sigset_t)1 << (SIGKILLTHR - 1)))

#define SYSCALL_RESTART_PARAMETER_SIZE	32

// kernel-internal signals
#define SIGNAL_CANCEL_THREAD	63
    // Cancel a thread. Non-blockable.
#define SIGNAL_CONTINUE_THREAD	64
    // Continue a thread. Used by resume_thread(). Non-blockable, prevents
    // syscall restart.


struct signal_frame_data {
    siginfo_t	info;
    ucontext_t	context;
    void*		user_data;
    void*		handler;
    bool		siginfo_handler;
    int32_t		thread_flags;
    uint64		syscall_restart_return_value;
    uint8		syscall_restart_parameters[SYSCALL_RESTART_PARAMETER_SIZE];
    void*		commpage_address;
};


namespace BKernel {


struct QueuedSignalsCounter : BReferenceable {
                                QueuedSignalsCounter(int32_t limit);

            bool				Increment();
            void				Decrement()		{ ReleaseReference(); }

private:
            int32_t				fLimit;
};


struct Signal : KernelReferenceable, DoublyLinkedListLinkImpl<Signal> {
public:
                                Signal();
                                    // cheap no-init constructor
                                Signal(const Signal& other);
                                Signal(uint32_t number, int32_t signalCode,
                                    int32_t errorCode, pid_t sendingProcess);
    virtual						~Signal();

    static	status_t			CreateQueuable(const Signal& signal,
                                    bool queuingRequired,
                                    Signal*& _signalToQueue);

            void				SetTo(uint32_t number);

            uint32_t				Number() const { return fNumber; }
            void				SetNumber(uint32_t number)
                                    { fNumber = number; }

            int32_t				Priority() const;

            int32_t				SignalCode() const
                                    { return fSignalCode; }
            int32_t				ErrorCode() const
                                    { return fErrorCode; }
            pid_t				SendingProcess() const
                                    { return fSendingProcess; }

            uid_t				SendingUser() const
                                    { return fSendingUser; }
            void				SetSendingUser(uid_t user)
                                    { fSendingUser = user; }

            int32_t				Status() const
                                    { return fStatus; }
            void				SetStatus(int32_t status)
                                    { fStatus = status; }

            int32_t				PollBand() const
                                    { return fPollBand; }
            void				SetPollBand(int32_t pollBand)
                                    { fPollBand = pollBand; }

            void*				Address() const
                                    { return fAddress; }
            void				SetAddress(void* address)
                                    { fAddress = address; }

            union sigval		UserValue() const
                                    { return fUserValue; }
            void				SetUserValue(union sigval userValue)
                                    { fUserValue = userValue; }

            bool				IsPending() const
                                    { return fPending; }
            void				SetPending(bool pending)
                                    { fPending = pending; }

    virtual	void				Handled();

protected:
    virtual	void				LastReferenceReleased();

private:
            QueuedSignalsCounter* fCounter;
            uint32_t				fNumber;
            int32_t				fSignalCode;
            int32_t				fErrorCode;	// error code associated with the
                                            // signal
            pid_t				fSendingProcess;
            uid_t				fSendingUser;
            int32_t				fStatus;	// exit value
            int32_t				fPollBand;	// for SIGPOLL
            void*				fAddress;
            union sigval		fUserValue;
            bool				fPending;
};


struct PendingSignals {
                                PendingSignals();
                                ~PendingSignals();

            sigset_t			AllSignals() const
                                    { return fQueuedSignalsMask
                                        | fUnqueuedSignalsMask; }

            int32_t				HighestSignalPriority(sigset_t nonBlocked)
                                    const;

            void				Clear();
            void				AddSignal(int32_t signal)
                                    { fUnqueuedSignalsMask
                                        |= SIGNAL_TO_MASK(signal); }
            void				AddSignal(Signal* signal);
            void				RemoveSignal(int32_t signal)
                                    { RemoveSignals(SIGNAL_TO_MASK(signal)); }
            void				RemoveSignal(Signal* signal);
            void				RemoveSignals(sigset_t mask);

            Signal*				DequeueSignal(sigset_t nonBlocked,
                                    Signal& buffer);

private:
            typedef DoublyLinkedList<Signal> SignalList;

private:
            int32_t				_GetHighestPrioritySignal(sigset_t nonBlocked,
                                    Signal*& _queuedSignal,
                                    int32_t& _unqueuedSignal) const;
            void				_UpdateQueuedSignalMask();

private:
            sigset_t			fQueuedSignalsMask;
            sigset_t			fUnqueuedSignalsMask;
            SignalList			fQueuedSignals;
};


}	// namespace BKernel


using BKernel::PendingSignals;
using BKernel::QueuedSignalsCounter;
using BKernel::Signal;


#ifdef __cplusplus
extern "C" {
#endif

void handle_signals(Thread* thread);
bool is_team_signal_blocked(Team* team, int signal);
void signal_get_user_stack(addr_t address, stack_t* stack);

status_t send_signal_to_thread_locked(Thread* thread, uint32_t signalNumber,
    Signal* signal, uint32_t flags);
status_t send_signal_to_thread(Thread* thread, const Signal& signal,
    uint32_t flags);
status_t send_signal_to_thread_id(thread_id threadID, const Signal& signal,
    uint32_t flags);

status_t send_signal_to_team_locked(Team* team, uint32_t signalNumber,
    Signal* signal, uint32_t flags);
status_t send_signal_to_team(Team* team, const Signal& signal, uint32_t flags);
status_t send_signal_to_team_id(team_id teamID, const Signal& signal,
    uint32_t flags);

status_t send_signal_to_process_group_locked(ProcessGroup* group,
    const Signal& signal, uint32_t flags);
status_t send_signal_to_process_group(pid_t groupID, const Signal& signal,
    uint32_t flags);

status_t _user_send_signal(int32_t id, uint32_t signal,
    const union sigval* userValue, uint32_t flags);
status_t _user_set_signal_mask(int how, const sigset_t *set, sigset_t *oldSet);
status_t _user_sigaction(int sig, const struct sigaction *newAction,
    struct sigaction *oldAction);
bigtime_t _user_set_alarm(bigtime_t time, uint32_t mode);
status_t _user_sigwait(const sigset_t *set, siginfo_t *info, uint32_t flags,
    bigtime_t timeout);
status_t _user_sigsuspend(const sigset_t *mask);
status_t _user_sigpending(sigset_t *set);
status_t _user_set_signal_stack(const stack_t *newUserStack,
    stack_t *oldUserStack);
int64 _user_restore_signal_frame(struct signal_frame_data* signalFrameData);

#ifdef __cplusplus
}
#endif

#endif	/* _KERNEL_SIGNAL_H */
