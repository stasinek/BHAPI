/*
 * Copyright 2013-2015, Rene Gollent, rene@gollent.com.
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef ARRAY_VALUE_NODE_H
#define ARRAY_VALUE_NODE_H


#include <ObjectList.h>

#include <ValueNode.h>


class AbstractArrayValueNodeChild;
class ArrayType;


class AbstractArrayValueNode : public ValueNode {
public:
								AbstractArrayValueNode(
									ValueNodeChild* nodeChild, ArrayType* type,
									int32_t dimension);
	virtual						~AbstractArrayValueNode();

			ArrayType*			GetArrayType() const
									{ return fType; }
			int32_t				Dimension() const
									{ return fDimension; }

	virtual	Type*				GetType() const;

	virtual	status_t			ResolvedLocationAndValue(
									ValueLoader* valueLoader,
									ValueLocation*& _location,
									Value*& _value);

			// locking required

	virtual	status_t			CreateChildren(TeamTypeInformation* info);
	virtual	int32_t				CountChildren() const;
	virtual	ValueNodeChild*		ChildAt(int32_t index) const;

	virtual	bool				IsRangedContainer() const;
	virtual	void				ClearChildren();
	virtual	status_t			CreateChildrenInRange(
									TeamTypeInformation* info,
									int32_t lowIndex, int32_t highIndex);
	virtual	status_t			SupportedChildRange(int32_t& lowIndex,
									int32_t& highIndex) const;
protected:
			typedef BObjectList<AbstractArrayValueNodeChild> ChildList;

protected:
			ArrayType*			fType;
			ChildList			fChildren;
			int32_t				fDimension;
			int32_t				fLowerBound;
			int32_t				fUpperBound;
			bool				fBoundsInitialized;
};


// TODO: Are ArrayValueNode and InternalArrayValueNode still needed?

class ArrayValueNode : public AbstractArrayValueNode {
public:
								ArrayValueNode(ValueNodeChild* nodeChild,
									ArrayType* type);
	virtual						~ArrayValueNode();
};


class InternalArrayValueNode : public AbstractArrayValueNode {
public:
								InternalArrayValueNode(
									ValueNodeChild* nodeChild,
									ArrayType* type, int32_t dimension);
	virtual						~InternalArrayValueNode();
};


class AbstractArrayValueNodeChild : public ValueNodeChild {
public:
								AbstractArrayValueNodeChild(
									AbstractArrayValueNode* parent,
									const BString& name, int64 elementIndex);
	virtual						~AbstractArrayValueNodeChild();

			AbstractArrayValueNode* ArrayParent() const	{ return fParent; }
			int32_t				ElementIndex() const { return fElementIndex; }

	virtual	const BString&		Name() const;
	virtual	ValueNode*			Parent() const;

protected:
			AbstractArrayValueNode* fParent;
			BString				fName;
			int64				fElementIndex;
};


class ArrayValueNodeChild : public AbstractArrayValueNodeChild {
public:
								ArrayValueNodeChild(
									AbstractArrayValueNode* parent,
									const BString& name, int64 elementIndex,
									Type* type);
	virtual						~ArrayValueNodeChild();

	virtual	Type*				GetType() const;

	virtual	status_t			ResolveLocation(ValueLoader* valueLoader,
									ValueLocation*& _location);

private:
			Type*				fType;
};


class InternalArrayValueNodeChild : public AbstractArrayValueNodeChild {
public:
								InternalArrayValueNodeChild(
									AbstractArrayValueNode* parent,
									const BString& name, int64 elementIndex,
									ArrayType* type);
	virtual						~InternalArrayValueNodeChild();

	virtual	Type*				GetType() const;

	virtual	bool				IsInternal() const;
	virtual	status_t			CreateInternalNode(ValueNode*& _node);

	virtual	status_t			ResolveLocation(ValueLoader* valueLoader,
									ValueLocation*& _location);

private:
			ArrayType*			fType;
};


#endif	// ARRAY_VALUE_NODE_H
