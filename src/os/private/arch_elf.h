#ifndef ARCH_ELF_H
#define ARCH_ELF_H
    #ifdef __INTEL__
    #include <arch/x86/arch_elf.h>
    #endif
    #ifdef __ARM__
    #include <arch/arm/arch_elf.h>
    #endif
    #ifdef __POWERPC__
    #include <arch/powerpc/arch_elf.h>
    #endif
#endif
