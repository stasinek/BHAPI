/*
 * Copyright 2015 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Lotz, mmlr@mlotz.ch
 */

#include <libroot_private.h>


/*!	Captures a stack trace (the return addresses) of the current thread.
	\param returnAddresses The array the return address shall be written to.
	\param maxCount The maximum number of return addresses to be captured.
	\param skipFrames The number of stack frames that shall be skipped.
	\param stackBase The base address of the thread stack.
	\param stackEnd The first address past the thread stack.
	\return The number of return addresses written to the given array.
*/
int32_t __arch_get_stack_trace(addr_t* returnAddresses, int32_t maxCount,
	int32_t skipFrames, addr_t stackBase, addr_t stackEnd)
{
	int32_t count = 0;
	addr_t basePointer = (addr_t)get_stack_frame();

	while (basePointer != 0 && count < maxCount) {
		if (basePointer < stackBase || basePointer >= stackEnd)
			break;

		struct stack_frame {
			addr_t	previous;
			addr_t	return_address;
		};

		stack_frame* frame = (stack_frame*)basePointer;

		if (skipFrames <= 0)
			returnAddresses[count++] = frame->return_address;
		else
			skipFrames--;

		basePointer = frame->previous;
	}

	return count;
}
