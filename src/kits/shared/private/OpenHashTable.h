/*
Open Tracker License

Terms and Conditions

Copyright (c) 1991-2000, Be Incorporated. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice applies to all licensees
and shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF TITLE, MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
BE INCORPORATED BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Be Incorporated shall not be
used in advertising or otherwise to promote the sale, use or other dealings in
this Software without prior written authorization from Be Incorporated.

Tracker(TM), Be(R), BeOS(R), and BeIA(TM) are trademarks or registered trademarks
of Be Incorporated in the United States and other countries. Other brand product
names are registered trademarks or trademarks of their respective holders.
All rights reserved.
*/

// bonefish:
// * removed need for exceptions
// * fixed warnings
// * implemented rehashing
// * added RemoveAll()
// TODO:
// * shrinking of element vectors

// Hash table with open addresssing

#ifndef __OPEN_HASH_TABLE__
#define __OPEN_HASH_TABLE__

#include <stdlib.h>
#include <new>

// don't include <Debug.h>
#ifndef _OPEN_HASH_TABLE_ASSERT
#	define _OPEN_HASH_TABLE_ASSERT(E)	(void)0
#endif
#ifndef _OPEN_HASH_TABLE_TRESPASS
#	define _OPEN_HASH_TABLE_TRESPASS()	(void)0
#endif

namespace BPrivate {

template <class Element>
class ElementVector {
	// element vector for OpenHashTable needs to implement this
	// interface
public:
	Element &At(int32_t index);
	Element *Add();
	int32_t IndexOf(const Element &) const;
	void Remove(int32_t index);
};

class OpenHashElement {
public:
	uint32_t Hash() const;
	bool operator==(const OpenHashElement &) const;
	void Adopt(OpenHashElement &);
		// low overhead copy, original element is in undefined state
		// after call (calls Adopt on BString members, etc.)
	int32_t fNext;
};

const uint32_t kPrimes [] = {
	509, 1021, 2039, 4093, 8191, 16381, 32749, 65521, 131071, 262139,
	524287, 1048573, 2097143, 4194301, 8388593, 16777213, 33554393, 67108859,
	134217689, 268435399, 536870909, 1073741789, 2147483647, 0
};

template <class Element, class ElementVec = ElementVector<Element> >
class OpenHashTable {
public:
	OpenHashTable(int32_t minSize, ElementVec *elementVector = 0,
				  float maxLoadFactor = 0.8);
		// it is up to the subclass of OpenHashTable to supply
		// elementVector
	~OpenHashTable();

	bool InitCheck() const;

	void SetElementVector(ElementVec *elementVector);

	Element *FindFirst(uint32_t elementHash) const;
	Element *Add(uint32_t elementHash);

	void Remove(Element *element, bool dontRehash = false);
	void RemoveAll();

	// when calling Add, any outstanding element pointer may become
	// invalid; to deal with this, get the element index and restore
	// it after the add
	int32_t ElementIndex(const Element *) const;
	Element *ElementAt(int32_t index) const;

	int32_t ArraySize() const;
	int32_t VectorSize() const;
	int32_t CountElements() const;

protected:
	static int32_t OptimalSize(int32_t minSize);

private:
	bool _RehashIfNeeded();
	bool _Rehash();

	int32_t fArraySize;
	int32_t fInitialSize;
	int32_t fElementCount;
	int32_t *fHashArray;
	ElementVec *fElementVector;
	float fMaxLoadFactor;
};

template <class Element>
class OpenHashElementArray : public ElementVector<Element> {
	// this is a straightforward implementation of an element vector
	// deleting is handled by linking deleted elements into a free list
	// the vector never shrinks
public:
	OpenHashElementArray(int32_t initialSize);
	~OpenHashElementArray();

	bool InitCheck() const;

	Element &At(int32_t index);
	const Element &At(int32_t index) const;
	Element *Add(const Element &);
	Element *Add();
	void Remove(int32_t index);
	int32_t IndexOf(const Element &) const;
	int32_t Size() const;

private:
	Element *fData;
	int32_t fSize;
	int32_t fNextFree;
	int32_t fNextDeleted;
};


//-----------------------------------

template<class Element, class ElementVec>
OpenHashTable<Element, ElementVec>::OpenHashTable(int32_t minSize,
	ElementVec *elementVector, float maxLoadFactor)
	:	fArraySize(OptimalSize(minSize)),
		fInitialSize(fArraySize),
		fElementCount(0),
		fElementVector(elementVector),
		fMaxLoadFactor(maxLoadFactor)
{
	// sanity check the maximal load factor
	if (fMaxLoadFactor < 0.5)
		fMaxLoadFactor = 0.5;
	// allocate and init the array
	fHashArray = (int32_t*)calloc(fArraySize, sizeof(int32_t));
	if (fHashArray) {
		for (int32_t index = 0; index < fArraySize; index++)
			fHashArray[index] = -1;
	}
}

template<class Element, class ElementVec>
OpenHashTable<Element, ElementVec>::~OpenHashTable()
{
	RemoveAll();
	free(fHashArray);
}

template<class Element, class ElementVec>
bool OpenHashTable<Element, ElementVec>::InitCheck() const
{
	return (fHashArray && fElementVector);
}

template<class Element, class ElementVec>
int32_t OpenHashTable<Element, ElementVec>::OptimalSize(int32_t minSize)
{
	for (int32_t index = 0; ; index++)
		if (!kPrimes[index] || kPrimes[index] >= (uint32_t)minSize)
			return (int32_t)kPrimes[index];

	return 0;
}

template<class Element, class ElementVec>
Element *
OpenHashTable<Element, ElementVec>::FindFirst(uint32_t hash) const
{
	_OPEN_HASH_TABLE_ASSERT(fElementVector);
	hash %= fArraySize;
	if (fHashArray[hash] < 0)
		return 0;

	return &fElementVector->At(fHashArray[hash]);
}

template<class Element, class ElementVec>
int32_t OpenHashTable<Element, ElementVec>::ElementIndex(const Element *element) const
{
	return fElementVector->IndexOf(*element);
}

template<class Element, class ElementVec>
Element *
OpenHashTable<Element, ElementVec>::ElementAt(int32_t index) const
{
	return &fElementVector->At(index);
}

template<class Element, class ElementVec>
int32_t OpenHashTable<Element, ElementVec>::ArraySize() const
{
	return fArraySize;
}

template<class Element, class ElementVec>
int32_t OpenHashTable<Element, ElementVec>::VectorSize() const
{
	return fElementVector->Size();
}

template<class Element, class ElementVec>
int32_t OpenHashTable<Element, ElementVec>::CountElements() const
{
	return fElementCount;
}


template<class Element, class ElementVec>
Element *
OpenHashTable<Element, ElementVec>::Add(uint32_t hash)
{
	_OPEN_HASH_TABLE_ASSERT(fElementVector);
	_RehashIfNeeded();
	hash %= fArraySize;
	Element *result = fElementVector->Add();
	if (result) {
		result->fNext = fHashArray[hash];
		fHashArray[hash] = fElementVector->IndexOf(*result);
		fElementCount++;
	}
	return result;
}

template<class Element, class ElementVec>
void OpenHashTable<Element, ElementVec>::Remove(Element *element, bool dontRehash)
{
	if (!dontRehash)
		_RehashIfNeeded();
	uint32_t hash = element->Hash() % fArraySize;
	int32_t next = fHashArray[hash];
	_OPEN_HASH_TABLE_ASSERT(next >= 0);

	if (&fElementVector->At(next) == element) {
		fHashArray[hash] = element->fNext;
		fElementVector->Remove(next);
		fElementCount--;
		return;
	}

	for (int32_t index = next; index >= 0; ) {
		// look for an existing match in table
		next = fElementVector->At(index).fNext;
		if (next < 0) {
			_OPEN_HASH_TABLE_TRESPASS();
			return;
		}

		if (&fElementVector->At(next) == element) {
			fElementVector->At(index).fNext = element->fNext;
			fElementVector->Remove(next);
			fElementCount--;
			return;
		}
		index = next;
	}
}

template<class Element, class ElementVec>
void OpenHashTable<Element, ElementVec>::RemoveAll()
{
	for (int32_t i = 0; fElementCount > 0 && i < fArraySize; i++) {
		int32_t index = fHashArray[i];
		while (index >= 0) {
			Element* element = &fElementVector->At(index);
			int32_t next = element->fNext;
			fElementVector->Remove(index);
			fElementCount--;
			index = next;
		}
		fHashArray[i] = -1;
	}
	_RehashIfNeeded();
}

template<class Element, class ElementVec>
void OpenHashTable<Element, ElementVec>::SetElementVector(ElementVec *elementVector)
{
	fElementVector = elementVector;
}

// _RehashIfNeeded
template<class Element, class ElementVec>
bool OpenHashTable<Element, ElementVec>::_RehashIfNeeded()
{
	// The load factor range [fMaxLoadFactor / 3, fMaxLoadFactor] is fine,
	// I think. After rehashing the load factor will be about
	// fMaxLoadFactor * 2 / 3, respectively fMaxLoadFactor / 2.
	float loadFactor = (float)fElementCount / (float)fArraySize;
	if (loadFactor > fMaxLoadFactor
		|| (fArraySize > fInitialSize && loadFactor < fMaxLoadFactor / 3)) {
		return _Rehash();
	}
	return true;
}

// _Rehash
template<class Element, class ElementVec>
bool OpenHashTable<Element, ElementVec>::_Rehash()
{
	bool result = true;
	int32_t newSize = int32_t(fElementCount * 1.73 * fMaxLoadFactor);
	newSize = (fInitialSize > newSize ? fInitialSize : newSize);
	if (newSize != fArraySize) {
		// allocate a new array
		int32_t *newHashArray = (int32_t*)calloc(newSize, sizeof(int32_t));
		if (newHashArray) {
			// init the new hash array
			for (int32_t index = 0; index < newSize; index++)
				newHashArray[index] = -1;
			// iterate through all elements and put them into the new
			// hash array
			for (int i = 0; i < fArraySize; i++) {
				int32_t index = fHashArray[i];
				while (index >= 0) {
					// insert the element in the new array
					Element &element = fElementVector->At(index);
					int32_t next = element.fNext;
					uint32_t hash = (element.Hash() % newSize);
					element.fNext = newHashArray[hash];
					newHashArray[hash] = index;
					// next element in old list
					index = next;
				}
			}
			// delete the old array and set the new one
			free(fHashArray);
			fHashArray = newHashArray;
			fArraySize = newSize;
		} else
			result = false;
	}
	return result;
}


template<class Element>
OpenHashElementArray<Element>::OpenHashElementArray(int32_t initialSize)
	:	fSize(initialSize),
		fNextFree(0),
		fNextDeleted(-1)
{
	fData = (Element*)calloc((size_t)initialSize, sizeof(Element));
}

template<class Element>
OpenHashElementArray<Element>::~OpenHashElementArray()
{
	free(fData);
}

template<class Element>
bool OpenHashElementArray<Element>::InitCheck() const
{
	return fData;
}

template<class Element>
Element &
OpenHashElementArray<Element>::At(int32_t index)
{
	_OPEN_HASH_TABLE_ASSERT(index < fSize);
	return fData[index];
}

template<class Element>
const Element &
OpenHashElementArray<Element>::At(int32_t index) const
{
	_OPEN_HASH_TABLE_ASSERT(index < fSize);
	return fData[index];
}

template<class Element>
int32_t OpenHashElementArray<Element>::IndexOf(const Element &element) const
{
	int32_t result = &element - fData;
	if (result < 0 || result > fSize)
		return -1;

	return result;
}

template<class Element>
int32_t OpenHashElementArray<Element>::Size() const
{
	return fSize;
}


template<class Element>
Element *
OpenHashElementArray<Element>::Add(const Element &newElement)
{
	Element *element = Add();
	if (element)
		element->Adopt(newElement);
	return element;
}

#if DEBUG
const int32_t kGrowChunk = 10;
#else
const int32_t kGrowChunk = 1024;
#endif

template<class Element>
Element *
OpenHashElementArray<Element>::Add()
{
	int32_t index = fNextFree;
	if (fNextDeleted >= 0) {
		index = fNextDeleted;
		fNextDeleted = At(index).fNext;
	} else if (fNextFree >= fSize - 1) {
		int32_t newSize = fSize + kGrowChunk;
/*
		Element *newData = (Element *)calloc((size_t)newSize , sizeof(Element));
		if (!newData)
			return NULL;
		memcpy(newData, fData, fSize * sizeof(Element));
		free(fData);
*/
		Element *newData = (Element*)realloc(fData,
			(size_t)newSize * sizeof(Element));
		if (!newData)
			return NULL;

		fData = newData;
		fSize = newSize;
		index = fNextFree;
		fNextFree++;
	} else
		fNextFree++;

	new (&At(index)) Element;
		// call placement new to initialize the element properly
	_OPEN_HASH_TABLE_ASSERT(At(index).fNext == -1);

	return &At(index);
}

template<class Element>
void OpenHashElementArray<Element>::Remove(int32_t index)
{
	// delete by chaining empty elements in a single linked
	// list, reusing the next field
	_OPEN_HASH_TABLE_ASSERT(index < fSize);
	At(index).~Element();
		// call the destructor explicitly to destroy the element
		// properly
	At(index).fNext = fNextDeleted;
	fNextDeleted = index;
}

} // namespace BPrivate

using BPrivate::OpenHashTable;

#endif // __OPEN_HASH_TABLE__
