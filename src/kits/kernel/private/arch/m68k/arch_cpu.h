/*
** Copyright 2003-2004, Axel Dörfler, axeld@pinc-software.de. All rights reserved.
** Distributed under the terms of the Haiku License.
*/
#ifndef _KERNEL_ARCH_M68K_CPU_H
#define _KERNEL_ARCH_M68K_CPU_H

#ifndef _ASSEMBLER

#include <arch/m68k/arch_thread_types.h>
#include <kernel.h>

#endif	// !_ASSEMBLER


#define SR_IP_MASK 0x0700
#define SR_S 0x2000
#define M68K_SR_S 13
#define M68K_SR_T_MASK 0xC000
#define M68K_SR_T0 14
#define M68K_SR_T1 15

#ifndef _ASSEMBLER

/* 68k has many different possible stack frames, differentiated by a 4 bit number,
 * but they also depend on the cpu type.
 * cf. mint/sys/arch/check_exc.h
 */

/* definitions for special status word */

// 020 as well
struct mc68030_ssw {
	uint16 fc:1;
	uint16 fb:1;
	uint16 rc:1;
	uint16 rb:1;
	uint16 :3;
	uint16 df:1;
	uint16 rm:1;
	uint16 rw:1;
	uint16 size:2;
	uint16 :1;
	uint16 as:3;
} _PACKED;

struct mc68040_ssw {
	uint16 cp:1;
	uint16 cu:1;
	uint16 ct:1;
	uint16 cm:1;
	uint16 ma:1;
	uint16 atc:1;
	uint16 lk:1;
	uint16 rw:1;
	uint16 :1;
	uint16 size:2;
	uint16 tt:2;
	uint16 tm:3;
} _PACKED;

struct mc68060_fslw {
	uint32_t :4;
	uint32_t ma:1;
	uint32_t :1;
	uint32_t lk:1;
	uint32_t rw:2; //XXX ??
	uint32_t size:2;
	uint32_t tt:2;
	uint32_t tm:2;
	uint32_t io:1;
	uint32_t pbe:1;
	uint32_t sbe:1;
	uint32_t pta:1;
	uint32_t ptb:1;
	uint32_t il:1;
	uint32_t pf:1;
	uint32_t sb:1;
	uint32_t wp:1;
	uint32_t twe:1;
	uint32_t re:1;
	uint32_t we:1;
	uint32_t ttr:1;
	uint32_t bpe:1;
	uint32_t :1;
	uint32_t see:1;
} _PACKED;

/* raw exception frames */

struct mc680x0_type_0_frame {
	uint16 sr;
	addr_t pc;
	uint16 type:4;
	uint16 vector:12;
};

struct mc680x0_type_1_frame {
	uint16 sr;
	addr_t pc;
	uint16 type:4;
	uint16 vector:12;
};

struct mc680x0_type_2_frame {
	uint16 sr;
	addr_t pc;
	uint16 type:4;
	uint16 vector:12;
	addr_t instruction_address;
};

struct mc680x0_type_3_frame {
	uint16 sr;
	addr_t pc;
	uint16 type:4;
	uint16 vector:12;
	addr_t effective_address;
};

struct mc68040_type_7_frame {
	uint16 sr;
	addr_t pc;
	uint16 type:4;
	uint16 vector:12;
	addr_t effective_address;
	struct mc68040_ssw ssw;
	// write-back status
	uint16 wb3s;
	uint16 wb2s;
	uint16 wb1s;
	addr_t fault_address;
	addr_t wb3a;
	uint32_t wb3d;
	addr_t wb2a;
	uint32_t wb2d;
	addr_t wb1a;
	uint32_t wb1d; // also pd0
	uint32_t pd1;
	uint32_t pd2;
	uint32_t pd3;
};

struct mc680x0_type_9_frame {
	uint16 sr;
	addr_t pc;
	uint16 type:4;
	uint16 vector:12;
	addr_t instruction_address;
	uint16 intregs[4];
};

struct mc68030_type_a_frame {
	uint16 sr;
	addr_t pc;
	uint16 type:4;
	uint16 vector:12;
	uint16 intreg1;
	struct mc68030_ssw ssw;
	uint16 instpipe_c;
	uint16 instpipe_b;
	addr_t fault_address;
	uint16 intregs2[2];
	uint32_t dataout;
	uint16 intregs3[2];
};

struct mc68030_type_b_frame {
	uint16 sr;
	addr_t pc;
	uint16 type:4;
	uint16 vector:12;
	uint16 intreg1;
	struct mc68030_ssw ssw;
	uint16 instpipe_c;
	uint16 instpipe_b;
	addr_t fault_address;
	uint16 intregs2[2];
	uint32_t dataout;
	uint16 intregs3[4];
	uint32_t stbaddr;
	uint16 intregs4[2];
	uint32_t datain;
	uint16 intregs5[3];
	uint16 intinfo;
	uint16 intregs6[18];
};

//XXX: add 060 frames

struct mc680x0_frame {
	union {
		struct {
			uint16 sr;
			addr_t pc;
			uint16 type:4;
			uint16 vector:12;
		};
		struct mc680x0_type_0_frame type_0;
		struct mc680x0_type_1_frame type_1;
		struct mc680x0_type_3_frame type_3;
		struct mc680x0_type_2_frame type_2;
		struct mc68040_type_7_frame type_7;
		struct mc680x0_type_9_frame type_9;
		struct mc68030_type_a_frame type_a;
		struct mc68030_type_b_frame type_b;
		// XXX: add 060 frames
	};
};

struct mc680x0_null_fpu_state {
	uint8 version; // 0
	uint8 type; // undefined
	uint16 dummy;
};

struct mc680x0_type_00_fpu_state {
	uint8 version;
	uint8 type; // 0
	uint16 dummy;
};

struct mc680x0_type_28_fpu_state {
	uint8 version;
	uint8 type; // 0x28
	uint16 dummy;
	// XXX: replace dummies
	uint32_t dummies[0x28/4];
};

struct mc680x0_type_60_fpu_state {
	uint8 version;
	uint8 type; // 0x60
	uint16 dummy;
	// XXX: replace dummies
	uint32_t dummies[0x60/4];
};

//XXX: those are 040, add others
// biggest known:
struct mc68882_type_d4_fpu_state {
	uint8 version;
	uint8 type; // 0xd4
	uint16 dummy;
	// XXX: replace dummies
	uint32_t dummies[0xd4/4];
};

struct mc680x0_fpu_state {
	union {
		struct {
			uint8 version;
			uint8 type; // 0x60
			uint16 dummy;
		};
		struct mc680x0_null_fpu_state null;
		struct mc680x0_type_00_fpu_state type_00;
		struct mc680x0_type_28_fpu_state type_28;
		struct mc680x0_type_60_fpu_state type_60;
		struct mc68882_type_d4_fpu_state type_d4;
	};
};

// 96-bit format
struct mc680x0_fp_data_reg {
	uint8 data[12];
};

struct mc680x0_fp_control_regs {
	uint32_t fpcr;
	uint32_t fpsr;
	uint32_t fpiar;
};



#warning M68K: check for missing regs/movem
struct iframe {
	/* fpu data registers */
#warning M68K: sizeof(fp*)
	struct mc680x0_fp_data_reg fp[8];
	/* fpu control registers */
	struct mc680x0_fp_control_regs fpc;
	/* fpu state */
	struct mc680x0_fpu_state fpu;

	/* data and address registers */
	uint32_t d[8];
	uint32_t a[7];
	/* cpu exception frame, including sr, pc, format and vector */
	struct mc680x0_frame cpu;

/*	uint32_t vector;
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
	double f0;*/
} _PACKED;

#if 0 /* ppc */
enum machine_state {
	MSR_EXCEPTIONS_ENABLED			= 1L << 15,		// EE
	MSR_PRIVILEGE_LEVEL				= 1L << 14,		// PR
	MSR_FP_AVAILABLE				= 1L << 13,		// FP
	MSR_MACHINE_CHECK_ENABLED		= 1L << 12,		// ME
	MSR_EXCEPTION_PREFIX			= 1L << 6,		// IP
	MSR_INST_ADDRESS_TRANSLATION	= 1L << 5,		// IR
	MSR_DATA_ADDRESS_TRANSLATION	= 1L << 4,		// DR
};

//struct block_address_translation;
#endif

typedef struct arch_cpu_info {
	int null;
} arch_cpu_info;


#ifdef __cplusplus
extern "C" {
#endif

#if 0
//PPC stuff
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
#endif

//extern void sethid0(unsigned int val);
//extern unsigned int getl2cr(void);
//extern void setl2cr(unsigned int val);
extern long long get_time_base(void);

void __m68k_setup_system_time(vint32_t *cvFactor);
	// defined in libroot: os/arch/system_time.c
int64 __m68k_get_time_base(void);
	// defined in libroot: os/arch/system_time_asm.S

extern void m68k_context_switch(void **_oldStackPointer, void *newStackPointer);

extern bool m68k_set_fault_handler(addr_t *handlerLocation, addr_t handler)
	__attribute__((noinline));

extern bool m68k_is_hw_register_readable(addr_t address);
extern bool m68k_is_hw_register_writable(addr_t address, uint16 value);
	// defined in kernel: arch/m68k/cpu_asm.S

#ifdef __cplusplus
}
#endif

struct m68k_cpu_ops {
	void (*flush_insn_pipeline)(void);
	void (*flush_atc_all)(void);
	void (*flush_atc_user)(void);
	void (*flush_atc_addr)(addr_t addr);
	void (*flush_dcache)(addr_t address, size_t len);
	void (*flush_icache)(addr_t address, size_t len);
	void (*idle)(void);
};

extern struct m68k_cpu_ops cpu_ops;

//#define

#if 0
#define eieio()	asm volatile("eieio")
#define isync() asm volatile("isync")
#define tlbsync() asm volatile("tlbsync")
#define ppc_sync() asm volatile("sync")
#define tlbia() asm volatile("tlbia")
#define tlbie(addr) asm volatile("tlbie %0" :: "r" (addr))
#endif

#if 0

// XXX: not used: we just use decimal chip number, like 68030

// m68k processor version.
enum m68k_processor_version {
	/* those two we don't support */
	CPU_68000		= 0x0000,
	CPU_68010		= 0x0001,
	/* maybe with a pmmu and fpu */
	CPU_68020		= 0x0002,
	/* should work */
	CPU_68030		= 0x0003,
	CPU_68040		= 0x0004,
	CPU_68060		= 0x0006,
	/* mask */
	CPU_MASK		= 0x000F
};

enum m68k_fpu_version {
	/* we don't support */
	FPU_NONE		= 0x0000,
	FPU_68881		= 0x0010,
	FPU_68882		= 0x0020,
	FPU_030			= 0x0030,
	FPU_040			= 0x0040,
	FPU_060			= 0x0060,
	FPU_MASK		= 0x00F0
};

enum m68k_mmu_version {
	MMU_NONE		= 0x0000,
	MMU_68551		= 0x0100,
	MMU_68030		= 0x0300,
	MMU_68040		= 0x0400,
	MMU_68060		= 0x0600,
	MMU_MASK		= 0x0F00
};
#endif

extern int arch_cpu_type;
extern int arch_fpu_type;
extern int arch_mmu_type;
extern int arch_platform;
extern int arch_machine;

/*
	Use of (some) special purpose registers.
	XXX: those regs aren't implemented/accessed the same way on different cpus...

	SRP[63-32]: current Thread*
	SRP[31-0] :
	CAAR      : can we use it ??
	MSP       :

	PPC:
	SPRG0: per CPU physical address pointer to an ppc_cpu_exception_context
	       structure
	SPRG1: scratch
	SPRG2: current Thread*
	SPRG3: TLS base pointer (only for userland threads)
*/

#endif	// !_ASSEMBLER

#endif	/* _KERNEL_ARCH_PPC_CPU_H */
