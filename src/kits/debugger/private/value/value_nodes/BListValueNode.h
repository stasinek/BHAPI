/*
 * Copyright 2012-2015, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef BLIST_VALUE_NODE_H
#define BLIST_VALUE_NODE_H


#include <kits/support/List.h>
#include <Variant.h>

#include <ObjectList.h>

#include <ValueLocation.h>
#include <ValueNode.h>


class CompoundType;


class BListValueNode : public ValueNode {
public:
									BListValueNode(ValueNodeChild* nodeChild,
										Type* type);
	virtual							~BListValueNode();

	virtual	Type*					GetType() const;
	virtual	status_t				ResolvedLocationAndValue(
										ValueLoader* valueLoader,
										ValueLocation*& _location,
										Value*& _value);

	virtual bool					ChildCreationNeedsValue() const
										{ return true; }
	virtual status_t				CreateChildren(TeamTypeInformation* info);
	virtual int32_t					CountChildren() const;
	virtual ValueNodeChild*			ChildAt(int32_t index) const;

	virtual	bool					IsRangedContainer() const;
	virtual	bool					IsContainerRangeFixed() const;
	virtual	void					ClearChildren();
	virtual	status_t				CreateChildrenInRange(
										TeamTypeInformation* info,
										int32_t lowIndex, int32_t highIndex);
	virtual	status_t				SupportedChildRange(int32_t& lowIndex,
										int32_t& highIndex) const;
private:
			class BListElementNodeChild;
			class BListItemCountNodeChild;

			// for GCC2
			friend class BListElementNodeChild;
			friend class BListItemCountNodeChild;

			typedef BObjectList<ValueNodeChild> ChildNodeList;

private:

			Type*					fType;
			ChildNodeList			fChildren;
			BVariant				fDataLocation;
			BVariant				fItemCountLocation;
			Type*					fItemCountType;
			int32_t					fItemCount;
			bool					fCountChildCreated;
};


#endif	// BLIST_VALUE_NODE_H
