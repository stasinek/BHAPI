/*
 * Copyright 2011, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef TYPE_LOOKUP_CONSTRAINTS_H
#define TYPE_LOOKUP_CONSTRAINTS_H


#include <kits/support/String.h>

#include <Type.h>


class TypeLookupConstraints {
public:
								TypeLookupConstraints();
									// no constraints
								TypeLookupConstraints(type_kind typeKind);
									// constrain on type only
								TypeLookupConstraints(type_kind typeKind,
									int32_t subtypeKind);

		bool					HasTypeKind() const;
		bool					HasSubtypeKind() const;
		bool					HasBaseTypeName() const;
		type_kind				TypeKind() const;
		int32_t					SubtypeKind() const;

		// TODO: This should be further generalized to being able to
		// pass a full set of constraints for the base type, not just the name
		const BString&			BaseTypeName() const;

		void					SetTypeKind(type_kind typeKind);
		void					SetSubtypeKind(int32_t subtypeKind);
		void					SetBaseTypeName(const BString& name);

private:
		type_kind				fTypeKind;
		int32_t					fSubtypeKind;
		bool					fTypeKindGiven;
		bool					fSubtypeKindGiven;
		BString					fBaseTypeName;
};

#endif // TYPE_LOOKUP_CONSTRAINTS_H
