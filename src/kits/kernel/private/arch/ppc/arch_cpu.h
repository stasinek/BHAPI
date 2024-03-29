/*
 * Copyright 2003-2004, Axel Dörfler, axeld@pinc-software.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef _KERNEL_ARCH_PPC_CPU_H
#define _KERNEL_ARCH_PPC_CPU_H


#include <arch/ppc/arch_thread_types.h>
#include <kernel.h>


#define CPU_MAX_CACHE_LEVEL	8
#define CACHE_LINE_SIZE		128
	// 128 Byte lines on PPC970


struct iframe {
	uint32_t vector;
	uint32_t srr0;
	uint32_t srr1;
	uint32_t dar;
	uint32_t dsisr;
	uint32_t lr;
	uint32_t cr;
	uint32_t xer;
	uint32_t ctr;
	uint32_t fpscr;
	uint32_t r31;
	uint32_t r30;
	uint32_t r29;
	uint32_t r28;
	uint32_t r27;
	uint32_t r26;
	uint32_t r25;
	uint32_t r24;
	uint32_t r23;
	uint32_t r22;
	uint32_t r21;
	uint32_t r20;
	uint32_t r19;
	uint32_t r18;
	uint32_t r17;
	uint32_t r16;
	uint32_t r15;
	uint32_t r14;
	uint32_t r13;
	uint32_t r12;
	uint32_t r11;
	uint32_t r10;
	uint32_t r9;
	uint32_t r8;
	uint32_t r7;
	uint32_t r6;
	uint32_t r5;
	uint32_t r4;
	uint32_t r3;
	uint32_t r2;
	uint32_t r1;
	uint32_t r0;
	double f31;
	double f30;
	double f29;
	double f28;
	double f27;
	double f26;
	double f25;
	double f24;
	double f23;
	double f22;
	double f21;
	double f20;
	double f19;
	double f18;
	double f17;
	double f16;
	double f15;
	double f14;
	double f13;
	double f12;
	double f11;
	double f10;
	double f9;
	double f8;
	double f7;
	double f6;
	double f5;
	double f4;
	double f3;
	double f2;
	double f1;
	double f0;
};

enum machine_state {
	MSR_EXCEPTIONS_ENABLED			= 1L << 15,		// EE
	MSR_PRIVILEGE_LEVEL				= 1L << 14,		// PR
	MSR_FP_AVAILABLE				= 1L << 13,		// FP
	MSR_MACHINE_CHECK_ENABLED		= 1L << 12,		// ME
	MSR_EXCEPTION_PREFIX			= 1L << 6,		// IP
	MSR_INST_ADDRESS_TRANSLATION	= 1L << 5,		// IR
	MSR_DATA_ADDRESS_TRANSLATION	= 1L << 4,		// DR
};

struct block_address_translation;

typedef struct arch_cpu_info {
	int null;
} arch_cpu_info;


#define eieio()	asm volatile("eieio")
#define isync() asm volatile("isync")
#define tlbsync() asm volatile("tlbsync")
#define ppc_sync() asm volatile("sync")
#define tlbia() asm volatile("tlbia")
#define tlbie(addr) asm volatile("tlbie %0" :: "r" (addr))

// adjust thread priority on PowerPC (Shared resource hints)
#define SRH_very_low() asm volatile("or 31,31,31")
#define SRH_low() asm volatile("or 1,1,1")
#define SRH_medium_low() asm volatile("or 6,6,6")
#define SRH_medium() asm volatile("or 2,2,2")
#define SRH_medium_high() asm volatile("or 5,5,5")
#define SRH_high() asm volatile("or 3,3,3")


#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t get_sdr1(void);
extern void set_sdr1(uint32_t value);
extern uint32_t get_sr(void *virtualAddress);
extern void set_sr(void *virtualAddress, uint32_t value);
extern uint32_t get_msr(void);
extern uint32_t set_msr(uint32_t value);
extern uint32_t get_pvr(void);

extern void set_ibat0(struct block_address_translation *bat);
extern void set_ibat1(struct block_address_translation *bat);
extern void set_ibat2(struct block_address_translation *bat);
extern void set_ibat3(struct block_address_translation *bat);
extern void set_dbat0(struct block_address_translation *bat);
extern void set_dbat1(struct block_address_translation *bat);
extern void set_dbat2(struct block_address_translation *bat);
extern void set_dbat3(struct block_address_translation *bat);

extern void get_ibat0(struct block_address_translation *bat);
extern void get_ibat1(struct block_address_translation *bat);
extern void get_ibat2(struct block_address_translation *bat);
extern void get_ibat3(struct block_address_translation *bat);
extern void get_dbat0(struct block_address_translation *bat);
extern void get_dbat1(struct block_address_translation *bat);
extern void get_dbat2(struct block_address_translation *bat);
extern void get_dbat3(struct block_address_translation *bat);

extern void reset_ibats(void);
extern void reset_dbats(void);

//extern void sethid0(unsigned int val);
//extern unsigned int getl2cr(void);
//extern void setl2cr(unsigned int val);
extern long long get_time_base(void);

void __ppc_setup_system_time(vint32_t *cvFactor);
	// defined in libroot: os/arch/system_time.c
int64 __ppc_get_time_base(void);
	// defined in libroot: os/arch/system_time_asm.S

extern void ppc_context_switch(void **_oldStackPointer, void *newStackPointer);

extern bool ppc_set_fault_handler(addr_t *handlerLocation, addr_t handler)
	__attribute__((noinline));


static inline void arch_cpu_pause(void)
{
	// TODO: PowerPC review logic of setting very low for pause
	SRH_very_low();
}


static inline void arch_cpu_idle(void)
{
	// TODO: PowerPC CPU idle call
}


#ifdef __cplusplus
}
#endif

// PowerPC processor version (the upper 16 bits of the PVR).
enum ppc_processor_version {
	MPC601		= 0x0001,
	MPC603		= 0x0003,
	MPC604		= 0x0004,
	MPC602		= 0x0005,
	MPC603e		= 0x0006,
	MPC603ev	= 0x0007,
	MPC750		= 0x0008,
	MPC604ev	= 0x0009,
	MPC7400		= 0x000c,
	MPC620		= 0x0014,
	IBM403		= 0x0020,
	IBM401A1	= 0x0021,
	IBM401B2	= 0x0022,
	IBM401C2	= 0x0023,
	IBM401D2	= 0x0024,
	IBM401E2	= 0x0025,
	IBM401F2	= 0x0026,
	IBM401G2	= 0x0027,
	IBMPOWER3	= 0x0041,
	MPC860		= 0x0050,
	MPC8240		= 0x0081,
	AMCC460EX	= 0x1302,
	IBM405GP	= 0x4011,
	IBM405L		= 0x4161,
	AMCC440EP	= 0x4222,
	IBM750FX	= 0x7000,
	MPC7450		= 0x8000,
	MPC7455		= 0x8001,
	MPC7457		= 0x8002,
	MPC7447A	= 0x8003,
	MPC7448		= 0x8004,
	MPC7410		= 0x800c,
	MPC8245		= 0x8081,
};


/*
	Use of (some) special purpose registers.

	SPRG0: per CPU physical address pointer to an ppc_cpu_exception_context
	       structure
	SPRG1: scratch
	SPRG2: current Thread*
	SPRG3: TLS base pointer (only for userland threads)
*/

#endif	/* _KERNEL_ARCH_PPC_CPU_H */
