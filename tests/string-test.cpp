/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * ETK++ library is a freeware; it may be used and distributed according to
 * the terms of The MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File: string-test.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "./../support/StringClass.h>
#include "./../kernel/OS.h>
#include "./../kernel/Debug.h>

int main()
{
	EString source, destination;

	ETK_OUTPUT("string-test in ETK(%u.%u.%u)...\n",
		etk_major_version, etk_minor_version, etk_micro_version);

	source.AppendFormat("%s", "test");
	ETK_OUTPUT("SOURCE:(AppendFormat(\"%%s\", \"test\"): %s\n", source.String());
	assert(source == "test");

	source.SetTo("abcdefg");
	ETK_OUTPUT("SOURCE(SetTo(\"abcdefg\")): %s\n", source.String());

	source.MoveInto(destination, 2, 3);
	ETK_OUTPUT("SOURCE:(MoveInto(&destination, 2, 3)): %s\n", source.String());
	ETK_OUTPUT("DESTINATION: %s\n", destination.String());

	assert(source == "abfg");
	assert(destination == EString("cde"));
	
	ETK_OUTPUT("SOURCE:(ToUpper()): %s\n", source.ToUpper().String());
	ETK_OUTPUT("SOURCE:(ToLower()): %s\n", source.ToLower().String());
	
	source += "  Looking for a result";
	assert(source == "abfg  Looking for a result");
	ETK_OUTPUT("SOURCE + \" Looking for a result\" = %s\n", source.String());
	
	EString test;

	test << "The result is: " << -12.05 << " seconds";
	ETK_OUTPUT("TEST << \"The result is: \" << -12.05 << \" seconds\" = %s\n", test.String());
	assert(test == "The result is: -12.05 seconds");

	EString a("a"), b("b"), c("c");
	a << b << c;

	assert(a == "abc");
	assert(b == "b");
	assert(c == "c");

	ETK_OUTPUT("a = \"a\"\tb = \"b\"\tc = \"c\"\n");
	ETK_OUTPUT("a << b << c: a = %s\n", a.String());
	ETK_OUTPUT("a << b << c: b = %s\n", b.String());
	ETK_OUTPUT("a << b << c: c = %s\n", c.String());

	EString astring("AbcAbcAbc");
	assert(astring.FindLast("Abc", 7) == 3);
	ETK_OUTPUT("astring(\"AbcAbcAbc\").FindLast(\"Abc\", 7) = %ld\n", astring.FindLast("Abc", 7));

	astring.SetTo("I am a man. You are a girl!");
	assert(astring.Capitalize() == "I am a man. you are a girl!");
	ETK_OUTPUT("Capitalize(\"I am a man. You are a girl!\") = %s\n", astring.String());

	astring.SetTo("I am a man. You are a girl!");
	assert(astring.CapitalizeEachWord() == "I Am A Man. You Are A Girl!");
	ETK_OUTPUT("CapitalizeEachWord(\"I am a man. You are a girl!\") = %s\n", astring.String());

	astring.SetTo("I am ready to \\t \\n study C++");
	assert(astring.CharacterEscape("\\", '\\') == "I am ready to \\\\t \\\\n study C++");
	ETK_OUTPUT("CharacterEscape(\"I am ready to \\t \\n study C++\", \"\\\", '\\') = %s\n", astring.String());
	assert(astring.CharacterDeescape('\\') == "I am ready to \\t \\n study C++");
	ETK_OUTPUT("Then CharacterDeescape('\\') = %s\n", astring.String());

	ETK_OUTPUT("IsNumber(\"+0.12\") = %s\n", EString("+0.12").IsNumber() ? "true" : "false");
	ETK_OUTPUT("IsNumber(\"-0.12\") = %s\n", EString("-0.12").IsNumber() ? "true" : "false");
	ETK_OUTPUT("IsNumber(\"+.12\") = %s\n", EString("+.12").IsNumber() ? "true" : "false");
	ETK_OUTPUT("IsNumber(\"-.12\") = %s\n", EString("-.12").IsNumber() ? "true" : "false");
	ETK_OUTPUT("IsNumber(\".12\") = %s\n", EString(".12").IsNumber() ? "true" : "false");

	ETK_OUTPUT("IsNumber(\"+1\") = %s\n", EString("+1").IsNumber() ? "true" : "false");
	ETK_OUTPUT("IsNumber(\"-1\") = %s\n", EString("-1").IsNumber() ? "true" : "false");
	ETK_OUTPUT("IsNumber(\"1\") = %s\n", EString("1").IsNumber() ? "true" : "false");
	ETK_OUTPUT("IsNumber(\"-0.abcd12\") = %s\n", EString("-0.abcd12").IsNumber() ? "true" : "false");
	ETK_OUTPUT("IsNumber(\"0x01234abC\") = %s\n", EString("0x01234abC").IsNumber() ? "true" : "false");
	ETK_OUTPUT("IsNumber(\"0xak47\") = %s\n", EString("0xak47").IsNumber() ? "true" : "false");

	ETK_OUTPUT("IsInteger(\"+1.000\") = %s\n", EString("+1.000").IsInteger() ? "true" : "false");
	ETK_OUTPUT("IsInteger(\"-1.000\") = %s\n", EString("-1.000").IsInteger() ? "true" : "false");
	ETK_OUTPUT("IsInteger(\"+1\") = %s\n", EString("+1").IsInteger() ? "true" : "false");
	ETK_OUTPUT("IsInteger(\"-1\") = %s\n", EString("-1").IsInteger() ? "true" : "false");
	ETK_OUTPUT("IsInteger(\"+.000\") = %s\n", EString("+.000").IsInteger() ? "true" : "false");
	ETK_OUTPUT("IsInteger(\"-.000\") = %s\n", EString("-.000").IsInteger() ? "true" : "false");
	ETK_OUTPUT("IsInteger(\"1.3200\") = %s\n", EString("1.3200").IsInteger() ? "true" : "false");

	double test_value = 0;

	assert(EString("-.0004235").GetDecimal(&test_value) == true);
	ETK_OUTPUT("EString(\"-.0004235\").GetDecimal = %g\n", test_value);

	assert(EString("+400.23567444").GetDecimal(&test_value) == true);
	ETK_OUTPUT("EString(\"+400.23567444\").GetDecimal = %f\n", test_value);

	astring.MakeEmpty();
	astring << "E_MAXINT8:" << E_MAXINT8 << "\n";
	astring << "E_MININT8:" << E_MININT8 << "\n";
	ETK_OUTPUT("%s", astring.String());

	astring.MakeEmpty();
	astring << "E_MAXINT16:" << E_MAXINT16 << "\n";
	astring << "E_MININT16:" << E_MININT16 << "\n";
	ETK_OUTPUT("%s", astring.String());

	astring.MakeEmpty();
	astring << "E_MAXINT32:" << E_MAXINT32 << "\n";
	astring << "E_MININT32:" << E_MININT32 << "\n";
	ETK_OUTPUT("%s", astring.String());

	astring.MakeEmpty();
	astring << "E_MAXINT64:" << E_MAXINT64 << "\n";
	astring << "E_MININT64:" << E_MININT64 << "\n";
	ETK_OUTPUT("%s", astring.String());

	astring.MakeEmpty();
	astring << "microseconds eslaped since system start:" << e_system_time();
	ETK_OUTPUT("%s\n", astring.String());	

	astring = "ASCII UTF8 Test";
	ETK_OUTPUT("length of \"%s\" is %d(UTF-8)\n", astring.String(), astring.CountChars());
	euint8 suLen = 0;
	const char *suStr = astring.CharAt(4, &suLen);
	if(suStr == NULL || suLen == 0) ETK_OUTPUT("the fourth char in \"%s\" is EMPTY\n", astring.String());
	else
	{
		EString anstring;
		anstring.SetTo(suStr, (eint32_t)suLen);
		ETK_OUTPUT("the fourth char in \"%s\" is %s\n", astring.String(), anstring.String());
	}

	astring = "这是一个测试UTF8的例子";
	ETK_OUTPUT("length of \"%s\" is %d(UTF-8)\n", astring.String(), astring.CountChars());
	suStr = astring.CharAt(4, &suLen);
	if(suStr == NULL || suLen == 0) ETK_OUTPUT("the fourth char in \"%s\" is EMPTY\n", astring.String());
	else
	{
		EString anstring;
		anstring.SetTo(suStr, (eint32_t)suLen);
		ETK_OUTPUT("the fourth char in \"%s\" is %s\n", astring.String(), anstring.String());
	}

	astring = "测试 UTF-8 <-> UNICODE 的句子";
	eunichar* unicode = e_utf8_convert_to_unicode(astring.String(), -1);
	char* utf8 = (unicode ? e_unicode_convert_to_utf8(unicode, 22) : NULL);
	ETK_OUTPUT("Convert \"%s\"(UTF-8) from 1 to 22 is \"%s\"(UNICODE).\n", astring.String(), utf8 ? utf8 : "EMPTY");
	if(unicode) free(unicode);
	if(utf8) free(utf8);

	return 0;
}
