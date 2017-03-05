/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Copyright 2010-2016, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef LOCAL_DEBUGGER_INTERFACE_H
#define LOCAL_DEBUGGER_INTERFACE_H

#include "DebuggerInterface.h"


class LocalDebuggerInterface : public DebuggerInterface {
public:
								LocalDebuggerInterface(team_id team);
	virtual						~LocalDebuggerInterface();

	virtual	status_t			Init();
	virtual	void				Close(bool killTeam);

	virtual	bool				Connected() const;

	virtual	team_id				TeamID() const;

	virtual	Architecture*		GetArchitecture() const;

	virtual	status_t			GetNextDebugEvent(DebugEvent*& _event);

	virtual	status_t			SetTeamDebuggingFlags(uint32 flags);

	virtual	status_t			ContinueThread(thread_id thread);
	virtual	status_t			StopThread(thread_id thread);
	virtual	status_t			SingleStepThread(thread_id thread);

	virtual	status_t			InstallBreakpoint(target_addr_t address);
	virtual	status_t			UninstallBreakpoint(target_addr_t address);

	virtual status_t			InstallWatchpoint(target_addr_t address,
									uint32 type, int32 length);
	virtual status_t			UninstallWatchpoint(target_addr_t address);

	virtual	status_t			GetSystemInfo(SystemInfo& info);
	virtual	status_t			GetTeamInfo(TeamInfo& info);
	virtual	status_t			GetThreadInfos(BObjectList<ThreadInfo>& infos);
	virtual	status_t			GetImageInfos(BObjectList<ImageInfo>& infos);
	virtual status_t			GetAreaInfos(BObjectList<AreaInfo>& infos);
	virtual status_t			GetSemaphoreInfos(
									BObjectList<SemaphoreInfo>& infos);
	virtual	status_t			GetSymbolInfos(team_id team, image_id image,
									BObjectList<SymbolInfo>& infos);
	virtual	status_t			GetSymbolInfo(team_id team, image_id image,
									const char* name, int32 symbolType,
									SymbolInfo& info);

	virtual	status_t			GetThreadInfo(thread_id thread,
									ThreadInfo& info);
	virtual	status_t			GetCpuState(thread_id thread,
									CpuState*& _state);
										// returns a reference to the caller
	virtual	status_t			SetCpuState(thread_id thread,
									const CpuState* state);

	virtual	status_t			GetCpuFeatures(uint32& flags);

	virtual	status_t			WriteCoreFile(const char* path);

	// TeamMemory
	virtual	status_t			GetMemoryProperties(target_addr_t address,
									uint32& protection, uint32& locking);

	virtual	ssize_t				ReadMemory(target_addr_t address, void* buffer,
									size_t size);
	virtual	ssize_t				WriteMemory(target_addr_t address,
									void* buffer, size_t size);

private:
	struct DebugContext;
	struct DebugContextPool;
	struct DebugContextGetter;

private:
			status_t			_CreateDebugEvent(int32 messageCode,
									const debug_debugger_message_data& message,
									bool& _ignore, DebugEvent*& _event);

			status_t			_GetNextSystemWatchEvent(DebugEvent*& _event,
									BPrivate::KMessage& message);

			status_t			_GetDebugCpuState(thread_id thread,
									debug_cpu_state& _state);

private:
			team_id				fTeamID;
			port_id				fDebuggerPort;
			port_id				fNubPort;
			DebugContextPool*	fDebugContextPool;
			Architecture*		fArchitecture;
};

#endif	// DEBUGGER_INTERFACE_H
