/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Copyright 2011-2015, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef ARCHITECTURE_X86_H
#define ARCHITECTURE_X86_H


#include <Array.h>

#include <Architecture.h>
#include <Register.h>


enum {
	X86_CPU_FEATURE_FLAG_NONE = 0,
	X86_CPU_FEATURE_FLAG_MMX = 1,
	X86_CPU_FEATURE_FLAG_SSE = 2
};


class SourceLanguage;


class ArchitectureX86 : public Architecture {
public:
								ArchitectureX86(TeamMemory* teamMemory);
	virtual						~ArchitectureX86();

	virtual	status_t			Init();

	virtual int32_t				StackGrowthDirection() const;

	virtual	int32_t				CountRegisters() const;
	virtual	const Register*		Registers() const;
	virtual status_t			InitRegisterRules(CfaContext& context) const;

	virtual	status_t			GetDwarfRegisterMaps(RegisterMap** _toDwarf,
									RegisterMap** _fromDwarf) const;

	virtual	status_t			GetCpuFeatures(uint32_t& flags);

	virtual	status_t			CreateCpuState(CpuState*& _state);
	virtual	status_t			CreateCpuState(const void* cpuStateData,
									size_t size, CpuState*& _state);
	virtual	status_t			CreateStackFrame(Image* image,
									FunctionDebugInfo* function,
									CpuState* cpuState, bool isTopFrame,
									StackFrame*& _previousFrame,
									CpuState*& _previousCpuState);
	virtual	void				UpdateStackFrameCpuState(
									const StackFrame* frame,
									Image* previousImage,
									FunctionDebugInfo* previousFunction,
									CpuState* previousCpuState);

	virtual	status_t			ReadValueFromMemory(target_addr_t address,
									uint32_t valueType, BVariant& _value) const;
	virtual	status_t			ReadValueFromMemory(target_addr_t addressSpace,
									target_addr_t address, uint32_t valueType,
									BVariant& _value) const;

	virtual	status_t			DisassembleCode(FunctionDebugInfo* function,
									const void* buffer, size_t bufferSize,
									DisassembledCode*& _sourceCode);
	virtual	status_t			GetStatement(FunctionDebugInfo* function,
									target_addr_t address,
									Statement*& _statement);
	virtual	status_t			GetInstructionInfo(target_addr_t address,
									InstructionInfo& _info, CpuState* state);
	virtual	status_t			ResolvePICFunctionAddress(
									target_addr_t instructionAddress,
									CpuState* state,
									target_addr_t& _targetAddress);

	virtual	status_t			GetWatchpointDebugCapabilities(
									int32_t& _maxRegisterCount,
									int32_t& _maxBytesPerRegister,
									uint8& _watchpointCapabilityFlags);

	virtual	status_t			GetReturnAddressLocation(
									StackFrame* frame, target_size_t valueSize,
									ValueLocation*& _location);


private:
			struct ToDwarfRegisterMap;
			struct FromDwarfRegisterMap;

private:
			void				_AddRegister(int32_t index, const char* name,
									uint32_t bitSize, uint32_t valueType,
									register_type type, bool calleePreserved);
			void				_AddIntegerRegister(int32_t index,
									const char* name, uint32_t valueType,
									register_type type, bool calleePreserved);
			void				_AddFPRegister(int32_t index,
									const char* name);
			void				_AddSIMDRegister(int32_t index,
									const char* name, uint32_t byteSize);
			bool				_HasFunctionPrologue(
									FunctionDebugInfo* function) const;

private:
			uint32_t				fFeatureFlags;
			Array<Register>		fRegisters;
			SourceLanguage*		fAssemblyLanguage;
			ToDwarfRegisterMap*	fToDwarfRegisterMap;
			FromDwarfRegisterMap* fFromDwarfRegisterMap;
};


#endif	// ARCHITECTURE_X86_H
