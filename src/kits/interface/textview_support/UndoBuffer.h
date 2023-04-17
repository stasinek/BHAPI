/*
 * Copyright 2003-2008, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stefano Ceccherini (burton666@libero.it)
 */

#ifndef __UNDOBUFFER_H
#define __UNDOBUFFER_H

#include <TextView.h>


class BClipboard;


// UndoBuffer
class BTextView::UndoBuffer {
public:
								UndoBuffer(BTextView* view, undo_state state);
	virtual						~UndoBuffer();

			void				Undo(BClipboard* clipboard);
			undo_state			State(bool* _isRedo) const;

protected:
	virtual	void				UndoSelf(BClipboard* clipboard);
	virtual	void				RedoSelf(BClipboard* clipboard);
	
			BTextView*			fTextView;
			int32_t				fStart;
			int32_t				fEnd;

			char*				fTextData;
			int32_t				fTextLength;
			text_run_array*		fRunArray;
			int32_t				fRunArrayLength;

			bool				fRedo;

private:
			undo_state			fState;
};


// CutUndoBuffer
class BTextView::CutUndoBuffer : public BTextView::UndoBuffer {
public:
								CutUndoBuffer(BTextView* textView);
	virtual						~CutUndoBuffer();

protected:
	virtual	void				RedoSelf(BClipboard* clipboard);
};


// PasteUndoBuffer
class BTextView::PasteUndoBuffer : public BTextView::UndoBuffer {
public:
								PasteUndoBuffer(BTextView* textView,
									const char* text, int32_t textLength,
									text_run_array* runArray,
									int32_t runArrayLen);
	virtual						~PasteUndoBuffer();

protected:
	virtual	void				UndoSelf(BClipboard* clipboard);
	virtual	void				RedoSelf(BClipboard* clipboard);

private:
			char*				fPasteText;
			int32_t				fPasteTextLength;
			text_run_array*		fPasteRunArray;
};


// ClearUndoBuffer
class BTextView::ClearUndoBuffer : public BTextView::UndoBuffer {
public:
								ClearUndoBuffer(BTextView* textView);
	virtual						~ClearUndoBuffer();

protected:
	virtual	void				RedoSelf(BClipboard* clipboard);
};


// DropUndoBuffer
class BTextView::DropUndoBuffer : public BTextView::UndoBuffer {
public:
								DropUndoBuffer(BTextView* textView,
									char const* text, int32_t textLength,
									text_run_array* runArray,
									int32_t runArrayLength, int32_t location,
									bool internalDrop);
	virtual						~DropUndoBuffer();

protected:
	virtual	void				UndoSelf(BClipboard* clipboard);
	virtual	void				RedoSelf(BClipboard* clipboard);

private:
			char*				fDropText;
			int32_t				fDropTextLength;
			text_run_array*		fDropRunArray;
	
			int32_t				fDropLocation;
			bool				fInternalDrop;
};


// TypingUndoBuffer
class BTextView::TypingUndoBuffer : public BTextView::UndoBuffer {
public:
								TypingUndoBuffer(BTextView* textView);
	virtual						~TypingUndoBuffer();

			void				InputCharacter(int32_t length);
			void				BackwardErase();
			void				ForwardErase();

protected:
	virtual	void				RedoSelf(BClipboard* clipboard);
	virtual	void				UndoSelf(BClipboard* clipboard);

private:
			void				_Reset();
	
			char*				fTypedText;
			int32_t				fTypedStart;
			int32_t				fTypedEnd;
			int32_t				fUndone;
};

#endif //__UNDOBUFFER_H
