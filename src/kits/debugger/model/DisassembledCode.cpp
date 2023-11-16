/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include <DisassembledCode.h>

#include <stdlib.h>
#include <kits/support/String.h>

#include <new>

#include <kits/support/String.h>

#include <SourceLanguage.h>
#include <Statement.h>


struct DisassembledCode::Line {
	BString					line;
	ContiguousStatement*	statement;

	Line(const BString& line, ContiguousStatement* statement)
		:
		line(line),
		statement(statement)
	{
	}
};


DisassembledCode::DisassembledCode(SourceLanguage* language)
	:
	fLanguage(language),
	fLines(20, true)
{
	fLanguage->AcquireReference();
}


DisassembledCode::~DisassembledCode()
{
	for (int32_t i = 0; Statement* statement = fStatements.ItemAt(i); i++)
		statement->ReleaseReference();

	fLanguage->ReleaseReference();
}


bool DisassembledCode::Lock()
{
	// We're immutable, so no locking required.
	return true;
}


void DisassembledCode::Unlock()
{
}


SourceLanguage*
DisassembledCode::GetSourceLanguage() const
{
	return fLanguage;
}


int32_t DisassembledCode::CountLines() const
{
	return fLines.CountItems();
}


const char*  DisassembledCode::LineAt(int32_t index) const
{
	Line* line = fLines.ItemAt(index);
	return line != NULL ? line->line.String() : NULL;
}


int32_t DisassembledCode::LineLengthAt(int32_t index) const
{
	Line* line = fLines.ItemAt(index);
	return line != NULL ? line->line.Length() : 0;
}


bool DisassembledCode::GetStatementLocationRange(const SourceLocation& location,
	SourceLocation& _start, SourceLocation& _end) const
{
	Line* line = fLines.ItemAt(location.Line());
	if (line == NULL || line->statement == NULL)
		return false;

	_start = line->statement->StartSourceLocation();
	_end = SourceLocation(_start.Line() + 1);
		// TODO: Multi-line instructions!
	return true;
}


LocatableFile*
DisassembledCode::GetSourceFile() const
{
	return NULL;
}


Statement*
DisassembledCode::StatementAtLocation(const SourceLocation& location) const
{
	Line* line = fLines.ItemAt(location.Line());
	return line != NULL ? line->statement : NULL;
}


Statement*
DisassembledCode::StatementAtAddress(target_addr_t address) const
{
	return fStatements.BinarySearchByKey(address, &_CompareAddressStatement);
}


TargetAddressRange
DisassembledCode::StatementAddressRange() const
{
	if (fStatements.IsEmpty())
		return TargetAddressRange();

	ContiguousStatement* first = fStatements.ItemAt(0);
	ContiguousStatement* last
		= fStatements.ItemAt(fStatements.CountItems() - 1);
	return TargetAddressRange(first->AddressRange().Start(),
		last->AddressRange().End());
}


bool DisassembledCode::AddCommentLine(const BString& line)
{
	return _AddLine(line, NULL);
}


bool DisassembledCode::AddInstructionLine(const BString& line, target_addr_t address,
	target_size_t size)
{
	int32_t lineIndex = fLines.CountItems();

	ContiguousStatement* statement = new(std::nothrow) ContiguousStatement(
		SourceLocation(lineIndex), TargetAddressRange(address, size));
	if (statement == NULL)
		return false;

	if (!fStatements.AddItem(statement)) {
		delete statement;
		return false;
	}

	if (!_AddLine(line, statement))
		return false;

	return true;
}


bool DisassembledCode::_AddLine(const BString& _line, ContiguousStatement* statement)
{
	Line* line = new(std::nothrow) Line(_line, statement);
	if (line == NULL)
		return false;

	if (!fLines.AddItem(line)) {
		delete line;
		return false;
	}

	return true;
}


/*static*/ int
DisassembledCode::_CompareAddressStatement(const target_addr_t* address,
	const ContiguousStatement* statement)
{
	const TargetAddressRange& range = statement->AddressRange();

	if (*address < range.Start())
		return -1;
	return *address < range.End() ? 0 : 1;
}

