/*
Open Tracker License

Terms and Conditions

Copyright (c) 1991-2000, Be Incorporated. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice applies to all licensees
and shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF TITLE, MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
BE INCORPORATED BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Be Incorporated shall not be
used in advertising or otherwise to promote the sale, use or other dealings in
this Software without prior written authorization from Be Incorporated.

Tracker(TM), Be(R), BeOS(R), and BeIA(TM) are trademarks or registered trademarks
of Be Incorporated in the United States and other countries. Other brand product
names are registered trademarks or trademarks of their respective holders.
All rights reserved.
*/


#include <Message.h>
#include <PropertyInfo.h>

#include <ContainerWindow.h>
#include <FSUtils.h>
#include <Tracker.h>


#define kPropertyTrash "Trash"
#define kPropertyFolder "Folder"
#define kPropertyPreferences "Preferences"


/*
Available scripting commands:

doo Tracker delete Trash
doo Tracker create Folder to '/boot/home/Desktop/hello'		# mkdir
doo Tracker get Folder to '/boot/home/Desktop/hello'		# get window for path
doo Tracker execute Folder to '/boot/home/Desktop/hello'	# open window

ToDo:
Create file: on a "Tracker" "File" "B_CREATE_PROPERTY" "name"
Create query: on a "Tracker" "Query" "B_CREATE_PROPERTY" "name"
*/


const property_info kTrackerPropertyList[] = {
	{
		kPropertyTrash,
		{ B_DELETE_PROPERTY },
		{ B_DIRECT_SPECIFIER },
		"delete Trash # Empties the Trash",
		0,
		{},
		{},
		{}
	},
	{
		kPropertyFolder,
		{ B_CREATE_PROPERTY, B_GET_PROPERTY, B_EXECUTE_PROPERTY },
		{ B_DIRECT_SPECIFIER },
		"create Folder to path # creates a new folder\n"
		"get Folder to path # get Tracker window pointing to folder\n"
		"execute Folder to path # opens Tracker window pointing to folder\n",
		0,
		{ B_REF_TYPE },
		{},
		{}
	},
	{
		kPropertyPreferences,
		{ B_EXECUTE_PROPERTY },
		{ B_DIRECT_SPECIFIER },
		"shows Tracker preferences",
		0,
		{},
		{},
		{}
	},
	{
		NULL,
		{},
		{},
		NULL,
		0,
		{},
		{},
		{}
	}
};


status_t TTracker::GetSupportedSuites(BMessage* data)
{
	data->AddString("suites", kTrackerSuites);
	BPropertyInfo propertyInfo(const_cast<property_info*>(
		kTrackerPropertyList));
	data->AddFlat("messages", &propertyInfo);

	return _inherited::GetSupportedSuites(data);
}


BHandler*
TTracker::ResolveSpecifier(BMessage* message, int32_t index, BMessage* specifier,
	int32_t form, const char* property)
{
	BPropertyInfo propertyInfo(const_cast<property_info*>(
		kTrackerPropertyList));

	int32_t result = propertyInfo.FindMatch(message, index, specifier, form,
		property);
	if (result < 0) {
		//PRINT(("FindMatch result %d %s\n", result, strerror(result)));
		return _inherited::ResolveSpecifier(message, index, specifier,
			form, property);
	}

	return this;
}


bool TTracker::HandleScriptingMessage(BMessage* message)
{
	if (message->what != B_GET_PROPERTY
		&& message->what != B_SET_PROPERTY
		&& message->what != B_CREATE_PROPERTY
		&& message->what != B_COUNT_PROPERTIES
		&& message->what != B_DELETE_PROPERTY
		&& message->what != B_EXECUTE_PROPERTY) {
		return false;
	}

	// dispatch scripting messages
	BMessage reply(B_REPLY);
	const char* property = NULL;
	bool handled = false;

	int32_t index = 0;
	int32_t form = 0;
	BMessage specifier;

	status_t result = message->GetCurrentSpecifier(&index, &specifier,
		&form, &property);
	if (result != B_OK || index == -1)
		return false;

	ASSERT(property != NULL);

	switch (message->what) {
		case B_CREATE_PROPERTY:
			handled = CreateProperty(message, &specifier, form, property,
				&reply);
			break;

		case B_GET_PROPERTY:
			handled = GetProperty(message, form, property, &reply);
			break;

		case B_SET_PROPERTY:
			handled = SetProperty(message, &specifier, form, property,
				&reply);
			break;

		case B_COUNT_PROPERTIES:
			handled = CountProperty(&specifier, form, property, &reply);
			break;

		case B_DELETE_PROPERTY:
			handled = DeleteProperty(&specifier, form, property, &reply);
			break;

		case B_EXECUTE_PROPERTY:
			handled = ExecuteProperty(message, form, property, &reply);
			break;
	}

	if (handled) {
		// done handling message, send a reply
		message->SendReply(&reply);
	}

	return handled;
}


bool TTracker::CreateProperty(BMessage* message, BMessage*, int32_t form,
	const char* property, BMessage* reply)
{
	bool handled = false;
	status_t error = B_OK;

	if (strcmp(property, kPropertyFolder) == 0) {
		if (form != B_DIRECT_SPECIFIER)
			return false;

		// create new empty folders
		entry_ref ref;
		for (int32_t index = 0;
			message->FindRef("data", index, &ref) == B_OK; index++) {

			BEntry entry(&ref);
			if (!entry.Exists())
				error = FSCreateNewFolder(&ref);

			if (error != B_OK)
				break;
		}

		handled = true;
	}

	if (error != B_OK)
		reply->AddInt32("error", error);

	return handled;
}


bool TTracker::DeleteProperty(BMessage*, int32_t form, const char* property, BMessage*)
{
	if (strcmp(property, kPropertyTrash) == 0) {
		// deleting on a selection is handled as removing a part of the
		// selection not to be confused with deleting a selected item

		if (form != B_DIRECT_SPECIFIER) {
			// only support direct specifier
			return false;
		}

		// empty the trash
		FSEmptyTrash();

		return true;

	}

	return false;
}


bool TTracker::ExecuteProperty(BMessage* message, int32_t form, const char* property,
	BMessage* reply)
{
	if (strcmp(property, kPropertyPreferences) == 0) {
		if (form != B_DIRECT_SPECIFIER) {
			// only support direct specifier
			return false;
		}
		ShowSettingsWindow();

		return true;
	}

	if (strcmp(property, kPropertyFolder) == 0) {
		message->PrintToStream();
		if (form != B_DIRECT_SPECIFIER)
			return false;

		// create new empty folders
		entry_ref ref;
		for (int32_t index = 0;
			message->FindRef("data", index, &ref) == B_OK; index++) {
			status_t error = OpenRef(&ref, NULL, NULL, kOpen, NULL);

			if (error == B_OK) {
				reply->AddMessenger("window",
					BMessenger(FindContainerWindow(&ref)));
			} else {
				reply->AddInt32("error", error);
				break;
			}
		}

		return true;
	}

	return false;
}


bool TTracker::CountProperty(BMessage*, int32_t, const char*, BMessage*)
{
	return false;
}


bool TTracker::GetProperty(BMessage* message, int32_t form, const char* property,
		BMessage* reply)
{
	if (strcmp(property, kPropertyFolder) == 0) {
		message->PrintToStream();
		if (form != B_DIRECT_SPECIFIER)
			return false;

		// create new empty folders
		entry_ref ref;
		for (int32_t index = 0;
			message->FindRef("data", index, &ref) == B_OK; index++) {
			BHandler* window = FindContainerWindow(&ref);

			if (window != NULL)
				reply->AddMessenger("window", BMessenger(window));
			else {
				reply->AddInt32("error", B_NAME_NOT_FOUND);
				break;
			}
		}

		return true;
	}

	return false;
}


bool TTracker::SetProperty(BMessage*, BMessage*, int32_t, const char*, BMessage*)
{
	return false;
}
