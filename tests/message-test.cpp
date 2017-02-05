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
 * File: message-test.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "./../app/Message.h>
#include "./../kernel/Debug.h>

int main(int argc, char **argv)
{
	ETK_OUTPUT("message-test in ETK(%u.%u.%u)...\n",
		etk_major_version, etk_minor_version, etk_micro_version);

	EMessage *msg = new EMessage('TMSG');

	ETK_OUTPUT("=============== SET DATA 0 ===================\n");

	msg->AddString("String", "I am a test");
	if(msg->HasString("String")) ETK_OUTPUT("String <= \"I am a test\"\n");
	
	msg->AddInt8("Int8", (eint8)-0x3A);
	if(msg->HasInt8("Int8")) ETK_OUTPUT("Int8 <= %hhx -- %hhi\n", (eint8)-0x3A, (eint8)-0x3A);
	
	msg->AddInt16("Int16", (eint16)-0x3AFA);
	if(msg->HasInt16("Int16")) ETK_OUTPUT("Int16 <= %hx -- %hi\n", (eint16)-0x3AFA, (eint16)-0x3AFA);

	msg->AddInt32("Int32", (eint32)-0x3AFAFAFA);
	if(msg->HasInt32("Int32")) ETK_OUTPUT("Int32 <= %x --- %i\n", (eint32)-0x3AFAFAFA, (eint32)-0x3AFAFAFA);
	
	msg->AddInt64("Int64", E_INT64_CONSTANT(-0x3AFAFAFAFAFAFAFA));
	if(msg->HasInt64("Int64")) ETK_OUTPUT("Int64 <= %llx -- %lli\n", E_INT64_CONSTANT(-0x3AFAFAFAFAFAFAFA), E_INT64_CONSTANT(-0x3AFAFAFAFAFAFAFA));
	
	msg->AddBool("Bool", true);
	if(msg->HasBool("Bool")) ETK_OUTPUT("Bool <= %d\n", true);
	
	msg->AddFloat("Float", 0.5);
	if(msg->HasFloat("Float")) ETK_OUTPUT("Float <= %g\n", 0.5);
	
//	msg->AddDouble("Double", M_PI);
//	if(msg->HasDouble("Double")) ETK_OUTPUT("Double <= %lg\n", M_PI);
	
	msg->AddPoint("Point", EPoint(12, 15));
	if(msg->HasPoint("Point")) ETK_OUTPUT("Point <= X: %g\tY: %g\n", 12., 15.);

	msg->AddRect("Rect", ERect(0, 0, 800, 600));
	if(msg->HasRect("Rect")) ETK_OUTPUT("Rect <= LEFT: %g\tTOP: %g\tRIGHT: %g\tBOTTOM: %g\n", 0., 0., 800., 600.);

	ETK_OUTPUT("==============================================\n");


	ETK_OUTPUT("\n\n\n");
	ETK_OUTPUT("=============== SET DATA 1 ===================\n");

	msg->AddString("String", "Test again");
	if(msg->HasString("String")) ETK_OUTPUT("String <= \"Test again\"\n");
	
	msg->AddInt8("Int8", (eint8)0xFA);
	if(msg->HasInt8("Int8")) ETK_OUTPUT("Int8 <= %hhx -- %hhi\n", (eint8)0xFA, (eint8)0xFA);
	
	msg->AddInt16("Int16", (eint16)0xFAFA);
	if(msg->HasInt16("Int16")) ETK_OUTPUT("Int16 <= %hx -- %hi\n", (eint16)0xFAFA, (eint16)0xFAFA);

	msg->AddInt32("Int32", (eint32)0xFAFAFAFA);
	if(msg->HasInt32("Int32")) ETK_OUTPUT("Int32 <= %x --- %i\n", (eint32)0xFAFAFAFA, (eint32)0xFAFAFAFA);
	
	msg->AddInt64("Int64", E_INT64_CONSTANT(0xFAFAFAFAFAFAFAFA));
	if(msg->HasInt64("Int64")) ETK_OUTPUT("Int64 <= %llx -- %lli\n", E_INT64_CONSTANT(0xFAFAFAFAFAFAFAFA), E_INT64_CONSTANT(0xFAFAFAFAFAFAFAFA));
	
	msg->AddBool("Bool", false);
	if(msg->HasBool("Bool")) ETK_OUTPUT("Bool <= %d\n", false);
	
	msg->AddFloat("Float", 0.125);
	if(msg->HasFloat("Float")) ETK_OUTPUT("Float <= %g\n", 0.125);
	
//	msg->AddDouble("Double", M_PI_4);
//	if(msg->HasDouble("Double")) ETK_OUTPUT("Double <= %lg\n", M_PI_4);
	
	msg->AddPoint("Point", EPoint(34, 176));
	if(msg->HasPoint("Point")) ETK_OUTPUT("Point <= X: %g\tY: %g\n", 34., 176.);

	msg->AddRect("Rect", ERect(231, 121, 355, 595));
	if(msg->HasRect("Rect")) ETK_OUTPUT("Rect <= LEFT: %g\tTOP: %g\tRIGHT: %g\tBOTTOM: %g\n", 231., 121., 355., 595.);
	
	ETK_OUTPUT("==============================================\n");


	const char *gstr = NULL;
	eint8 gi8;
	eint16 gi16;
	eint32 gi32;
	eint64 gi64;
	bool gbool;
	float gfl;
	double gdl;
	EPoint gpt;
	ERect grt;

	ETK_OUTPUT("\n\n\n");	
	ETK_OUTPUT("=============== GET DATA 0 ===================\n");

	if(msg->FindString("String", &gstr)) ETK_OUTPUT("String => \"%s\"\n", gstr);
	if(msg->FindInt8("Int8", &gi8)) ETK_OUTPUT("Int8 => %hhx --- %hhi\n", gi8, gi8);
	if(msg->FindInt16("Int16", &gi16)) ETK_OUTPUT("Int16 => %hx --- %hi\n", gi16, gi16);
	if(msg->FindInt32("Int32", &gi32)) ETK_OUTPUT("Int32 => %x --- %i\n", gi32, gi32);
	if(msg->FindInt64("Int64", &gi64)) ETK_OUTPUT("Int64 => %llx --- %lli\n", gi64, gi64);
	if(msg->FindBool("Bool", &gbool)) ETK_OUTPUT("Bool => %d\n", gbool);
	if(msg->FindFloat("Float", &gfl)) ETK_OUTPUT("Float => %g\n", gfl);
	if(msg->FindDouble("Double", &gdl)) ETK_OUTPUT("Double => %lg\n", gdl);

	if(msg->FindPoint("Point", &gpt)) ETK_OUTPUT("Point => X: %g\tY: %g\n", gpt.x, gpt.y);
	if(msg->FindRect("Rect", &grt)) ETK_OUTPUT("Rect => LEFT: %g\tTOP: %g\tRIGHT: %g\tBOTTOM: %g\n", grt.left, grt.top, grt.right, grt.bottom);

	ETK_OUTPUT("==============================================\n");

	
	ETK_OUTPUT("\n\n\n");	
	ETK_OUTPUT("=============== GET DATA 1 ===================\n");

	if(msg->FindString("String", 1, &gstr)) ETK_OUTPUT("String => \"%s\"\n", gstr);
	if(msg->FindInt8("Int8", 1, &gi8)) ETK_OUTPUT("Int8 => %hhx --- %hhi\n", gi8, gi8);
	if(msg->FindInt16("Int16", 1, &gi16)) ETK_OUTPUT("Int16 => %hx --- %hi\n", gi16, gi16);
	if(msg->FindInt32("Int32", 1, &gi32)) ETK_OUTPUT("Int32 => %x --- %i\n", gi32, gi32);
	if(msg->FindInt64("Int64", 1, &gi64)) ETK_OUTPUT("Int64 => %llx --- %lli\n", gi64, gi64);
	if(msg->FindBool("Bool", 1, &gbool)) ETK_OUTPUT("Bool => %d\n", gbool);
	if(msg->FindFloat("Float", 1, &gfl)) ETK_OUTPUT("Float => %g\n", gfl);
	if(msg->FindDouble("Double", 1, &gdl)) ETK_OUTPUT("Double => %lg\n", gdl);

	if(msg->FindPoint("Point", 1, &gpt)) ETK_OUTPUT("Point => X: %g\tY: %g\n", gpt.x, gpt.y);
	if(msg->FindRect("Rect", 1, &grt)) ETK_OUTPUT("Rect => LEFT: %g\tTOP: %g\tRIGHT: %g\tBOTTOM: %g\n", grt.left, grt.top, grt.right, grt.bottom);

	ETK_OUTPUT("==============================================\n");

	getchar();

	ETK_OUTPUT("\n\n\n");
	ETK_OUTPUT("=============== PRINT TO STREAM ===================\n");
	msg->PrintToStream();
	ETK_OUTPUT("===================================================\n");

	getchar();

	ETK_OUTPUT("\n\n\n");	
	ETK_OUTPUT("=============== FLATTEN ===================\n");
	size_t flattened_size = msg->FlattenedSize();
	ETK_OUTPUT("FlattenedSize = %lu\n", (unsigned long)flattened_size);
	char *flatten = (char*)malloc(flattened_size);
	if(flatten)
	{
		if(msg->Flatten(flatten, flattened_size))
		{
			msg->MakeEmpty();
			ETK_OUTPUT("Message have been flattened to the buffer and empty\n");
			if(msg->Unflatten(flatten, flattened_size))
			{
				ETK_OUTPUT("Message have been unflattened to the buffer\n");
				msg->PrintToStream();
			}
		}

		free(flatten);
	}
	ETK_OUTPUT("===========================================\n");

	getchar();

	ETK_OUTPUT("\n\n\n");	
	ETK_OUTPUT("=============== REPLACE DATA ===================\n");
	msg->ReplaceString("String", 1, "Test again, Test again, do you right?");
	msg->ReplaceInt8("Int8", 1, (eint8)0xEE);
	msg->ReplaceInt16("Int16", 1, (eint16)0xEEEE);
	msg->ReplaceInt32("Int32", 1, (eint32)0xEEEEEEEE);
	msg->ReplaceInt64("Int64", 1, E_INT64_CONSTANT(0xEEEEEEEEEEEEEEEE));
	msg->ReplaceBool("Bool", 1, false);
	msg->ReplaceFloat("Float", 1, 0.00005);
//	msg->ReplaceDouble("Double", 1, M_PI_2);

	msg->PrintToStream();
	ETK_OUTPUT("================================================\n");

	getchar();

	ETK_OUTPUT("\n\n\n");	
	ETK_OUTPUT("=============== REMOVE DATA ===================\n");
	msg->RemoveString("String");
	msg->RemoveInt8("Int8");
	msg->RemoveInt16("Int16", 1);
	msg->RemoveInt32("Int32", 1);
	msg->RemoveInt64("Int64");
	msg->RemoveBool("Bool", 1);
	msg->RemoveFloat("Float", 1);
	msg->RemoveDouble("Double", 1);

	msg->PrintToStream();
	ETK_OUTPUT("===============================================\n");

	getchar();

	ETK_OUTPUT("\n\n\n");
	ETK_OUTPUT("=============== aMsg = *msg ===================\n");
	EMessage aMsg = *msg;

	aMsg.PrintToStream();
	ETK_OUTPUT("===============================================\n");

	ETK_OUTPUT("\n\n\n");
	ETK_OUTPUT("=============== aMsg.MakeEmpty ===============\n");
	aMsg.MakeEmpty();

	aMsg.PrintToStream();
	ETK_OUTPUT("===============================================\n");

	ETK_OUTPUT("\n\n\n");
	ETK_OUTPUT("=============== aMsg.AddPointer ===============\n");
	aMsg.AddPointer("Pointer", msg);

	aMsg.PrintToStream();
	ETK_OUTPUT("===============================================\n");

	getchar();

	ETK_OUTPUT("\n\n\n");
	ETK_OUTPUT("=============== aMsg.FindPointer ===============\n");
	EMessage *kMsg = NULL;
	if(aMsg.FindPointer("Pointer", (void**)&kMsg) && kMsg)
	{
		ETK_OUTPUT("msg:address %p\n", kMsg);
		kMsg->PrintToStream();
	}
	ETK_OUTPUT("===============================================\n");

	getchar();

	ETK_OUTPUT("\n\n\n");	
	ETK_OUTPUT("=============== aMsg.Flatten ===============\n");
	flattened_size = aMsg.FlattenedSize();
	ETK_OUTPUT("FlattenedSize = %lu\n", (unsigned long)flattened_size);
	flatten = (char*)malloc(flattened_size);
	if(flatten)
	{
		if(aMsg.Flatten(flatten, flattened_size))
		{
			aMsg.what = 0;
			aMsg.MakeEmpty();
			ETK_OUTPUT("Message have been flattened to the buffer and empty\n");
			if(aMsg.Unflatten(flatten, flattened_size))
			{
				ETK_OUTPUT("Message have been unflattened to the buffer\n");
				aMsg.PrintToStream();

				EMessage *pMsg = NULL;
				if(aMsg.FindPointer("Pointer", (void**)&pMsg) && pMsg)
				{
					pMsg->PrintToStream();
				}
				else
				{
					ETK_OUTPUT("********FAILED**********\n");
				}
			}
		}

		free(flatten);
	}
	ETK_OUTPUT("===========================================\n");

	ETK_OUTPUT("\n\n\n");	
	ETK_OUTPUT("=============== msg->BGetInfo ===============\n");
	char *name;
	euint32 type;
	eint32 count;
	for(eint32 i = 0; msg->BGetInfo(E_ANY_TYPE, i, &name, &type, &count) == E_OK; i++)
	{
		ETK_OUTPUT("[%I32i]: Name: \"%s\", type: \'%c%c%c%c\', count: %I32i\n",
			   i, name,
#ifdef ETK_LITTLE_ENDIAN
			   (euint8)(type >> 24), (euint8)((type >> 16) & 0xff), (euint8)((type >> 8) & 0xff), (euint8)(type & 0xff),
#else
			   (euint8)(type & 0xff), (euint8)((type >> 8) & 0xff), (euint8)((type >> 16) & 0xff), (euint8)(type >> 24),
#endif
			   count);
	}
	ETK_OUTPUT("===========================================\n");

	delete msg;

	return 0;
}
