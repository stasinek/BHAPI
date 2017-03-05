/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Copyright 2011-2016, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef JOBS_H
#define JOBS_H

#include <Entry.h>
#include <String.h>

#include "ImageDebugInfoLoadingState.h"
#include "ImageDebugInfoProvider.h"
#include "Types.h"
#include "Worker.h"


class Architecture;
class BVariant;
class CpuState;
class DebuggerInterface;
class ExpressionInfo;
class ExpressionResult;
class Function;
class FunctionInstance;
class Image;
class SourceLanguage;
class StackFrame;
class StackFrameValues;
class Team;
class TeamMemory;
class TeamMemoryBlock;
class TeamTypeInformation;
class Thread;
class Type;
class TypeComponentPath;
class Value;
class ValueLocation;
class ValueNode;
class ValueNodeChild;
class ValueNodeContainer;
class ValueNodeManager;
class Variable;


// job types
enum {
	JOB_TYPE_GET_THREAD_STATE,
	JOB_TYPE_GET_CPU_STATE,
	JOB_TYPE_GET_STACK_TRACE,
	JOB_TYPE_LOAD_IMAGE_DEBUG_INFO,
	JOB_TYPE_LOAD_SOURCE_CODE,
	JOB_TYPE_GET_STACK_FRAME_VALUE,
	JOB_TYPE_RESOLVE_VALUE_NODE_VALUE,
	JOB_TYPE_WRITE_VALUE_NODE_VALUE,
	JOB_TYPE_GET_MEMORY_BLOCK,
	JOB_TYPE_WRITE_MEMORY,
	JOB_TYPE_EVALUATE_EXPRESSION,
	JOB_TYPE_WRITE_CORE_FILE
};


class GetThreadStateJob : public Job {
public:
								GetThreadStateJob(
									DebuggerInterface* debuggerInterface,
									Thread* thread);
	virtual						~GetThreadStateJob();

	virtual	const JobKey&		Key() const;
	virtual	status_t			Do();

private:
			SimpleJobKey		fKey;
			DebuggerInterface*	fDebuggerInterface;
			Thread*				fThread;
};


class GetCpuStateJob : public Job {
public:
								GetCpuStateJob(
									DebuggerInterface* debuggerInterface,
									::Thread* thread);
	virtual						~GetCpuStateJob();

	virtual	const JobKey&		Key() const;
	virtual	status_t			Do();

private:
			SimpleJobKey		fKey;
			DebuggerInterface*	fDebuggerInterface;
			::Thread*			fThread;
};


class GetStackTraceJob : public Job, private ImageDebugInfoProvider {
public:
								GetStackTraceJob(
									DebuggerInterface* debuggerInterface,
									JobListener* jobListener,
									Architecture* architecture,
									::Thread* thread);
	virtual						~GetStackTraceJob();

	virtual	const JobKey&		Key() const;
	virtual	status_t			Do();

private:
	// ImageDebugInfoProvider
	virtual	status_t			GetImageDebugInfo(Image* image,
									ImageDebugInfo*& _info);

private:
			SimpleJobKey		fKey;
			DebuggerInterface*	fDebuggerInterface;
			JobListener*		fJobListener;
			Architecture*		fArchitecture;
			::Thread*			fThread;
			CpuState*			fCpuState;
};


class LoadImageDebugInfoJob : public Job {
public:
								LoadImageDebugInfoJob(Image* image);
	virtual						~LoadImageDebugInfoJob();

	virtual	const JobKey&		Key() const;
	virtual	status_t			Do();

	static	status_t			ScheduleIfNecessary(Worker* worker,
									Image* image,
									JobListener* jobListener,
									ImageDebugInfo** _imageDebugInfo = NULL);
										// If already loaded returns a
										// reference, if desired. If not loaded
										// schedules a job, but does not wait;
										// returns B_OK and NULL. An error,
										// if scheduling the job failed, or the
										// debug info already failed to load
										// earlier.

			ImageDebugInfoLoadingState*
									GetLoadingState()
										{ return &fState; }

private:
			SimpleJobKey		fKey;
			Image*				fImage;
			ImageDebugInfoLoadingState
								fState;
};


class LoadSourceCodeJob : public Job {
public:
								LoadSourceCodeJob(
									DebuggerInterface* debuggerInterface,
									Architecture* architecture, Team* team,
									FunctionInstance* functionInstance,
									bool loadForFunction);
	virtual						~LoadSourceCodeJob();

	virtual	const JobKey&		Key() const;
	virtual	status_t			Do();

private:
			SimpleJobKey		fKey;
			DebuggerInterface*	fDebuggerInterface;
			Architecture*		fArchitecture;
			Team*				fTeam;
			FunctionInstance*	fFunctionInstance;
			bool				fLoadForFunction;
};


class ResolveValueNodeValueJob : public Job {
public:
								ResolveValueNodeValueJob(
									DebuggerInterface* debuggerInterface,
									Architecture* architecture,
									CpuState* cpuState,
									TeamTypeInformation* typeInformation,
									ValueNodeContainer*	container,
									ValueNode* valueNode);
	virtual						~ResolveValueNodeValueJob();

	virtual	const JobKey&		Key() const;
	virtual	status_t			Do();

private:
			status_t			_ResolveNodeValue();
			status_t			_ResolveNodeChildLocation(
									ValueNodeChild* nodeChild);
			status_t			_ResolveParentNodeValue(ValueNode* parentNode);


private:
			SimpleJobKey		fKey;
			DebuggerInterface*	fDebuggerInterface;
			Architecture*		fArchitecture;
			CpuState*			fCpuState;
			TeamTypeInformation*
								fTypeInformation;
			ValueNodeContainer*	fContainer;
			ValueNode*			fValueNode;
};


class WriteValueNodeValueJob : public Job {
public:
								WriteValueNodeValueJob(
									DebuggerInterface* debuggerInterface,
									Architecture* architecture,
									CpuState* cpuState,
									TeamTypeInformation* typeInformation,
									ValueNode* valueNode,
									Value* newValue);
	virtual						~WriteValueNodeValueJob();

	virtual	const JobKey&		Key() const;
	virtual	status_t			Do();

private:
			SimpleJobKey		fKey;
			DebuggerInterface*	fDebuggerInterface;
			Architecture*		fArchitecture;
			CpuState*			fCpuState;
			TeamTypeInformation*
								fTypeInformation;
			ValueNode*			fValueNode;
			Value*				fNewValue;
};


class RetrieveMemoryBlockJob : public Job {
public:
								RetrieveMemoryBlockJob(Team* team,
									TeamMemory* teamMemory,
									TeamMemoryBlock* memoryBlock);
	virtual						~RetrieveMemoryBlockJob();

	virtual const JobKey&		Key() const;
	virtual status_t			Do();

private:
			SimpleJobKey		fKey;
			Team*				fTeam;
			TeamMemory*			fTeamMemory;
			TeamMemoryBlock*	fMemoryBlock;
};


class WriteMemoryJob : public Job {
public:
								WriteMemoryJob(Team* team,
									TeamMemory* teamMemory,
									target_addr_t address, void* data,
									target_size_t size);
	virtual						~WriteMemoryJob();

	virtual const JobKey&		Key() const;
	virtual status_t			Do();

private:
			SimpleJobKey		fKey;
			Team*				fTeam;
			TeamMemory*			fTeamMemory;
			target_addr_t		fTargetAddress;
			void*				fData;
			target_size_t		fSize;
};


class ExpressionEvaluationJob : public Job {
public:
								ExpressionEvaluationJob(Team* team,
									DebuggerInterface* debuggerInterface,
									SourceLanguage* language,
									ExpressionInfo* info,
									StackFrame* frame,
									Thread* thread);
	virtual						~ExpressionEvaluationJob();

	virtual	const JobKey&		Key() const;
	virtual	status_t			Do();

			ExpressionResult*	GetResult() const { return fResultValue; }

private:
			status_t			ResolveNodeValue(ValueNode* node);

private:
			SimpleJobKey		fKey;
			Team*				fTeam;
			DebuggerInterface*	fDebuggerInterface;
			Architecture*		fArchitecture;
			TeamTypeInformation* fTypeInformation;
			SourceLanguage*		fLanguage;
			ExpressionInfo*		fExpressionInfo;
			StackFrame*			fFrame;
			Thread*				fThread;
			ValueNodeManager*	fManager;
			ExpressionResult*	fResultValue;
};


class WriteCoreFileJob : public Job {
public:
								WriteCoreFileJob(Team* team,
									DebuggerInterface* debuggerInterface,
									const entry_ref& targetPath);
	virtual						~WriteCoreFileJob();

	virtual	const JobKey&		Key() const;
	virtual	status_t			Do();

private:
			SimpleJobKey		fKey;
			Team*				fTeam;
			DebuggerInterface*	fDebuggerInterface;
			entry_ref			fTargetPath;
};


#endif	// JOBS_H
