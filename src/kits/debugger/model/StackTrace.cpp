/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */

#include <StackTrace.h>


StackTrace::StackTrace()
{
}


StackTrace::~StackTrace()
{
	for (int32_t i = 0; StackFrame* frame = FrameAt(i); i++)
		frame->ReleaseReference();
}


bool StackTrace::AddFrame(StackFrame* frame)
{
	if (fStackFrames.AddItem(frame))
		return true;

	frame->ReleaseReference();
	return false;
}


int32_t StackTrace::CountFrames() const
{
	return fStackFrames.CountItems();
}


StackFrame*
StackTrace::FrameAt(int32_t index) const
{
	return fStackFrames.ItemAt(index);
}
