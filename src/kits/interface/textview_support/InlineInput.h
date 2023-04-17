/*
 * Copyright 2003-2006, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stefano Ceccherini (burton666@libero.it)
 */

#ifndef __INLINEINPUT_H
#define __INLINEINPUT_H

#include <Messenger.h>
#include <TextView.h>

struct clause;

class BTextView::InlineInput {
public:
	InlineInput(BMessenger);
	~InlineInput();
	
	const BMessenger *Method() const;
	
	bool IsActive() const;
	void SetActive(bool active);
	
	int32_t Length() const;
	void SetLength(int32_t length);
	
	int32_t Offset() const;
	void SetOffset(int32_t offset);
		
	int32_t SelectionLength() const;
	void SetSelectionLength(int32_t);
	
	int32_t SelectionOffset() const;
	void SetSelectionOffset(int32_t offset);
	
	bool AddClause(int32_t, int32_t);
	bool GetClause(int32_t index, int32_t *start, int32_t *end) const;
	int32_t CountClauses() const;
	
	void ResetClauses();
	
private:
	const BMessenger fMessenger;

	bool fActive;
		
	int32_t fOffset;
	int32_t fLength;
	
	int32_t fSelectionOffset;
	int32_t fSelectionLength;
	
	int32_t fNumClauses;
	clause *fClauses;
};

#endif //__INLINEINPUT_H
