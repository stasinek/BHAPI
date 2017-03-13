#ifndef _I386_BITOPS_H
#define _I386_BITOPS_H

/*
 * Copyright 1992, Linus Torvalds.
 */

#include <linux/config.h> 
#include "linux/kernel.h"

#include <ntifs.h>


#define test_bit(nr,addr) \
(((1UL << ((nr)&31)) & (((const volatile unsigned int *) (addr))[(nr) >> 5])) != 0)


/**
 * find_first_zero_bit - find the first zero bit in a memory region
 * @addr: The address to start the search at
 * @size: The maximum size to search
 *
 * Returns the bit-number of the first zero bit, not the number of the byte
 * containing a bit.
 */ 
static __inline int find_first_zero_bit(void * addr, unsigned size)
{
	RTL_BITMAP bitmap;
	ULONG index;

	ASSERT(!((size>>3)%sizeof(ULONG)));	// daca este apelat si altfel o sa tb sa fac de mana
	RtlInitializeBitMap(&bitmap, (PULONG)addr, size);
	index = RtlFindClearBits(&bitmap, 1, 0);
	if (index == 0xFFFFFFFF)
		index = size;
	return index;
}

/**
 * find_next_zero_bit - find the first zero bit in a memory region
 * @addr: The address to base the search on
 * @offset: The bitnumber to start searching at
 * @size: The maximum size to search
 */
static __inline__ int find_next_zero_bit (void * addr, int size, int offset)
{
	unsigned long * p = ((unsigned long *) addr) + (offset >> 5);
	int set = 0, bit = offset & 31, res;
	
	if (bit) {
		/*
		 * Look for zero in first byte
		 */
		printk("find_next_zero_bit unimplemented!\n");
		if (set < (32 - bit))
			return set + offset;
		set = 32 - bit;
		p++;
	}
	/*
	 * No zero yet, search remaining full bytes for a zero
	 */
	res = find_first_zero_bit (p, size - 32 * (p - (unsigned long *) addr));
	return (offset + set + res);
} 

/**
 * test_and_set_bit - Set a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is atomic and cannot be reordered.  
 * It also implies a memory barrier.
 */ 
static __inline int test_and_set_bit(int nr, volatile void * addr)
{
	RTL_BITMAP bitmap;
	ULONG index;
	ULONG size = (nr + 31) & 32;
	int ret;

	// nu stiu daca e bine
	RtlInitializeBitMap(&bitmap, (PULONG)addr, size);
	ret = RtlCheckBit(&bitmap, nr);
	RtlSetBits(&bitmap, nr, 1);
	return ret;
}


/**
 * __test_and_set_bit - Set a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is non-atomic and can be reordered.  
 * If two examples of this operation race, one can appear to succeed
 * but actually fail.  You must protect multiple accesses with a lock.
 */ 
static __inline int __test_and_set_bit(int nr, volatile void * addr)
{
	RTL_BITMAP bitmap;
	ULONG index;
	ULONG size = (nr + 31) & 32;
	int ret;

	// nu stiu daca e bine
	RtlInitializeBitMap(&bitmap, (PULONG)addr, size);
	ret = RtlCheckBit(&bitmap, nr);
	RtlSetBits(&bitmap, nr, 1);
	return ret;
}

/**
 * test_and_clear_bit - Clear a bit and return its old value
 * @nr: Bit to clear
 * @addr: Address to count from
 *
 * This operation is atomic and cannot be reordered.  
 * It also implies a memory barrier.
 */ 
static __inline int test_and_clear_bit(int nr, volatile void * addr)
{
	RTL_BITMAP bitmap;
	ULONG index;
	ULONG size = (nr + 31) & 32;
	int ret;

	// nu stiu daca e bine
	RtlInitializeBitMap(&bitmap, (PULONG)addr, size);
	ret = RtlCheckBit(&bitmap, nr);
	RtlClearBits(&bitmap, nr, 1);
	return ret;
}

/**
 * __test_and_clear_bit - Clear a bit and return its old value
 * @nr: Bit to clear
 * @addr: Address to count from
 *
 * This operation is non-atomic and can be reordered.  
 * If two examples of this operation race, one can appear to succeed
 * but actually fail.  You must protect multiple accesses with a lock.
 */ 
static __inline int __test_and_clear_bit(int nr, volatile void * addr)
{
	RTL_BITMAP bitmap;
	ULONG index;
	ULONG size = (nr + 31) & 32;
	int ret;

	// nu stiu daca e bine
	RtlInitializeBitMap(&bitmap, (PULONG)addr, size);
	ret = RtlCheckBit(&bitmap, nr);
	RtlClearBits(&bitmap, nr, 1);
	return ret;
}

/**
 * set_bit - Atomically set a bit in memory
 * @nr: the bit to set
 * @addr: the address to start counting from
 *
 * This function is atomic and may not be reordered.  See __set_bit()
 * if you do not require the atomic guarantees.
 * Note that @nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static __inline void set_bit(int nr, volatile void * addr)
{
	RTL_BITMAP bitmap;
	ULONG index;
	ULONG size = (nr + 31) & 32;

	RtlInitializeBitMap(&bitmap, (PULONG)addr, size);
	RtlSetBits(&bitmap, nr, 1);
}

/**
 * __set_bit - Set a bit in memory
 * @nr: the bit to set
 * @addr: the address to start counting from
 *
 * Unlike set_bit(), this function is non-atomic and may be reordered.
 * If it's called on the same region of memory simultaneously, the effect
 * may be that only one operation succeeds.
 */
static __inline void __set_bit(int nr, volatile void * addr)
{
	RTL_BITMAP bitmap;
	ULONG index;
	ULONG size = (nr + 31) & 32;

	RtlInitializeBitMap(&bitmap, (PULONG)addr, size);
	RtlSetBits(&bitmap, nr, 1);
}

/**
 * clear_bit - Clears a bit in memory
 * @nr: Bit to clear
 * @addr: Address to start counting from
 *
 * clear_bit() is atomic and may not be reordered.  However, it does
 * not contain a memory barrier, so if it is used for locking purposes,
 * you should call smp_mb__before_clear_bit() and/or smp_mb__after_clear_bit()
 * in order to ensure changes are visible on other processors.
 */ 
static __inline void clear_bit(int nr, volatile void * addr)
{
	RTL_BITMAP bitmap;
	ULONG index;
	ULONG size = (nr + 31) & 32;

	RtlInitializeBitMap(&bitmap, (PULONG)addr, size);
	RtlClearBits(&bitmap, nr, 1);
}

#define ext2_set_bit                 __test_and_set_bit
#define ext2_clear_bit               __test_and_clear_bit
#define ext2_test_bit                test_bit
#define ext2_find_first_zero_bit     find_first_zero_bit
#define ext2_find_next_zero_bit      find_next_zero_bit

/* Bitmap functions for the minix filesystem.  */
#define minix_test_and_set_bit(nr,addr) __test_and_set_bit(nr,addr)
#define minix_set_bit(nr,addr) __set_bit(nr,addr)
#define minix_test_and_clear_bit(nr,addr) __test_and_clear_bit(nr,addr)
#define minix_test_bit(nr,addr) test_bit(nr,addr)
#define minix_find_first_zero_bit(addr,size) find_first_zero_bit(addr,size) 


/**
 * ffz - find first zero in word.
 * @word: The word to search
 *
 * Undefined if no zero exists, so code should check against ~0UL first.
 */
static __inline unsigned long ffz(unsigned long word)
{
	unsigned long tmp = word, poz = 1;
	while (word % 2) {
		word >>= 1;
		poz++;
	}
//	printk("ffz(%u)=%u\n", tmp, poz);
	return poz;
}
/**
 * ffs - find first bit set
 * @x: the word to search
 *
 * This is defined the same way as
 * the libc and compiler builtin ffs routines, therefore
 * differs in spirit from the above ffz (man ffs).
 */
static __inline int ffs(int x)
{
	unsigned int tmp = (unsigned int)x, poz = 1;
	while (x % 2 == 0) {
		x >>= 1;
		poz++;
	}
//	printk("ffs(%u)=%u\n", tmp, poz);
	return poz;
}



#endif
