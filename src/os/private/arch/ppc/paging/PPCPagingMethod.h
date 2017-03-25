/*
 * Copyright 2010, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef KERNEL_ARCH_PPC_PAGING_PPC_PAGING_METHOD_H
#define KERNEL_ARCH_PPC_PAGING_PPC_PAGING_METHOD_H


#include <SupportDefs.h>

#include <vm/vm_types.h>


struct kernel_args;
struct VMPhysicalPageMapper;
struct VMTranslationMap;


class PPCPagingMethod {
public:
	virtual						~PPCPagingMethod();

	virtual	status_t			Init(kernel_args* args,
									VMPhysicalPageMapper** _physicalPageMapper)
										= 0;
	virtual	status_t			InitPostArea(kernel_args* args) = 0;

	virtual	status_t			CreateTranslationMap(bool kernel,
									VMTranslationMap** _map) = 0;

	virtual	status_t			MapEarly(kernel_args* args,
									addr_t virtualAddress,
									phys_addr_t physicalAddress,
									uint8 attributes,
									page_num_t (*get_free_page)(kernel_args*))
										= 0;

	virtual	bool				IsKernelPageAccessible(addr_t virtualAddress,
									uint32 protection) = 0;
};


extern PPCPagingMethod* gPPCPagingMethod;


#endif	// KERNEL_ARCH_PPC_PAGING_PPC_PAGING_METHOD_H
