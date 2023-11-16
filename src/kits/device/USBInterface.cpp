/*
 * Copyright 2007-2008, Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Lotz <mmlr@mlotz.ch>
 *		Salvatore Benedetto <salvatore.benedetto@gmail.com>
 */

#include <USBKit.h>
#include <usb_raw.h>

#include <new>
#include <kits/support/String.h>
#include <unistd.h>


BUSBInterface::BUSBInterface(BUSBConfiguration *config, uint32_t index,
	uint32_t alternate, int rawFD)
	:	fConfiguration(config),
		fIndex(index),
		fAlternate(alternate),
		fRawFD(rawFD),
		fEndpoints(NULL),
		fAlternateCount(0),
		fAlternates(NULL),
		fInterfaceString(NULL)
{
	_UpdateDescriptorAndEndpoints();
}


BUSBInterface::~BUSBInterface()
{
	delete[] fInterfaceString;

	if (fEndpoints != NULL) {
		for (int32_t i = 0; i < fDescriptor.num_endpoints; i++)
			delete fEndpoints[i];
		delete[] fEndpoints;
	}

	if (fAlternates != NULL) {
		for (uint32_t i = 0; i < fAlternateCount; i++)
			delete fAlternates[i];
		delete[] fAlternates;
	}
}


uint32_t BUSBInterface::Index() const
{
	return fIndex;
}


uint32_t BUSBInterface::AlternateIndex() const
{
	if (fAlternate == B_USB_RAW_ACTIVE_ALTERNATE)
		return ActiveAlternateIndex();
	return fAlternate;
}


const BUSBConfiguration *
BUSBInterface::Configuration() const
{
	return fConfiguration;
}


const BUSBDevice *
BUSBInterface::Device() const
{
	return fConfiguration->Device();
}


uint8
BUSBInterface::Class() const
{
	return fDescriptor.interface_class;
}


uint8
BUSBInterface::Subclass() const
{
	return fDescriptor.interface_subclass;
}


uint8
BUSBInterface::Protocol() const
{
	return fDescriptor.interface_protocol;
}


const char *
BUSBInterface::InterfaceString() const
{
	if (fDescriptor.interface == 0)
		return "";

	if (fInterfaceString)
		return fInterfaceString;

	fInterfaceString = Device()->DecodeStringDescriptor(fDescriptor.interface);
	if (fInterfaceString == NULL)
		return "";

	return fInterfaceString;
}


const usbe_interface_descriptor *
BUSBInterface::Descriptor() const
{
	return &fDescriptor;
}


status_t BUSBInterface::OtherDescriptorAt(uint32_t index, usb_descriptor *descriptor,
	size_t length) const
{
	if (length <= 0 || descriptor == NULL)
		return B_BAD_VALUE;

	usb_raw_command command;
	command.generic_etc.descriptor = descriptor;
	command.generic_etc.config_index = fConfiguration->Index();
	command.generic_etc.interface_index = fIndex;
	command.generic_etc.alternate_index = fAlternate;
	command.generic_etc.generic_index = index;
	command.generic_etc.length = length;
	if (ioctl(fRawFD, B_USB_RAW_COMMAND_GET_GENERIC_DESCRIPTOR_ETC, &command,
		sizeof(command)) || command.generic.status != B_USB_RAW_STATUS_SUCCESS)
		return B_ERROR;

	return B_OK;
}


uint32_t BUSBInterface::CountEndpoints() const
{
	return fDescriptor.num_endpoints;
}


const BUSBEndpoint *
BUSBInterface::EndpointAt(uint32_t index) const
{
	if (index >= fDescriptor.num_endpoints || fEndpoints == NULL)
		return NULL;

	return fEndpoints[index];
}


uint32_t BUSBInterface::CountAlternates() const
{
	usb_raw_command command;
	command.alternate.config_index = fConfiguration->Index();
	command.alternate.interface_index = fIndex;
	if (ioctl(fRawFD, B_USB_RAW_COMMAND_GET_ALT_INTERFACE_COUNT, &command,
		sizeof(command)) || command.alternate.status != B_USB_RAW_STATUS_SUCCESS)
		return 1;

	return command.alternate.alternate_info;
}


const BUSBInterface *
BUSBInterface::AlternateAt(uint32_t alternateIndex) const
{
	if (fAlternateCount > 0 && fAlternates != NULL) {
		if (alternateIndex >= fAlternateCount)
			return NULL;

		return fAlternates[alternateIndex];
	}

	if (fAlternateCount == 0)
		fAlternateCount = CountAlternates();
	if (alternateIndex >= fAlternateCount)
		return NULL;

	fAlternates = new(std::nothrow) BUSBInterface *[fAlternateCount];
	if (fAlternates == NULL)
		return NULL;

	for (uint32_t i = 0; i < fAlternateCount; i++) {
		fAlternates[i] = new(std::nothrow) BUSBInterface(fConfiguration, fIndex,
			i, fRawFD);
	}

	return fAlternates[alternateIndex];
}


uint32_t BUSBInterface::ActiveAlternateIndex() const
{
	usb_raw_command command;
	command.alternate.config_index = fConfiguration->Index();
	command.alternate.interface_index = fIndex;
	if (ioctl(fRawFD, B_USB_RAW_COMMAND_GET_ACTIVE_ALT_INTERFACE_INDEX, &command,
		sizeof(command)) || command.alternate.status != B_USB_RAW_STATUS_SUCCESS)
		return 0;

	return command.alternate.alternate_info;
}


status_t BUSBInterface::SetAlternate(uint32_t alternateIndex)
{
	usb_raw_command command;
	command.alternate.alternate_info = alternateIndex;
	command.alternate.config_index = fConfiguration->Index();
	command.alternate.interface_index = fIndex;
	if (ioctl(fRawFD, B_USB_RAW_COMMAND_SET_ALT_INTERFACE, &command,
		sizeof(command)) || command.alternate.status != B_USB_RAW_STATUS_SUCCESS)
		return B_ERROR;

	_UpdateDescriptorAndEndpoints();
	return B_OK;
}


void BUSBInterface::_UpdateDescriptorAndEndpoints()
{
	usb_raw_command command;
	command.interface_etc.descriptor = &fDescriptor;
	command.interface_etc.config_index = fConfiguration->Index();
	command.interface_etc.interface_index = fIndex;
	command.interface_etc.alternate_index = fAlternate;
	if (ioctl(fRawFD, B_USB_RAW_COMMAND_GET_INTERFACE_DESCRIPTOR_ETC, &command,
		sizeof(command)) || command.interface.status != B_USB_RAW_STATUS_SUCCESS)
		memset(&fDescriptor, 0, sizeof(fDescriptor));

	if (fEndpoints != NULL) {
		// Delete old endpoints
		for (int32_t i = 0; i < fDescriptor.num_endpoints; i++)
			delete fEndpoints[i];
		delete[] fEndpoints;
	}

	fEndpoints = new(std::nothrow) BUSBEndpoint *[fDescriptor.num_endpoints];
	if (fEndpoints == NULL)
		return;

	for (int32_t i = 0; i < fDescriptor.num_endpoints; i++)
		fEndpoints[i] = new(std::nothrow) BUSBEndpoint(this, i, fRawFD);
}
