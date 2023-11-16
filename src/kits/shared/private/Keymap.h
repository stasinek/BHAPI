/*
 * Copyright 2004-2010, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Jérôme Duval
 *		Axel Dörfler, axeld@pinc-software.de.
 */
#ifndef _KEYMAP_H
#define _KEYMAP_H


#include <DataIO.h>
#include <InterfaceDefs.h>
#include <ObjectList.h>


class BKeymap {
public:
								BKeymap();
	virtual						~BKeymap();

			status_t			SetTo(const char* path);
			status_t			SetTo(BDataIO& stream);
			status_t			SetToCurrent();
			status_t			SetToDefault();
			void				Unset();

			bool				IsModifierKey(uint32_t keyCode) const;
			uint32_t				Modifier(uint32_t keyCode) const;
			uint32_t				KeyForModifier(uint32_t modifier) const;
			uint8				ActiveDeadKey(uint32_t keyCode,
									uint32_t modifiers) const;
			uint8				DeadKey(uint32_t keyCode, uint32_t modifiers,
									bool* isEnabled = NULL) const;
			bool				IsDeadSecondKey(uint32_t keyCode,
									uint32_t modifiers,
									uint8 activeDeadKey) const;
			void				GetChars(uint32_t keyCode, uint32_t modifiers,
									uint8 activeDeadKey, char** chars,
									int32_t* numBytes) const;
			status_t			GetModifiedCharacters(const char* in,
									int32_t inModifiers, int32_t outModifiers,
									BObjectList<const char>* _outList);

			const key_map&		Map() const { return fKeys; }

			bool				operator==(const BKeymap& other) const;
			bool				operator!=(const BKeymap& other) const;

			BKeymap&			operator=(const BKeymap& other);

protected:
			int32_t				Offset(uint32_t keyCode, uint32_t modifiers,
									uint32_t* _table = NULL) const;
			uint8				DeadKeyIndex(int32_t offset) const;

protected:
			char*				fChars;
			key_map				fKeys;
			uint32_t				fCharsSize;
};


#endif	// KEYMAP_H
