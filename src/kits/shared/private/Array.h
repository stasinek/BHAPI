/*
 * Copyright 2009-2011, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ARRAY_H
#define _ARRAY_H


#include <stdio.h>
#include <stdlib.h>
#include <kits/support/String.h>

#include <Haiku.h>

#if DEBUG
#	include <kits/kernel/OS.h>
#endif


namespace BPrivate {


template<typename Element>
class Array {
public:
	inline						Array();
								Array(const Array<Element>& other);
	inline						~Array();

	inline	int32_t				Size() const		{ return fSize; }
	inline	int32_t				Count() const		{ return fSize; }
	inline	bool				IsEmpty() const		{ return fSize == 0; }
	inline	Element*			Elements() const	{ return fElements; }

	inline	bool				Add(const Element& element);
	inline	bool				AddUninitialized(int32_t elementCount);
	inline	bool				Insert(const Element& element, int32_t index);
	inline	bool				InsertUninitialized(int32_t index, int32_t count);
	inline	bool				Remove(int32_t index, int32_t count = 1);

			void				Clear();
	inline	void				MakeEmpty();

	inline	Element&			ElementAt(int32_t index);
	inline	const Element&		ElementAt(int32_t index) const;

	inline	Element&			operator[](int32_t index);
	inline	const Element&		operator[](int32_t index) const;

			Array<Element>&		operator=(const Array<Element>& other);

private:
	static	const int32_t			kMinCapacity = 8;

			bool				_Resize(int32_t index, int32_t delta);

private:
			Element*			fElements;
			int32_t				fSize;
			int32_t				fCapacity;
};


template<typename Element>
Array<Element>::Array()
	:
	fElements(NULL),
	fSize(0),
	fCapacity(0)
{
}


template<typename Element>
Array<Element>::Array(const Array<Element>& other)
	:
	fElements(NULL),
	fSize(0),
	fCapacity(0)
{
	*this = other;
}


template<typename Element>
Array<Element>::~Array()
{
	free(fElements);
}


template<typename Element>
bool Array<Element>::Add(const Element& element)
{
	if (!_Resize(fSize, 1))
		return false;

	fElements[fSize] = element;
	fSize++;
	return true;
}


template<typename Element>
inline bool Array<Element>::AddUninitialized(int32_t elementCount)
{
	return InsertUninitialized(fSize, elementCount);
}


template<typename Element>
bool Array<Element>::Insert(const Element& element, int32_t index)
{
	if (index < 0 || index > fSize)
		index = fSize;

	if (!_Resize(index, 1))
		return false;

	fElements[index] = element;
	fSize++;
	return true;
}


template<typename Element>
bool Array<Element>::InsertUninitialized(int32_t index, int32_t count)
{
	if (index < 0 || index > fSize || count < 0)
		return false;
	if (count == 0)
		return true;

	if (!_Resize(index, count))
		return false;

	fSize += count;
	return true;
}


template<typename Element>
bool Array<Element>::Remove(int32_t index, int32_t count)
{
	if (index < 0 || count < 0 || index + count > fSize) {
#if DEBUG
		char buffer[128];
		snprintf(buffer, sizeof(buffer), "Array::Remove(): index: %" B_PRId32
		", count: %" B_PRId32 ", size: %" B_PRId32, index, count, fSize);
		debugger(buffer);
#endif
		return false;
	}
	if (count == 0)
		return true;

	if (index + count < fSize) {
		memmove(fElements + index, fElements + index + count,
			sizeof(Element) * (fSize - index - count));
	}

	_Resize(index, -count);

	fSize -= count;
	return true;
}


template<typename Element>
void Array<Element>::Clear()
{
	if (fSize == 0)
		return;

	free(fElements);

	fElements = NULL;
	fSize = 0;
	fCapacity = 0;
}


template<typename Element>
void Array<Element>::MakeEmpty()
{
	Clear();
}


template<typename Element>
Element&
Array<Element>::ElementAt(int32_t index)
{
	return fElements[index];
}


template<typename Element>
const Element&
Array<Element>::ElementAt(int32_t index) const
{
	return fElements[index];
}


template<typename Element>
Element&
Array<Element>::operator[](int32_t index)
{
	return fElements[index];
}


template<typename Element>
const Element&
Array<Element>::operator[](int32_t index) const
{
	return fElements[index];
}


template<typename Element>
Array<Element>&
Array<Element>::operator=(const Array<Element>& other)
{
	Clear();

	if (other.fSize > 0 && _Resize(0, other.fSize)) {
		fSize = other.fSize;
		memcpy(fElements, other.fElements, fSize * sizeof(Element));
	}

	return *this;
}


template<typename Element>
bool Array<Element>::_Resize(int32_t index, int32_t delta)
{
	// determine new capacity
	int32_t newSize = fSize + delta;
	int32_t newCapacity = kMinCapacity;
	while (newCapacity < newSize)
		newCapacity *= 2;

	if (newCapacity == fCapacity) {
		// the capacity doesn't change -- still make room for/remove elements
		if (index < fSize) {
			if (delta > 0) {
				// leave a gap of delta elements
				memmove(fElements + index + delta, fElements + index,
					(fSize - index) * sizeof(Element));
			} else if (index < fSize + delta) {
				// drop -delta elements
				memcpy(fElements + index, fElements + index - delta,
					(fSize - index + delta) * sizeof(Element));
			}
		}

		return true;
	}

	// allocate new array
	Element* elements = (Element*)malloc(newCapacity * sizeof(Element));
	if (elements == NULL)
		return false;

	if (index > 0)
		memcpy(elements, fElements, index * sizeof(Element));
	if (index < fSize) {
		if (delta > 0) {
			// leave a gap of delta elements
			memcpy(elements + index + delta, fElements + index,
				(fSize - index) * sizeof(Element));
		} else if (index < fSize + delta) {
			// drop -delta elements
			memcpy(elements + index, fElements + index - delta,
				(fSize - index + delta) * sizeof(Element));
		}
	}

	free(fElements);
	fElements = elements;
	fCapacity = newCapacity;
	return true;
}


}	// namespace BPrivate


using BPrivate::Array;


#endif	// _ARRAY_H
