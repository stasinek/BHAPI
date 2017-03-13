#include<stdio.h>
#include<Windows.h>

void main(int argc, char **argv)
{
	long count = 0, div = 100;
	SECURITY_ATTRIBUTES sa;
	HANDLE hfile;

	if (argc>1) div = atoi(argv[1]);
	printf(". = %ld operations\n", div);
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
		if (hfile == INVALID_HANDLE_VALUE)
			printf("\ncreate failed!\n");
		else CloseHandle(hfile);
		if (++count%div == 0) printf(".");
	}
}