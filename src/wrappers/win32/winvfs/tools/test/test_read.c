#include<stdio.h>
#include<Windows.h>

char buffer[1024*1024];
void main(int argc, char **argv)
{
	long count = 0, div = 10, size = 1024;
	DWORD bytesRead;
	SECURITY_ATTRIBUTES sa;
	HANDLE hfile;

	if (argc>1) div = atoi(argv[1]);
	if (argc>2) size = atoi(argv[2]);
	printf(". = %ld operations [reading %ld bytes]\n", div, size);
	sa.nLength=sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle=TRUE;
	sa.lpSecurityDescriptor=NULL;

	while(1) {
		hfile = CreateFile("file.54m",
			GENERIC_READ,
			FILE_SHARE_READ,
			&sa,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);     
		if (hfile == INVALID_HANDLE_VALUE) break;
		ReadFile(hfile, buffer, size , &bytesRead, NULL);
		if (bytesRead != size) printf("only read %d\n", bytesRead);
		CloseHandle(hfile);
		if (++count%div == 0) printf(".");
	}
	printf("\ncreate failed!\n");
}
