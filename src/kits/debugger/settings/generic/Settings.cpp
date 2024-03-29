/*
 * Copyright 2013, Rene Gollent, rene@gollent.com.
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include <Settings.h>

#include <AutoLocker.h>

#include <SettingsDescription.h>


// #pragma mark - Settings


Settings::Settings(SettingsDescription* description)
	:
	fLock("settings"),
	fDescription(description)
{
	fDescription->AcquireReference();
}


Settings::~Settings()
{
	fDescription->ReleaseReference();
}


status_t Settings::Init()
{
	return fLock.InitCheck();
}


BVariant
Settings::Value(Setting* setting) const
{
	AutoLocker<BLocker> locker(fLock);

	BVariant value;
	return value.SetFromMessage(fValues, setting->ID()) == B_OK
		? value : setting->DefaultValue();
}


BVariant
Settings::Value(const char* settingID) const
{
	AutoLocker<BLocker> locker(fLock);

	BVariant value;
	if (value.SetFromMessage(fValues, settingID) == B_OK)
		return value;

	Setting* setting = fDescription->SettingByID(settingID);
	return setting != NULL ? setting->DefaultValue() : value;
}


bool Settings::SetValue(Setting* setting, const BVariant& value)
{
	AutoLocker<BLocker> locker(fLock);

	// remove the message field and re-add it with the new value
	const char* fieldName = setting->ID();
	fValues.RemoveName(fieldName);

	bool success = value.AddToMessage(fValues, fieldName) == B_OK;

	// notify the listeners
	int32_t count = fListeners.CountItems();
	for (int32_t i = count - 1; i >= 0; i--)
		fListeners.ItemAt(i)->SettingValueChanged(setting);

	return success;
}


bool Settings::RestoreValues(const BMessage& message)
{
	AutoLocker<BLocker> locker(fLock);

	for (int32_t i = 0; i < fDescription->CountSettings(); i++) {
		Setting* setting = fDescription->SettingAt(i);
		BVariant value;
		if (value.SetFromMessage(message, setting->ID()) == B_OK) {
			if (!SetValue(setting, value))
				return false;
		}
	}

	return true;
}


SettingsOption*
Settings::OptionValue(OptionsSetting* setting) const
{
	BVariant value = Value(setting);
	return value.Type() == B_STRING_TYPE
		? setting->OptionByID(value.ToString())
		: setting->DefaultOption();
}


bool Settings::AddListener(Listener* listener)
{
	AutoLocker<BLocker> locker(fLock);
	return fListeners.AddItem(listener);
}


void Settings::RemoveListener(Listener* listener)
{
	AutoLocker<BLocker> locker(fLock);
	fListeners.RemoveItem(listener);
}


// #pragma mark - Listener


Settings::Listener::~Listener()
{
}
