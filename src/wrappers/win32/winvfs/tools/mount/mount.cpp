// mount.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "windows.h"
#include <winioctl.h>

int Mount(char *DevName, char *driver);
int Unmount(char *driver);


int main(int argc, char* argv[])
{
	bool bMount = false;
	bool bUnmount = false;
	int  nDisk = 0;
	int  nPart = 0;
	char DevName[256];

	printf( "\nMount/Umount tools by Matt Wu <mattwu@163.com>.\n"
			"                      http://sysinternals.yeah.net.\n\n"
		"Usage: mount/unmount disk_number partition_number driver_letter\n"
		"Ex:    To mount partition 1 of harddisk 0 to f:, then\n"
		"       mount 0 1 f:\n"
		"Ex:    To unmount f:, then\n"
		"       unmount f: or mount /umount f:\n\n" );

	if (_stricmp(argv[0], "mount") == 0)
	{
		if (argc == 4)
		{

			nDisk = atoi(argv[1]);
			nPart = atoi(argv[2]);

			if (nDisk < 0)
			{
				printf("%s: disk number (%d) is error.\n", argv[0], nDisk);
				return -1;
			}

			if (nPart >= 16 || nDisk < 0)
			{
				printf("%s: partition number (%d) is error.\n", argv[0], nPart);
				return -1;
			}

			if (strlen(argv[3]) !=2 || argv[3][1] != ':')
			{
				printf("%s: driver letter format (%s) is error.\n", argv[0], argv[3]);
				return -1;
			}

			sprintf(DevName,"\\Device\\Harddisk%d\\Partition%d", nDisk, nPart);

			printf("%s: now mounting %s ...  ", argv[0], DevName);
			if(Mount(DevName, argv[3]) == 0)
			{
				printf("Succeed.\n");
			}
			else
			{
				printf("Falied.\n");
			}
		}

        if (argc == 3)
        {
            if (_stricmp(argv[1], "/umount") == 0 || _stricmp(argv[1], "/unmount") == 0)
            {
			    if (strlen(argv[2]) !=2 || argv[2][1] != ':')
			    {
				    printf("%s: driver letter format (%s) is error.\n", argv[0], argv[2]);
				    return -1;
			    }

			    printf("%s: now unmounting %s ... \n", argv[0], argv[2]);
			    if(Unmount(argv[2]) == 0)
			    {
				    printf("         Succeed.\n");
			    }
			    else
			    {
				    printf("         Falied.\n");
			    }

            }
        }

		return 0;
	}


	if (_stricmp(argv[0], "unmount") == 0 || _stricmp(argv[0], "umount") == 0)
	{
		if (argc == 2)
		{

			if (strlen(argv[1]) !=2 || argv[1][1] != ':')
			{
				printf("%s: driver letter format (%s) is error.\n", argv[0], argv[1]);
				return -1;
			}

			printf("%s: now unmounting %s ... \n", argv[0], argv[1]);
			if(Unmount(argv[1]) == 0)
			{
				printf("         Succeed.\n");
			}
			else
			{
				printf("         Falied.\n");
			}

		}

		return 0;

	}

	return -1;
}

int Mount(char *DevName, char *driver)
{

	if (!DefineDosDevice(
		DDD_RAW_TARGET_PATH,
		driver,
		DevName))
	{
		return -1;
	}

	return 0;
}

int Unmount(char *driver)
{

	char	DosDevName[256];
	HANDLE	device;
	ULONG	dwBytes;

	sprintf(DosDevName, "\\\\.\\%s", driver);

	printf("umount: DosDevName: %s\n", DosDevName);

	device = CreateFile(
		DosDevName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL );

	if (device == INVALID_HANDLE_VALUE)
	{
		printf("Unmount: open %s error.\n", DosDevName);
		return -1;
	}

	if (!DeviceIoControl(
		device,
		FSCTL_LOCK_VOLUME,
		NULL,
		0,
		NULL,
		0,
		&dwBytes,
		NULL ))
	{
		printf("Unmount: ioctl: LockVolume %s error.\n", DosDevName);
		
//		CloseHandle(device);
//		return -1;
	}

	if (!DeviceIoControl(
		device,
		FSCTL_DISMOUNT_VOLUME,
		NULL,
		0,
		NULL,
		0,
		&dwBytes,
		NULL))
	{
		printf("Unmount: ioctl: DisMount %s error.\n", DosDevName);
		CloseHandle(device);
		return -1;
	}

	CloseHandle(device);

	if (!DefineDosDevice(
		DDD_REMOVE_DEFINITION,
		driver,
		NULL ))
	{
		printf("Unmount: Remove %s error.\n", driver);
		return -1;
	}

	return 0;
}
