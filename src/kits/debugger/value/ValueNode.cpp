/*
 * Copyright 2015, Rene Gollent, rene@gollent.com.
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include <ValueNode.h>

#include <Value.h>
#include <ValueLocation.h>
#include <ValueNodeContainer.h>


// #pragma mark - ValueNode


ValueNode::ValueNode(ValueNodeChild* nodeChild)
	:
	fContainer(NULL),
	fNodeChild(nodeChild),
	fLocation(NULL),
	fValue(NULL),
	fLocationResolutionState(VALUE_NODE_UNRESOLVED),
	fChildrenCreated(false)
{
	fNodeChild->AcquireReference();
}


ValueNode::~ValueNode()
{
	SetLocationAndValue(NULL, NULL, VALUE_NODE_UNRESOLVED);
	SetContainer(NULL);
	fNodeChild->ReleaseReference();
}


const BString&
ValueNode::Name() const
{
	return fNodeChild->Name();
}


void ValueNode::SetContainer(ValueNodeContainer* container)
{
	if (container == fContainer)
		return;

	if (fContainer != NULL)
		fContainer->ReleaseReference();

	fContainer = container;

	if (fContainer != NULL)
		fContainer->AcquireReference();

	// propagate to children
	int32_t childCount = CountChildren();
	for (int32_t i = 0; i < childCount; i++)
		ChildAt(i)->SetContainer(fContainer);
}


bool ValueNode::IsRangedContainer() const
{
	return false;
}


bool ValueNode::IsContainerRangeFixed() const
{
	return false;
}


void ValueNode::ClearChildren()
{
	// do nothing
}


status_t ValueNode::CreateChildrenInRange(TeamTypeInformation* info, int32_t lowIndex,
	int32_t highIndex)
{
	return B_NOT_SUPPORTED;
}


status_t ValueNode::SupportedChildRange(int32_t& lowIndex, int32_t& highIndex) const
{
	return B_NOT_SUPPORTED;
}


void ValueNode::SetLocationAndValue(ValueLocation* location, Value* value,
	status_t resolutionState)
{
	if (fLocation != location) {
		if (fLocation != NULL)
			fLocation->ReleaseReference();

		fLocation = location;

		if (fLocation != NULL)
			fLocation->AcquireReference();
	}

	if (fValue != value) {
		if (fValue != NULL)
			fValue->ReleaseReference();

		fValue = value;

		if (fValue != NULL)
			fValue->AcquireReference();
	}

	fLocationResolutionState = resolutionState;

	// notify listeners
	if (fContainer != NULL)
		fContainer->NotifyValueNodeValueChanged(this);
}


// #pragma mark - ValueNodeChild


ValueNodeChild::ValueNodeChild()
	:
	fContainer(NULL),
	fNode(NULL),
	fLocation(NULL),
	fLocationResolutionState(VALUE_NODE_UNRESOLVED)
{
}


ValueNodeChild::~ValueNodeChild()
{
	SetLocation(NULL, VALUE_NODE_UNRESOLVED);
	SetNode(NULL);
	SetContainer(NULL);
}


bool ValueNodeChild::IsInternal() const
{
	return false;
}


status_t ValueNodeChild::CreateInternalNode(ValueNode*& _node)
{
	return B_BAD_VALUE;
}


void ValueNodeChild::SetContainer(ValueNodeContainer* container)
{
	if (container == fContainer)
		return;

	if (fContainer != NULL)
		fContainer->ReleaseReference();

	fContainer = container;

	if (fContainer != NULL)
		fContainer->AcquireReference();

	// propagate to node
	if (fNode != NULL)
		fNode->SetContainer(fContainer);
}


void ValueNodeChild::SetNode(ValueNode* node)
{
	if (node == fNode)
		return;

	ValueNode* oldNode = fNode;
	BReference<ValueNode> oldNodeReference(oldNode, true);

	if (fNode != NULL)
		fNode->SetContainer(NULL);

	fNode = node;

	if (fNode != NULL) {
		fNode->AcquireReference();
		fNode->SetContainer(fContainer);
	}

	if (fContainer != NULL)
		fContainer->NotifyValueNodeChanged(this, oldNode, fNode);
}


ValueLocation*
ValueNodeChild::Location() const
{
	return fLocation;
}


void ValueNodeChild::SetLocation(ValueLocation* location, status_t resolutionState)
{
	if (fLocation != location) {
		if (fLocation != NULL)
			fLocation->ReleaseReference();

		fLocation = location;

		if (fLocation != NULL)
			fLocation->AcquireReference();
	}

	fLocationResolutionState = resolutionState;
}


// #pragma mark - ChildlessValueNode


ChildlessValueNode::ChildlessValueNode(ValueNodeChild* nodeChild)
	:
	ValueNode(nodeChild)
{
	fChildrenCreated = true;
}


status_t ChildlessValueNode::CreateChildren(TeamTypeInformation* info)
{
	return B_OK;
}

int32_t ChildlessValueNode::CountChildren() const
{
	return 0;
}


ValueNodeChild*
ChildlessValueNode::ChildAt(int32_t index) const
{
	return NULL;
}
