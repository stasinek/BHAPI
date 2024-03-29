//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//---------------------------------------------------------------------
/*!
	\file Range.cpp
	MIME sniffer range implementation
*/

#include <sniffer/Err.h>
#include <sniffer/Range.h>
#include <sniffer/Parser.h>


using namespace BPrivate::Storage::Sniffer;

Range::Range(int32_t start, int32_t end)
	: fStart(-1)
	, fEnd(-1)
	, fCStatus(B_NO_INIT)
{
	SetTo(start, end);
}

status_t Range::InitCheck() const {
	return fCStatus;
}

Err*
Range::GetErr() const {
	if (fCStatus == B_OK)
		return NULL;
	else {
		char start_str[32];
		char end_str[32];
		sprintf(start_str, "%" B_PRId32, fStart);
		sprintf(end_str, "%" B_PRId32, fEnd);
		return new Err(std::string("Sniffer Parser Error -- Invalid range: [") + start_str + ":" + end_str + "]", -1);
	}
}

int32_t Range::Start() const {
	return fStart;
}

int32_t Range::End() const {
	return fEnd;
}
	
void Range::SetTo(int32_t start, int32_t end) {
		fStart = start;
		fEnd = end;
	if (start > end) {
		fCStatus = B_BAD_VALUE;
	} else {
		fCStatus = B_OK;
	}
}



