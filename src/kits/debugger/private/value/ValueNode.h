/*
 * Copyright 2015, Rene Gollent, rene@gollent.com.
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef VALUE_NODE_H
#define VALUE_NODE_H


#include <kits/support/String.h>

#include <Referenceable.h>


class TeamTypeInformation;
class Type;
class Value;
class ValueLoader;
class ValueLocation;
class ValueNodeChild;
class ValueNodeContainer;


enum {
	VALUE_NODE_UNRESOLVED	= 1
};


class ValueNode : public BReferenceable {
public:
								ValueNode(ValueNodeChild* nodeChild);
	virtual						~ValueNode();

			ValueNodeChild*		NodeChild() const	{ return fNodeChild; }

	virtual	const BString&		Name() const;
	virtual	Type*				GetType() const = 0;

	virtual	status_t			ResolvedLocationAndValue(
									ValueLoader* valueLoader,
									ValueLocation*& _location,
									Value*& _value) = 0;
									// returns references, a NULL value can be
									// returned

			// locking required

			ValueNodeContainer*	Container() const
									{ return fContainer; }
			void				SetContainer(ValueNodeContainer* container);

			virtual bool		ChildCreationNeedsValue() const
									{ return false; }
			bool				ChildrenCreated() const
									{ return fChildrenCreated; }

	virtual	status_t			CreateChildren(TeamTypeInformation* info) = 0;
	virtual	int32_t				CountChildren() const = 0;
	virtual	ValueNodeChild*		ChildAt(int32_t index) const = 0;

	// optional virtual hooks for container type value nodes such as
	// arrays that may contain a variable (and potentially quite large)
	// number of children. The calls below should be implemented for such
	// node types to allow the upper layers to be aware of this, and to be
	// able to request that only a subset of children be created.
	virtual	bool				IsRangedContainer() const;
	virtual	bool				IsContainerRangeFixed() const;
									// indicates that the user can't
									// arbitrarily go outside of the
									// specified/supported range.
	virtual	void				ClearChildren();
	virtual	status_t			CreateChildrenInRange(
									TeamTypeInformation* info,
									int32_t lowIndex, int32_t highIndex);
	virtual	status_t			SupportedChildRange(int32_t& lowIndex,
									int32_t& highIndex) const;

			status_t			LocationAndValueResolutionState() const
									{ return fLocationResolutionState; }
			void				SetLocationAndValue(ValueLocation* location,
									Value* value, status_t resolutionState);
			ValueLocation*		Location() const	{ return fLocation; }
			Value*				GetValue() const	{ return fValue; }
									// immutable after SetLocationAndValue()

protected:
			ValueNodeContainer*	fContainer;
			ValueNodeChild*		fNodeChild;
			ValueLocation*		fLocation;
			Value*				fValue;
			status_t			fLocationResolutionState;
			bool				fChildrenCreated;
};


class ValueNodeChild : public BReferenceable {
public:
								ValueNodeChild();
	virtual						~ValueNodeChild();

	virtual	const BString&		Name() const = 0;
	virtual	Type*				GetType() const = 0;
	virtual	ValueNode*			Parent() const = 0;

	virtual	bool				IsInternal() const;
	virtual	status_t			CreateInternalNode(ValueNode*& _node);

	virtual	status_t			ResolveLocation(ValueLoader* valueLoader,
									ValueLocation*& _location) = 0;
									// returns a reference

			// locking required

			ValueNodeContainer*	Container() const
									{ return fContainer; }
			void				SetContainer(ValueNodeContainer* container);

			ValueNode*			Node() const	{ return fNode; }
			void				SetNode(ValueNode* node);

			status_t			LocationResolutionState() const
									{ return fLocationResolutionState; }
			ValueLocation*		Location() const;
									// immutable after SetLocation()
			void				SetLocation(ValueLocation* location,
									status_t resolutionState);

protected:
			ValueNodeContainer*	fContainer;
			ValueNode*			fNode;
			ValueLocation*		fLocation;
			status_t			fLocationResolutionState;
};


class ChildlessValueNode : public ValueNode {
public:
								ChildlessValueNode(ValueNodeChild* nodeChild);

	virtual	status_t			CreateChildren(TeamTypeInformation* info);
	virtual	int32_t				CountChildren() const;
	virtual	ValueNodeChild*		ChildAt(int32_t index) const;
};


#endif	// VALUE_NODE_H
