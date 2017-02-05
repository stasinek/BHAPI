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
 * File: FindDirectory.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#ifdef _WIN32_WINNT
#include <userenv.h>
#endif
extern HINSTANCE etk_dll_hinstance;
#endif

#ifdef __BEOS__
#include <be/storage/FindDirectory.h>
#include <be/storage/Volume.h>
#include <be/storage/Path.h>
#endif // __BEOS__


#include "./../support/SupportDefs.h"
#include "./../support/String.h"

#include "FindDirectory.h"

e_status_t e_find_directory(e_directory_which which, EPath *path)
{
	if(path == NULL) return E_ERROR;

	e_status_t retVal = E_ERROR;

#ifdef _WIN32
	char buffer[E_MAXPATH + 1];

	// here we find directory contains libetk.dll
	bzero(buffer, sizeof(buffer));
    if(GetModuleFileNameA((HMODULE)etk_dll_hinstance, buffer, E_MAXPATH) == 0) return E_ERROR;
	EPath prefixPath;
	prefixPath.SetTo(buffer);
	prefixPath.GetParent(&prefixPath);
	if(prefixPath.Path() == NULL) return E_ERROR;

	EString homeDir;
#ifdef _WIN32_WINNT
	HANDLE hToken = INVALID_HANDLE_VALUE;
	OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);
	if(hToken != INVALID_HANDLE_VALUE)
	{
		DWORD len = E_MAXPATH;
		bzero(buffer, sizeof(buffer));
        if(GetUserProfileDirectoryA(hToken, buffer, &len) != 0) homeDir = buffer;
		CloseHandle(hToken);
	}
#endif
	if(homeDir.Length() <= 0)
	{
		char userName[1025];
		DWORD userNameLen = (DWORD)sizeof(userName);
		bzero(userName, sizeof(userName));
        homeDir << prefixPath.Path() << "/." << (GetUserNameA(userName, &userNameLen) == 0 ? "user" : userName);
	}

	bzero(buffer, sizeof(buffer));
	switch(which)
	{
		case E_BOOT_DIRECTORY:
            if(GetWindowsDirectoryA(buffer, E_MAXPATH) != 0)
			{
				buffer[3] = '\0';
				if(path->SetTo(buffer) == E_OK) retVal = E_OK;
			}
			break;

		case E_APPS_DIRECTORY:
            if(GetWindowsDirectoryA(buffer, E_MAXPATH) != 0)
			{
				buffer[3] = '\0';
				if(path->SetTo(buffer, "Program Files") == E_OK) retVal = E_OK;
			}
			break;

		case E_BIN_DIRECTORY:
			if(!(path->SetTo(prefixPath.Path()) != E_OK ||
			     path->GetParent(path) != E_OK ||
			     path->Append("bin", true) != E_OK))
				retVal = E_OK;
			else if(path->SetTo(prefixPath.Path()) == E_OK)
				retVal = E_OK;
			break;

		case E_LIB_DIRECTORY:
			if(!(path->SetTo(prefixPath.Path()) != E_OK ||
			     path->GetParent(path) != E_OK ||
			     path->Append("lib", true) != E_OK))
				retVal = E_OK;
            else if(!(GetSystemDirectoryA(buffer, E_MAXPATH) == 0 || path->SetTo(buffer) != E_OK))
				retVal = E_OK;
			break;

		case E_ETC_DIRECTORY:
			if(!(path->SetTo(prefixPath.Path()) != E_OK ||
			     path->GetParent(path) != E_OK ||
			     path->Append("etc") != E_OK)) retVal = E_OK;
			break;

		case E_ADDONS_DIRECTORY:
			if(!(path->SetTo(prefixPath.Path()) != E_OK ||
			     path->GetParent(path) != E_OK ||
			     path->Append("lib/add-ons") != E_OK)) retVal = E_OK;
			break;

		case E_TEMP_DIRECTORY:
            if(!(GetTempPathA(E_MAXPATH, buffer) == 0 || path->SetTo(buffer) != E_OK)) retVal = E_OK;
			break;

		case E_USER_DIRECTORY:
			if(path->SetTo(homeDir.String()) == E_OK) retVal = E_OK;
			break;

		case E_USER_CONFIG_DIRECTORY:
			if(path->SetTo(homeDir.String(), "config") == E_OK) retVal = E_OK;
			break;

		case E_USER_BIN_DIRECTORY:
			if(path->SetTo(homeDir.String(), "config/bin") == E_OK) retVal = E_OK;
			break;

		case E_USER_LIB_DIRECTORY:
			if(path->SetTo(homeDir.String(), "config/lib") == E_OK) retVal = E_OK;
			break;

		case E_USER_ETC_DIRECTORY:
			if(path->SetTo(homeDir.String(), "config/etc") == E_OK) retVal = E_OK;
			break;

		case E_USER_ADDONS_DIRECTORY:
			if(path->SetTo(homeDir.String(), "config/add-ons") == E_OK) retVal = E_OK;
			break;

		default:
			break;
	}
#else // !_WIN32
#ifdef ETK_OS_BEOS
	BPath bPath;

	switch(which)
	{
		case E_BOOT_DIRECTORY:
			if(find_directory(B_BEOS_BOOT_DIRECTORY, &bPath, false, NULL) != B_OK) break;
			if(path->SetTo(bPath.Path()) == E_OK) retVal = E_OK;
			break;

		case E_APPS_DIRECTORY:
			if(find_directory(B_BEOS_APPS_DIRECTORY, &bPath, false, NULL) != B_OK) break;
			if(path->SetTo(bPath.Path()) == E_OK) retVal = E_OK;
			break;

		case E_BIN_DIRECTORY:
			if(find_directory(B_BEOS_BIN_DIRECTORY, &bPath, false, NULL) != B_OK) break;
			if(path->SetTo(bPath.Path()) == E_OK) retVal = E_OK;
			break;

		case E_LIB_DIRECTORY:
			if(find_directory(B_BEOS_LIB_DIRECTORY, &bPath, false, NULL) != B_OK) break;
			if(path->SetTo(bPath.Path()) == E_OK) retVal = E_OK;
			break;

		case E_ETC_DIRECTORY:
			if(find_directory(B_BEOS_ETC_DIRECTORY, &bPath, false, NULL) != B_OK) break;
			if(path->SetTo(bPath.Path()) == E_OK) retVal = E_OK;
			break;

		case E_ADDONS_DIRECTORY:
			if(find_directory(B_BEOS_ADDONS_DIRECTORY, &bPath, false, NULL) != B_OK) break;
			if(path->SetTo(bPath.Path()) == E_OK) retVal = E_OK;
			break;

		case E_TEMP_DIRECTORY:
			if(find_directory(B_COMMON_TEMP_DIRECTORY, &bPath, false, NULL) != B_OK) break;
			if(path->SetTo(bPath.Path()) == E_OK) retVal = E_OK;
			break;

		case E_USER_DIRECTORY:
			if(find_directory(B_USER_DIRECTORY, &bPath, false, NULL) != B_OK) break;
			if(path->SetTo(bPath.Path()) == E_OK) retVal = E_OK;
			break;

		case E_USER_CONFIG_DIRECTORY:
			if(find_directory(B_USER_CONFIG_DIRECTORY, &bPath, false, NULL) != B_OK) break;
			if(path->SetTo(bPath.Path()) == E_OK) retVal = E_OK;
			break;

		case E_USER_BIN_DIRECTORY:
			if(find_directory(B_USER_CONFIG_DIRECTORY, &bPath, false, NULL) != B_OK) break;
			if(path->SetTo(bPath.Path(), "bin") == E_OK) retVal = E_OK;
			break;

		case E_USER_LIB_DIRECTORY:
			if(find_directory(B_USER_LIB_DIRECTORY, &bPath, false, NULL) != B_OK) break;
			if(path->SetTo(bPath.Path()) == E_OK) retVal = E_OK;
			break;

		case E_USER_ETC_DIRECTORY:
			if(find_directory(B_USER_CONFIG_DIRECTORY, &bPath, false, NULL) != B_OK) break;
			if(path->SetTo(bPath.Path(), "etc") == E_OK) retVal = E_OK;
			break;

		case E_USER_ADDONS_DIRECTORY:
			if(find_directory(B_USER_ADDONS_DIRECTORY, &bPath, false, NULL) != B_OK) break;
			if(path->SetTo(bPath.Path()) == E_OK) retVal = E_OK;
			break;

		default:
			break;
	}
#else // !ETK_OS_BEOS
	switch(which)
	{
		case E_BOOT_DIRECTORY:
			if(path->SetTo("/") == E_OK) retVal = E_OK;
			break;

		case E_APPS_DIRECTORY:
			if(path->SetTo(DATA_DIR, "apps") == E_OK) retVal = E_OK;
			break;

		case E_BIN_DIRECTORY:
			if(path->SetTo(BIN_DIR) == E_OK) retVal = E_OK;
			break;

		case E_LIB_DIRECTORY:
			if(path->SetTo(LIB_DIR) == E_OK) retVal = E_OK;
			break;

		case E_ETC_DIRECTORY:
			if(path->SetTo(ETC_DIR) == E_OK) retVal = E_OK;
			break;

		case E_ADDONS_DIRECTORY:
			if(path->SetTo(LIB_DIR, "add-ons") == E_OK) retVal = E_OK;
			break;

		case E_TEMP_DIRECTORY:
			if(path->SetTo("/tmp") == E_OK) retVal = E_OK;
			break;

		case E_USER_DIRECTORY:
			if(path->SetTo(getenv("HOME")) == E_OK) retVal = E_OK;
			break;

		case E_USER_CONFIG_DIRECTORY:
			if(path->SetTo(getenv("HOME"), ".config") == E_OK) retVal = E_OK;
			break;

		case E_USER_BIN_DIRECTORY:
			if(path->SetTo(getenv("HOME"), ".config/bin") == E_OK) retVal = E_OK;
			break;

		case E_USER_LIB_DIRECTORY:
			if(path->SetTo(getenv("HOME"), ".config/lib") == E_OK) retVal = E_OK;
			break;

		case E_USER_ETC_DIRECTORY:
			if(path->SetTo(getenv("HOME"), ".config/etc") == E_OK) retVal = E_OK;
			break;

		case E_USER_ADDONS_DIRECTORY:
			if(path->SetTo(getenv("HOME"), ".config/add-ons") == E_OK) retVal = E_OK;
			break;

		default:
			break;
	}
#endif // ETK_OS_BEOS
#endif // _WIN32

	return retVal;
}

