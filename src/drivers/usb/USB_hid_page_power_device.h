/*
 * Copyright 2004-2010, Haiku Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _USB_HID_PAGE_POWER_DEVICE_H
#define _USB_HID_PAGE_POWER_DEVICE_H


/* Reference:
 *		HID Usage Page 0x84: POWER DEVICE
 *		Universal Serial Bus Usage Tables for HID Power Devices Ver. 1.0
 *		http://www.usb.org/developers/devclass_docs/pdcv10.pdf
 */

// Usage IDs
enum {
	B_HID_UID_POW_INAME = 0x01,
	B_HID_UID_POW_PRESENT_STATUS,
	B_HID_UID_POW_CHANGED_STATUS,
	B_HID_UID_POW_UPS,
	B_HID_UID_POW_POWER_SUPPLY,
	
	B_HID_UID_POW_BATTERY_SYSTEM = 0x10,
	B_HID_UID_POW_BATTERY_SYSTEM_ID,
	B_HID_UID_POW_BATTERY,
	B_HID_UID_POW_BATERY_ID,
	B_HID_UID_POW_CHARGER,
	B_HID_UID_POW_CHARGER_ID,
	B_HID_UID_POW_POWER_CONVERTER,
	B_HID_UID_POW_POWER_CONVERTER_ID,
	B_HID_UID_POW_OUTLET_SYSTEM,
	B_HID_UID_POW_OUTLET_SYSTEM_ID,
	B_HID_UID_POW_INPUT,
	B_HID_UID_POW_INPUT_ID,
	B_HID_UID_POW_OUTPUT,
	B_HID_UID_POW_OUTPUT_ID,
	B_HID_UID_POW_FLOW,
	B_HID_UID_POW_FLOW_ID,
	B_HID_UID_POW_OUTLET,
	B_HID_UID_POW_OUTLET_ID,
	B_HID_UID_POW_GANG,
	B_HID_UID_POW_GANG_ID,
	B_HID_UID_POW_POWER_SUMMARY,
	B_HID_UID_POW_POWER_SUMMARY_ID,
	
	B_HID_UID_POW_VOLTAGE = 0x30,
	B_HID_UID_POW_CURRENT,
	B_HID_UID_POW_FREQUENCY,
	B_HID_UID_POW_APPARENT_POWER,
	B_HID_UID_POW_ACTIVE_POWER,
	B_HID_UID_POW_PERCENT_LOAD,
	B_HID_UID_POW_TEMPERATURE,
	B_HID_UID_POW_HUMIDITY,
	B_HID_UID_POW_BAD_COUNT,
	
	B_HID_UID_POW_CONFIG_VOLTAGE = 0x40,
	B_HID_UID_POW_CONFIG_CURRENT,
	B_HID_UID_POW_CONFIG_FREQUENCY,
	B_HID_UID_POW_CONFIG_APPARENT_POWER,
	B_HID_UID_POW_CONFIG_ACTIVE_POWER,
	B_HID_UID_POW_CONFIG_PERCENT_LOAD,
	B_HID_UID_POW_CONFIG_TEMPERATURE,
	B_HID_UID_POW_CONFIG_HUMIDITY,
	
	B_HID_UID_POW_SWITCH_ON_CONTROL = 0x50,
	B_HID_UID_POW_SWITCH_OFF_CONTROL,
	B_HID_UID_POW_TOGGLE_CONTROL,
	B_HID_UID_POW_LOW_VOLTAGE_TRANSFER,
	B_HID_UID_POW_HIGH_VOLTAGE_TRANSFER,
	B_HID_UID_POW_DELAY_BEFORE_REBOOT,
	B_HID_UID_POW_DELAY_BEFORE_STARTUP,
	B_HID_UID_POW_DELAY_BEFORE_SHUTDOWN,
	B_HID_UID_POW_TEST,
	B_HID_UID_POW_MODULE_RESET,
	B_HID_UID_POW_AUDIBLE_ALARM_CONTROL,
	
	B_HID_UID_POW_PRESENT = 0x60,
	B_HID_UID_POW_GOOD,
	B_HID_UID_POW_INTERNAL_FAILURE,
	B_HID_UID_POW_VOLTAGE_OUT_OF_RANGE,
	B_HID_UID_POW_FREQUENCY_OUT_OF_RANGE,
	B_HID_UID_POW_OVERLOAD,
	B_HID_UID_POW_OVER_CHARGED,
	B_HID_UID_POW_OVER_TEMPERATURE,
	B_HID_UID_POW_SHUTDOWN_REQUEST,
	B_HID_UID_POW_SHUTDOWN_IMMINENT,
	
	B_HID_UID_POW_SWITCH_ON_OFF  =0x6b,
	B_HID_UID_POW_SWITCHABLE,
	B_HID_UID_POW_USED,
	B_HID_UID_POW_BOOST,
	B_HID_UID_POW_BUCK,
	B_HID_UID_POW_INITIALIZED,
	B_HID_UID_POW_TESTED,
	B_HID_UID_POW_AWAITING_POWER,
	B_HID_UID_POW_COMMUNICATION_LOST,
	
	B_HID_UID_POW_IMANUFACTURER = 0xfd,
	B_HID_UID_POW_IPRODUCT,
	B_HID_UID_POW_ISERIAL_NUMBER
};


#endif // _USB_HID_PAGE_POWER_DEVICE_H
