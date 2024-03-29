/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Copyright 2013-2014, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */


#include <BreakpointSetting.h>

#include <Message.h>

#include <ArchivingUtils.h>
#include <FunctionID.h>
#include <LocatableFile.h>
#include <UserBreakpoint.h>


BreakpointSetting::BreakpointSetting()
	:
	fFunctionID(NULL),
	fSourceFile(),
	fSourceLocation(),
	fRelativeAddress(0),
	fEnabled(false),
	fHidden(false),
	fConditionExpression()
{
}


BreakpointSetting::BreakpointSetting(const BreakpointSetting& other)
	:
	fFunctionID(other.fFunctionID),
	fSourceFile(other.fSourceFile),
	fSourceLocation(other.fSourceLocation),
	fRelativeAddress(other.fRelativeAddress),
	fEnabled(other.fEnabled),
	fHidden(other.fHidden),
	fConditionExpression(other.fConditionExpression)
{
	if (fFunctionID != NULL)
		fFunctionID->AcquireReference();
}


BreakpointSetting::~BreakpointSetting()
{
	_Unset();
}


status_t BreakpointSetting::SetTo(const UserBreakpointLocation& location, bool enabled,
	bool hidden, const BString& conditionExpression)
{
	_Unset();

	fFunctionID = location.GetFunctionID();
	if (fFunctionID != NULL)
		fFunctionID->AcquireReference();

	if (LocatableFile* file = location.SourceFile())
		file->GetPath(fSourceFile);

	fSourceLocation = location.GetSourceLocation();
	fRelativeAddress = location.RelativeAddress();
	fEnabled = enabled;
	fHidden = hidden;
	fConditionExpression = conditionExpression;

	return B_OK;
}


status_t BreakpointSetting::SetTo(const BMessage& archive)
{
	_Unset();

	fFunctionID = ArchivingUtils::UnarchiveChild<FunctionID>(archive,
		"function");
	if (fFunctionID == NULL)
		return B_BAD_VALUE;

	archive.FindString("sourceFile", &fSourceFile);

	int32_t line;
	if (archive.FindInt32("line", &line) != B_OK)
		line = -1;

	int32_t column;
	if (archive.FindInt32("column", &column) != B_OK)
		column = -1;

	fSourceLocation = SourceLocation(line, column);

	if (archive.FindUInt64("relativeAddress", &fRelativeAddress) != B_OK)
		fRelativeAddress = 0;

	if (archive.FindBool("enabled", &fEnabled) != B_OK)
		fEnabled = false;

	if (archive.FindBool("hidden", &fHidden) != B_OK)
		fHidden = false;

	if (archive.FindString("condition", &fConditionExpression) != B_OK)
		fConditionExpression.Truncate(0);

	return B_OK;
}


status_t BreakpointSetting::WriteTo(BMessage& archive) const
{
	if (fFunctionID == NULL)
		return B_BAD_VALUE;

	archive.MakeEmpty();

	status_t error;
	if ((error = ArchivingUtils::ArchiveChild(fFunctionID, archive, "function"))
			!= B_OK
		|| (error = archive.AddString("sourceFile", fSourceFile)) != B_OK
		|| (error = archive.AddInt32("line", fSourceLocation.Line())) != B_OK
		|| (error = archive.AddInt32("column", fSourceLocation.Column()))
			!= B_OK
		|| (error = archive.AddUInt64("relativeAddress", fRelativeAddress))
			!= B_OK
		|| (error = archive.AddBool("enabled", fEnabled)) != B_OK
		|| (error = archive.AddBool("hidden", fHidden)) != B_OK
		|| (error = archive.AddString("condition", fConditionExpression))
			!= B_OK) {
		return error;
	}

	return B_OK;
}


BreakpointSetting&
BreakpointSetting::operator=(const BreakpointSetting& other)
{
	if (this == &other)
		return *this;

	_Unset();

	fFunctionID = other.fFunctionID;
	if (fFunctionID != NULL)
		fFunctionID->AcquireReference();

	fSourceFile = other.fSourceFile;
	fSourceLocation = other.fSourceLocation;
	fRelativeAddress = other.fRelativeAddress;
	fEnabled = other.fEnabled;
	fHidden = other.fHidden;
	fConditionExpression = other.fConditionExpression;

	return *this;
}


void BreakpointSetting::_Unset()
{
	if (fFunctionID != NULL) {
		fFunctionID->ReleaseReference();
		fFunctionID = NULL;
	}

	fSourceFile.Truncate(0);
	fSourceLocation = SourceLocation();
	fRelativeAddress = 0;
	fEnabled = false;
	fConditionExpression.Truncate(0);
}
