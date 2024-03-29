/*
 * Copyright 2003-2009, Axel D�rfler, axeld@pinc-software.de.
 * Distributed under the terms of the MIT License.
 */

#include <asm_defs.h>

.text

/* int atomic_add(int *value, int increment)
 * (r3)           r3          r4
 */
FUNCTION(atomic_add):
lost1:	lwarx	%r5, 0, %r3		// reserve memory address located in "r3"
		add		%r0, %r5, %r4	// (contents are stored in "r5")
		stwcx.	%r0, 0, %r3
		bne-	lost1			// try again if reservation was lost
		mr		%r3, %r5		// return old value (was in "r5")
		blr
FUNCTION_END(atomic_add)

/* int atomic_and(int *value, int andValue)
 * (r3)           r3          r4
 */
FUNCTION(atomic_and):
lost2:	lwarx	%r5, 0, %r3
		and		%r0, %r5, %r4
		stwcx.	%r0, 0, %r3
		bne-	lost2
		mr		%r3, %r5
		blr
FUNCTION_END(atomic_and)

/* int atomic_or(int *value, int orValue)
 * (r3)          r3          r4
 */
FUNCTION(atomic_or):
lost3:	lwarx	%r5, 0, %r3
		or		%r0, %r5, %r4
		stwcx.	%r0, 0, %r3
		bne-	lost3
		mr		%r3, %r5
		blr
FUNCTION_END(atomic_or)

/* int atomic_set(int *value, int setTo)
 * (r3)           r3          r4
 */
FUNCTION(atomic_set):
lost4:	lwarx	%r5, 0, %r3
		stwcx.	%r4, 0, %r3
		bne-	lost4
		mr		%r3, %r5
		blr
FUNCTION_END(atomic_set)

/* int atomic_test_and_set(int *value, int setTo, int testValue)
 * (r3)                    r3          r4         r5
 */
FUNCTION(atomic_test_and_set):
lost5:	lwarx	%r6, 0, %r3
		cmpw	%r6, %r5
		bne		out5
		stwcx.	%r4, 0, %r3
		bne-	lost5
out5:	mr		%r3, %r6
		blr
FUNCTION_END(atomic_test_and_set)

// TODO: PowerPC atomic_get_and_set
FUNCTION(atomic_get_and_set):
		sync
		blr
FUNCTION_END(atomic_get_and_set)

/* int atomic_get(int *value)
 * (r3)           r3
 */
FUNCTION(atomic_get):
lost6:	lwarx	%r5, 0, %r3
		stwcx.	%r5, 0, %r3
		bne-	lost6
		mr		%r3, %r5
		blr
FUNCTION_END(atomic_get)
