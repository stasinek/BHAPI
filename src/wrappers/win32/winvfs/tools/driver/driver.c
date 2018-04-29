#include <stdio.h>
#include <string.h>
#include <windows.h>

void print_error() 
{
	char *error;
	
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&error, 0, NULL);
	printf("%s\n", error);
}

#define RKEY_PREFIX "System\\CurrentControlSet\\Services\\EventLog\\System\\"
#define RKEY_EVENTMESSAGEFILE "EventMessageFile"
#define RKEY_TYPESUPPORTED "TypesSupported"

int load(int argc, char **argv, int driver_type)
{
	HKEY hkey;
	unsigned int len;
	int error=0, dummy;
	char *driver_path=NULL;
	SC_HANDLE scman, scserv;
	char *driver_name, *b, *e, *rkey;


	if (argc != 3 || strcmp(argv[2], "?") == 0) {
		printf("syntax: driver [ load | load_fs ] driver.sys\n");
		return -1;
	}

	/*find out the full qualified driver path */
	do {
		if (driver_path)
			free(driver_path);
		driver_path=malloc((len=GetFullPathName(argv[2], 0, driver_path, NULL)));
	} while (GetFullPathName(argv[2], len, driver_path, &b) > len);
	
	/* find out the driver name */
	if ((e=strrchr(b, '.')) == NULL)
		e=b+strlen(b);
	else
		e--;
	driver_name=malloc(e-b+2);
	memcpy(driver_name, b, e-b+1); driver_name[e-b+1]=0;


	/* create and start the service associated with the driver */
	if (!(scman=OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
		printf("can't open scman: "); 
		print_error();
		error=-1; goto out;
	}


	if (!(scserv=CreateService(scman, driver_name, driver_name, SC_MANAGER_ALL_ACCESS, 
		driver_type, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, 
		driver_path, NULL, NULL, NULL, NULL, NULL))) {
		printf("can't create service %s to load driver %s: ", driver_name, driver_path); 
		print_error();
		error=-1; goto out;
	}

	
	if (!(StartService(scserv, 0, NULL))) {
		printf("can't start service: "); 
		print_error();
		if (!DeleteService(scserv)) {
			printf("can't delete service '%s': ", driver_name); 
			print_error();
		}
		error=-1; goto out;
	}


	/* create registry keys for event log messages */
	rkey=malloc(strlen(RKEY_PREFIX)+strlen(driver_name)+1);
	sprintf(rkey, "%s%s", RKEY_PREFIX, driver_name);

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, rkey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
			NULL, &hkey, &dummy);
	RegSetValueEx(hkey, RKEY_EVENTMESSAGEFILE, 0, REG_EXPAND_SZ, driver_path, strlen(driver_path));
	dummy=7;
	RegSetValueEx(hkey, RKEY_TYPESUPPORTED, 0, REG_DWORD, (char*)&dummy, sizeof(int));
	free(rkey);
	RegFlushKey(hkey);
	RegCloseKey(hkey);

out:
	if (scserv)
		CloseServiceHandle(scserv);
	if (scman)
		CloseServiceHandle(scman);
	return error;
}


int unload(int argc, char **argv)
{
	int error=0;
	const char *driver_name;
	char *rkey;
	SC_HANDLE scman, scserv;
	SERVICE_STATUS dummy;

	if (argc != 3 || strcmp(argv[2], "?") == 0) {
		printf("syntax: driver unload driver_name\n");
		return -1;
	}
	driver_name=argv[2];
	
	if (!(scman=OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
		printf("can't open scman: "); print_error();
		error=-1; goto out;
	}

	if (!(scserv=OpenService(scman, driver_name, SC_MANAGER_ALL_ACCESS))) {
		printf("can't open service %s: ", driver_name); 
		print_error();
		error=-1; goto out;
	}


	if (!(ControlService(scserv, SERVICE_CONTROL_STOP, &dummy))) {
		printf("can't stop service %s: ", driver_name);
		print_error();
	}

	if (!(DeleteService(scserv))) {
		printf("can't unload driver %s: ", driver_name); 
		print_error();
		error=-1; goto out;
	}

out:
	/* delete registry key used to keep messages */
	rkey=malloc(strlen(RKEY_PREFIX)+strlen(driver_name)+1);
	sprintf(rkey, "%s%s", RKEY_PREFIX, driver_name);
	RegDeleteKey(HKEY_LOCAL_MACHINE, rkey);	
	free(rkey);

	

	if (scserv)
		CloseServiceHandle(scserv);
	if (scman)
		CloseServiceHandle(scman);

	return error;
}


int list(int argc, char **argv)
{
	
	SC_HANDLE scman;
	char *type, *status, *driver=NULL;
	int i, n, error, ok, buflen;
	ENUM_SERVICE_STATUS *services=NULL;

	if (argc == 3) 
		if (strcmp(argv[2], "?") == 0) {
			printf("syntax: driver list [driver_name]\n");
			return -1;
		} else 
			driver=argv[2];

	
	if (!(scman=OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
		printf("can't open scman: "); print_error();
		error=-1; goto out;
	}

	do {
		if (services)
			free(services);
		EnumServicesStatus(scman, SERVICE_DRIVER, SERVICE_STATE_ALL, NULL, 0, &buflen, &n, 0);
		if (!(services=malloc(buflen))) {
			ok=0; 
			break;
		}
	} while (!(ok=EnumServicesStatus(scman, SERVICE_DRIVER, SERVICE_STATE_ALL, services, buflen, &buflen, &n, 0)) && GetLastError() == ERROR_MORE_DATA);

	if (!ok) {
		printf("can't list drivers: "); print_error();
		return -1;
	}
	


	printf("%-40s %-10s %-10s\n", "name", "type", "status");

	for(i=0; i<n; i++) {

		switch (services[i].ServiceStatus.dwServiceType) {
		case SERVICE_FILE_SYSTEM_DRIVER:
			type="fs_driver";
			break;
		case SERVICE_KERNEL_DRIVER:
			type="driver";
			break;
		default:
			type="unknown";
		}

		switch (services[i].ServiceStatus.dwCurrentState) {
		case SERVICE_CONTINUE_PENDING:
			status="continue_pending";
			break;
		case SERVICE_PAUSE_PENDING:
			status="pause_pending";
			break;
		case SERVICE_PAUSED:
			status="paused";
			break;
		case SERVICE_RUNNING:
			status="running";
			break;
		case SERVICE_START_PENDING:
			status="start_pending";
			break;
		case SERVICE_STOP_PENDING:
			status="stop_pending";
			break;
		case SERVICE_STOPPED:
			status="stopped";
			break;
		default:
			status="unknown";
		}

		if (!driver)
			printf("%-40s %-10s %-10s\n", services[i].lpServiceName, type, status);
		else 
			if (strcmp(driver, services[i].lpServiceName) == 0) {
				printf("%-40s %-10s %-10s\n", services[i].lpServiceName, type, status);
				printf("Display name: %s", services[i].lpDisplayName);
			}
	}


out:
	if (scman)
		CloseServiceHandle(scman);

	return error;

}

int mesg(int argc, char **argv)
{
	unsigned int len=4096, needed, readed, dummy;
	int error=0;
	char buffer[4096], *source, *driver_name, *driver_path=NULL, *e, *b;
	EVENTLOGRECORD *event;
	HANDLE log;
	HMODULE mod;			

	if (argc != 3) {
		printf("syntax: driver mesg driver_path");
		return -1;
	}
	

	/*find out the full qualified driver path */
	do {
		if (driver_path)
			free(driver_path);
		driver_path=malloc((len=GetFullPathName(argv[2], 0, driver_path, NULL)));
	} while (GetFullPathName(argv[2], len, driver_path, &b) > len);
	
	/* find out the driver name */
	if ((e=strrchr(b, '.')) == NULL)
		e=b+strlen(b);
	else
		e--;
	driver_name=malloc(e-b+2);
	memcpy(driver_name, b, e-b+1); driver_name[e-b+1]=0;


	if (!(log=OpenEventLog(NULL, "System"))) {
		printf("can't open event log: "); 
		print_error();
		return -1;
	}

	printf("%s\n", driver_path);

	if (!(mod=LoadLibraryEx(driver_path, NULL, LOAD_LIBRARY_AS_DATAFILE))) {
		printf("can't open '%s': ", driver_path);
		print_error();
		error=-1;
		goto out;
	}


	GetOldestEventLogRecord(log, &dummy); len=4096;
	while (ReadEventLog(log, EVENTLOG_FORWARDS_READ|EVENTLOG_SEQUENTIAL_READ, 0, buffer, len, &readed, &needed)) {
		event=(EVENTLOGRECORD*)buffer;
		while (readed > 0) { 
			source=(char*)event+sizeof(EVENTLOGRECORD);
			if (strcmp(driver_name, source) == 0) {
				char *message, *type;
				char **args;
				int i;


				switch (event->EventType){
				case EVENTLOG_ERROR_TYPE:
					type="error";
					break;
				case EVENTLOG_WARNING_TYPE:
					type="warning";
					break;
				case EVENTLOG_INFORMATION_TYPE:
					type="info";
					break;
				default:
					type="unknown";
				}

				if ((args=malloc(sizeof(WCHAR*)*event->NumStrings)))
					args[0]=(char*)event+event->StringOffset;
				for(i=1; i<event->NumStrings; i++)
					args[i]=args[i-1]+strlen(args[i-1])+1;

				if (!(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_HMODULE|FORMAT_MESSAGE_ARGUMENT_ARRAY,
					mod, event->EventID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&message, 0, args)))
					print_error();
				
				printf("%s: %s", type, message);
				LocalFree(message); free((void*)args);
			}

			readed-=event->Length;
			event=(EVENTLOGRECORD*)((char*)event+event->Length);
		}
	}
	print_error();
	
out:
	CloseEventLog(log);
	FreeLibrary(mod);
	return error;
}

int main(int argc, char **argv)
{
	int error=0;
	int driver_type=SERVICE_KERNEL_DRIVER;


	if (argc < 2) {
		error=-1;
		goto out;
	}


	if (strcmp(argv[1], "load") == 0)
		return load(argc, argv, SERVICE_KERNEL_DRIVER);
	if (strcmp(argv[1], "unload") == 0)
		return unload(argc, argv);
	if (strcmp(argv[1], "load_fs") == 0)
		return load(argc, argv, SERVICE_FILE_SYSTEM_DRIVER);
	if (strcmp(argv[1], "list") == 0)
		return list(argc, argv);
	if (strcmp(argv[1], "mesg") == 0)
		return mesg(argc, argv);

	error=-1;
out:
	printf("syntax: driver [ load | load_fs | list | unload | mesg ] ... \n");	
	return error;
}

