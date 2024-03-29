/*
 * Copyright 2011, Haiku, Inc.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PACKAGE__PACKAGE_RESOLVABLE_OPERATOR_H_
#define _PACKAGE__PACKAGE_RESOLVABLE_OPERATOR_H_


namespace BPackageKit {


enum BPackageResolvableOperator {
	B_PACKAGE_RESOLVABLE_OP_LESS			= 0,
	B_PACKAGE_RESOLVABLE_OP_LESS_EQUAL		= 1,
	B_PACKAGE_RESOLVABLE_OP_EQUAL			= 2,
	B_PACKAGE_RESOLVABLE_OP_NOT_EQUAL		= 3,
	B_PACKAGE_RESOLVABLE_OP_GREATER_EQUAL	= 4,
	B_PACKAGE_RESOLVABLE_OP_GREATER			= 5,
	//
	B_PACKAGE_RESOLVABLE_OP_ENUM_COUNT
};


}	// namespace BPackageKit


#endif	// _PACKAGE__PACKAGE_RESOLVABLE_OPERATOR_H_
