	/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef FLOAT_VALUE_H
#define FLOAT_VALUE_H


#include <Value.h>


class FloatValue : public Value {
public:
								FloatValue(const BVariant& value);
	virtual						~FloatValue();

			BVariant			GetValue() const
									{ return fValue; }

	virtual	bool				ToString(BString& _string) const;
	virtual	bool				ToVariant(BVariant& _value) const;

	virtual	bool				operator==(const Value& other) const;

private:
			BVariant			fValue;
};


#endif	// FLOAT_VALUE_H
