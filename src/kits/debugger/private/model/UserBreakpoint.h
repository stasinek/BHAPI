/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Copyright 2013-2014, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef USER_BREAKPOINT_H
#define USER_BREAKPOINT_H


#include <ObjectList.h>
#include <Referenceable.h>
#include <util/DoublyLinkedList.h>

#include <SourceLocation.h>
#include <kits/support/String.h>
#include <Types.h>


class Breakpoint;
class Function;
class FunctionID;
class LocatableFile;
class UserBreakpoint;


class UserBreakpointLocation {
public:
								UserBreakpointLocation(FunctionID* functionID,
									LocatableFile* sourceFile,
									const SourceLocation& sourceLocation,
									target_addr_t relativeAddress);
								UserBreakpointLocation(
									const UserBreakpointLocation& other);
	virtual						~UserBreakpointLocation();

			FunctionID*			GetFunctionID() const	{ return fFunctionID; }
			LocatableFile*		SourceFile() const		{ return fSourceFile; }
			SourceLocation		GetSourceLocation() const
									{ return fSourceLocation; }
			target_addr_t		RelativeAddress() const
									{ return fRelativeAddress; }

			UserBreakpointLocation& operator=(
									const UserBreakpointLocation& other);

private:
			FunctionID*			fFunctionID;
			LocatableFile*		fSourceFile;
			SourceLocation		fSourceLocation;
			target_addr_t		fRelativeAddress;
};


class UserBreakpointInstance
	: public DoublyLinkedListLinkImpl<UserBreakpointInstance> {
public:
								UserBreakpointInstance(
									UserBreakpoint* userBreakpoint,
									target_addr_t address);

			UserBreakpoint*		GetUserBreakpoint() const
									{ return fUserBreakpoint; }
			target_addr_t		Address() const	{ return fAddress; }

			Breakpoint*			GetBreakpoint() const	{ return fBreakpoint; }
			void				SetBreakpoint(Breakpoint* breakpoint);

private:
			target_addr_t		fAddress;
			UserBreakpoint*		fUserBreakpoint;
			Breakpoint*			fBreakpoint;
};


typedef DoublyLinkedList<UserBreakpointInstance> UserBreakpointInstanceList;


class UserBreakpoint : public BReferenceable,
	public DoublyLinkedListLinkImpl<UserBreakpoint> {
public:
								UserBreakpoint(
									const UserBreakpointLocation& location);
								~UserBreakpoint();

			const UserBreakpointLocation& Location() const { return fLocation; }

			int32_t				CountInstances() const;
			UserBreakpointInstance* InstanceAt(int32_t index) const;

			// Note: After known to the BreakpointManager, those can only be
			// invoked with the breakpoint manager lock held.
			bool				AddInstance(UserBreakpointInstance* instance);
			void				RemoveInstance(
									UserBreakpointInstance* instance);
			UserBreakpointInstance* RemoveInstanceAt(int32_t index);

			bool				IsValid() const		{ return fValid; }
			void				SetValid(bool valid);
									// BreakpointManager only

			bool				IsEnabled() const	{ return fEnabled; }
			void				SetEnabled(bool enabled);
									// BreakpointManager only

			bool				IsHidden() const	{ return fHidden; }
			void				SetHidden(bool hidden);

			bool				HasCondition() const
									{ return !fConditionExpression.IsEmpty(); }
			const BString&		Condition() const
									{ return fConditionExpression; }
			void				SetCondition(
									const BString& conditionExpression);

private:
			typedef BObjectList<UserBreakpointInstance> InstanceList;

private:
			UserBreakpointLocation fLocation;
			InstanceList		fInstances;
			bool				fValid;
			bool				fEnabled;
			bool				fHidden;
			BString				fConditionExpression;
};


typedef DoublyLinkedList<UserBreakpoint> UserBreakpointList;


#endif	// USER_BREAKPOINT_H
