//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//---------------------------------------------------------------------
/*!
	\file sniffer/Range.h
	MIME sniffer range declarations
*/
#ifndef _SNIFFER_RANGE_H
#define _SNIFFER_RANGE_H

#include <Haiku.h>

namespace BPrivate {
namespace Storage {
namespace Sniffer {

class Err;

//! A range of byte offsets from which to check a pattern against a data stream.
class Range {
public:
	Range(int32_t start, int32_t end);
	
	status_t InitCheck() const;
	Err* GetErr() const;

	int32_t Start() const;
	int32_t End() const;
	
	void SetTo(int32_t start, int32_t end);
private:
	int32_t fStart;
	int32_t fEnd;
	status_t fCStatus;
};

};	// namespace Sniffer
};	// namespace Storage
};	// namespace BPrivate

#endif	// _SNIFFER_RANGE_H


