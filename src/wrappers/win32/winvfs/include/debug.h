#ifndef _DEBUG_H
#define _DEBUG_H

#include <ntstrsafe.h>

void __inline__ dump_buffer(const char *buffer, int size, const char* label)
{
	char buf[128];
	char *tmp;
	int i,j;

	DbgPrint("******** %s", label);

	for (j=0; j<size/16; j++) {
		tmp = buf;
		for (i=0; i<16; i++) {
			RtlStringCbPrintfA(tmp, 4, "%02x ", (unsigned char)buffer[i+j*16]);
			tmp += 3;
		}

		for (i=0; i<16; i++) {
			char c=buffer[i+j*16];
			if (c >= ' ' && c <= '~')
				RtlStringCbPrintfA(tmp, 2, "%c", c);
			else
				RtlStringCbPrintfA(tmp, 2, " ");
			tmp += 1;
		}

		DbgPrint("%s", buf);
	}
}

#endif