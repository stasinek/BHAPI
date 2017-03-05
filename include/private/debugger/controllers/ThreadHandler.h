/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Copyright 2014-2016, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef THREAD_HANDLER_H
#define THREAD_HANDLER_H


#include <Referenceable.h>
#include <util/OpenHashTable.h>

#include "Breakpoint.h"
#include "ImageDebugInfoProvider.h"
#include "model/Thread.h"


class BreakpointHitEvent;
class BreakpointManager;
class DebugEvent;
class DebuggerCallEvent;
class DebuggerInterface;
class ExceptionOccurredEvent;
class ExpressionResult;
class ImageDebugInfoJobListener;
class JobListener;
class SignalReceivedEvent;
class SingleStepEvent;
class StackFrame;
class Statement;
class ThreadDebuggedEvent;
class WatchpointHitEvent;
class Worker;


class ThreadHandler : public BReferenceable, private ImageDebugInfoProvider,
	private BreakpointClient {
public:
								ThreadHandler(::Thread* thread, Worker* worker,
									DebuggerInterface* debuggerInterface,
									JobListener* listener,
									BreakpointManager* breakpointManager);
								~ThreadHandler();

			void				Init();

			thread_id			ThreadID() const	{ return fThread->ID(); }
			::Thread*			GetThread() const	{ return fThread; }

			status_t			SetBreakpointAndRun(target_addr_t address);
									// team lock held

			// All Handle*() methods are invoked in team debugger thread,
			// looper lock held.
			bool				HandleThreadDebugged(
									ThreadDebuggedEvent* event,
									const BString& stoppedReason = BString());
			bool				HandleDebuggerCall(
									DebuggerCallEvent* event);
			bool				HandleBreakpointHit(
									BreakpointHitEvent* event);
			bool				HandleWatchpointHit(
									WatchpointHitEvent* event);
			bool				HandleSingleStep(
									SingleStepEvent* event);
			bool				HandleExceptionOccurred(
									ExceptionOccurredEvent* event);
			bool				HandleSignalReceived(
									SignalReceivedEvent* event);

			void				HandleThreadAction(uint32 action,
									target_addr_t address);

			void				HandleThreadStateChanged();
			void				HandleCpuStateChanged();
			void				HandleStackTraceChanged();

private:
			friend class ExpressionEvaluationListener;

private:
	// ImageDebugInfoProvider
	virtual	status_t			GetImageDebugInfo(Image* image,
									ImageDebugInfo*& _info);

			bool				_HandleThreadStopped(CpuState* cpuState,
									uint32 stoppedReason,
									const BString& stoppedReasonInfo
										= BString());

			bool				_HandleSetAddress(CpuState* cpuState,
									target_addr_t address);

			void				_SetThreadState(uint32 state,
									CpuState* cpuState, uint32 stoppedReason,
									const BString& stoppedReasonInfo);

			Statement*			_GetStatementAtInstructionPointer(
									StackFrame* frame);

			void				_StepFallback();
			bool				_DoStepOver(CpuState* cpuState);

			status_t			_InstallTemporaryBreakpoint(
									target_addr_t address);
			void				_UninstallTemporaryBreakpoint();
			void				_ClearContinuationState();
			void				_RunThread(target_addr_t instructionPointer);
			void				_SingleStepThread(
									target_addr_t instructionPointer);

			bool				_HandleBreakpointConditionIfNeeded(
									CpuState* cpuState);
			void				_HandleBreakpointConditionEvaluated(
									ExpressionResult* value);
			bool				_CheckStopCondition();

			bool				_HandleBreakpointHitStep(CpuState* cpuState);
			bool				_HandleSingleStepStep(CpuState* cpuState);

			bool				_HasExitedFrame(target_addr_t framePointer)
									const;

private:
			::Thread*			fThread;
			Worker*				fWorker;
			DebuggerInterface*	fDebuggerInterface;
			JobListener*		fJobListener;
			BreakpointManager*	fBreakpointManager;
			uint32				fStepMode;
			Statement*			fStepStatement;
			target_addr_t		fBreakpointAddress;
			target_addr_t		fSteppedOverFunctionAddress;
			target_addr_t		fPreviousInstructionPointer;
			target_addr_t		fPreviousFrameAddress;
			bool				fSingleStepping;
			sem_id				fConditionWaitSem;
			ExpressionResult*	fConditionResult;

public:
			ThreadHandler*		fNext;
};


struct ThreadHandlerHashDefinition {
	typedef thread_id		KeyType;
	typedef	ThreadHandler	ValueType;

	size_t HashKey(thread_id key) const
	{
		return (size_t)key;
	}

	size_t Hash(const ThreadHandler* value) const
	{
		return HashKey(value->ThreadID());
	}

	bool Compare(thread_id key, ThreadHandler* value) const
	{
		return value->ThreadID() == key;
	}

	ThreadHandler*& GetLink(ThreadHandler* value) const
	{
		return value->fNext;
	}
};

typedef BOpenHashTable<ThreadHandlerHashDefinition> ThreadHandlerTable;


#endif	// THREAD_HANDLER_H
