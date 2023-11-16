/*
 * Copyright 2001-2010, Haiku, Inc.
 * Distributed under the terms of the MIT License.
 */

#ifndef __TEXT_VIEW_SUPPORT_BUFFER__H__
#define __TEXT_VIEW_SUPPORT_BUFFER__H__

#include <cstdlib>
#include <cstring>

#include <kits/kernel/OS.h>
#include <Haiku.h>


// _BTextViewSupportBuffer_ class ----------------------------------------------
template <class T>
class _BTextViewSupportBuffer_ {

public:
				_BTextViewSupportBuffer_(int32_t inExtraCount = 0, int32_t inCount = 0);
virtual			~_BTextViewSupportBuffer_();

		void	InsertItemsAt(int32_t inNumItems, int32_t inAtIndex, const T* inItem);
		void	RemoveItemsAt(int32_t inNumItems, int32_t inAtIndex);

		int32_t	ItemCount() const;

protected:
		int32_t	fExtraCount;	
		int32_t	fItemCount;				
		int32_t	fBufferCount;
		T*		fBuffer;
};


template <class T>
_BTextViewSupportBuffer_<T>::_BTextViewSupportBuffer_(int32_t inExtraCount,
													  int32_t inCount)
	:	fExtraCount(inExtraCount),
		fItemCount(inCount),
		fBufferCount(fExtraCount + fItemCount),
		fBuffer(NULL)
{
	fBuffer = (T*)calloc(fExtraCount + fItemCount, sizeof(T));
}


template <class T>
_BTextViewSupportBuffer_<T>::~_BTextViewSupportBuffer_()
{
	free(fBuffer);
}


template <class T>
void _BTextViewSupportBuffer_<T>::InsertItemsAt(int32_t inNumItems,
												int32_t inAtIndex,
												const T* inItem)
{
	if (inNumItems < 1)
		return;
	
	inAtIndex = (inAtIndex > fItemCount) ? fItemCount : inAtIndex;
	inAtIndex = (inAtIndex < 0) ? 0 : inAtIndex;

	int32_t delta = inNumItems * sizeof(T);
	int32_t logSize = fItemCount * sizeof(T);
	if ((logSize + delta) >= fBufferCount) {
		fBufferCount = logSize + delta + (fExtraCount * sizeof(T));
		fBuffer = (T*)realloc(fBuffer, fBufferCount);
		if (fBuffer == NULL)
			debugger("InsertItemsAt(): reallocation failed");
	}
	
	T* loc = fBuffer + inAtIndex;
	memmove(loc + inNumItems, loc, (fItemCount - inAtIndex) * sizeof(T));
	memcpy(loc, inItem, delta);
	
	fItemCount += inNumItems;
}


template <class T>
void _BTextViewSupportBuffer_<T>::RemoveItemsAt(int32_t inNumItems,
												int32_t inAtIndex)
{
	if (inNumItems < 1)
		return;
	
	inAtIndex = (inAtIndex > fItemCount - 1) ? (fItemCount - 1) : inAtIndex;
	inAtIndex = (inAtIndex < 0) ? 0 : inAtIndex;
	
	T* loc = fBuffer + inAtIndex;
	memmove(loc, loc + inNumItems, 
			(fItemCount - (inNumItems + inAtIndex)) * sizeof(T));
	
	int32_t delta = inNumItems * sizeof(T);
	int32_t logSize = fItemCount * sizeof(T);
	uint32_t extraSize = fBufferCount - (logSize - delta);
	if (extraSize > (fExtraCount * sizeof(T))) {
		fBufferCount = (logSize - delta) + (fExtraCount * sizeof(T));
		fBuffer = (T*)realloc(fBuffer, fBufferCount);
		if (fBuffer == NULL)
			debugger("RemoveItemsAt(): reallocation failed");
	}
	
	fItemCount -= inNumItems;
}


template<class T>
inline int32_t _BTextViewSupportBuffer_<T>::ItemCount() const
{
	return fItemCount;
}


#endif // __TEXT_VIEW_SUPPORT_BUFFER__H__
