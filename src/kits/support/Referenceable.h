/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku OS source-code, All Rights Reserved
 *
 * BHAPI++ library is a freeware; it may be used and distributed according to
 * the terms of The MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File: Referencable.h
 *
 * --------------------------------------------------------------------------*/

/*
 * Copyright 2004-2010, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_REFERENCEABLE_H
#define BHAPI_REFERENCEABLE_H

#include <kits/support/SupportDefs.h>

// #pragma mark - BReferenceable

class BReferenceable {
public:
								BReferenceable();
	virtual						~BReferenceable();

								// acquire and release return
								// the previous ref count
			int32_t				AcquireReference();
			int32_t				ReleaseReference();

			int32_t				CountReferences() const
									{ return fReferenceCount; }

protected:
	virtual	void				FirstReferenceAcquired();
	virtual	void				LastReferenceReleased();

protected:
			int32_t				fReferenceCount;
};


// #pragma mark - BReference


template<typename Type = BReferenceable> class BReference {
public:
	BReference()
		:
		fObject(NULL)
	{
	}

	BReference(Type* object, bool alreadyHasReference = false)
		:
		fObject(NULL)
	{
		SetTo(object, alreadyHasReference);
	}

	BReference(const BReference<Type>& other)
		:
		fObject(NULL)
	{
		SetTo(other.Get());
	}

	template<typename OtherType>
	BReference(const BReference<OtherType>& other)
		:
		fObject(NULL)
	{
		SetTo(other.Get());
	}

	~BReference()
	{
		Unset();
	}

	void SetTo(Type* object, bool alreadyHasReference = false)
	{
		if (object != NULL && !alreadyHasReference)
			object->AcquireReference();

		Unset();

		fObject = object;
	}

	void Unset()
	{
		if (fObject) {
			fObject->ReleaseReference();
			fObject = NULL;
		}
	}

	Type* Get() const
	{
		return fObject;
	}

	Type* Detach()
	{
		Type* object = fObject;
		fObject = NULL;
		return object;
	}

	Type& operator*() const
	{
		return *fObject;
	}

	Type* operator->() const
	{
		return fObject;
	}

	operator Type*() const
	{
		return fObject;
	}

	BReference& operator=(const BReference<Type>& other)
	{
		SetTo(other.fObject);
		return *this;
	}

	BReference& operator=(Type* other)
	{
		SetTo(other);
		return *this;
	}

	template<typename OtherType>
	BReference& operator=(const BReference<OtherType>& other)
	{
		SetTo(other.Get());
		return *this;
	}

	bool operator==(const BReference<Type>& other) const
	{
		return fObject == other.fObject;
	}

	bool operator==(const Type* other) const
	{
		return fObject == other;
	}

	bool operator!=(const BReference<Type>& other) const
	{
		return fObject != other.fObject;
	}

	bool operator!=(const Type* other) const
	{
		return fObject != other;
	}

private:
	Type*	fObject;
};


// #pragma mark - BReference<const>


template<typename Type> class BReference<const Type> {
public:
	BReference(Type* object, bool alreadyHasReference = false)
		:
		fReference(object, alreadyHasReference)
	{
	}

	BReference(const BReference<const Type>& other)
		:
		fReference(const_cast<Type*>(other.Get()))
	{
	}

	template<typename OtherType>
	BReference(const BReference<OtherType>& other)
		:
		fReference(other.Get())
	{
	}

	void SetTo(Type* object, bool alreadyHasReference = false)
	{
		fReference.SetTo(object, alreadyHasReference);
	}

	void Unset()
	{
		fReference.Unset();
	}

	const Type* Get() const
	{
		return fReference.Get();
	}

	const Type* Detach()
	{
		return fReference.Detach();
	}

	const Type& operator*() const
	{
		return *fReference;
	}

	const Type* operator->() const
	{
		return fReference.Get();
	}

	operator const Type*() const
	{
		return fReference.Get();
	}

	BReference& operator=(const BReference<const Type>& other)
	{
		fReference = other.fReference;
	}

	BReference& operator=(Type* other)
	{
		fReference = other;
	}

	template<typename OtherType>
	BReference& operator=(const BReference<OtherType>& other)
	{
		fReference = other.Get();
	}

	bool operator==(const BReference<const Type>& other) const
	{
		return fReference == other.Get();
	}

	bool operator==(const Type* other) const
	{
		return fReference == other;
	}

	bool operator!=(const BReference<const Type>& other) const
	{
		return fReference != other.Get();
	}

	bool operator!=(const Type* other) const
	{
		return fReference != other;
	}

private:
	BReference<Type> fReference;
};

#endif	// BHAPI_REFERENCEABLE_H
