/*
 * Copyright 2004-2010, Haiku Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _USB_HID_PAGE_LED_H
#define _USB_HID_PAGE_LED_H


/* Reference:
 *		HID Usage Page 0x08: LED
 *		HID Usage Tables Ver. 1.12
 *		http://www.usb.org/developers/devclass_docs/Hut1_12.pdf
 */

// Usage IDs
enum {
	B_HID_UID_LED_NUM_LOCK = 0x01,
	B_HID_UID_LED_CAPS_LOCK,
	B_HID_UID_LED_SCROLL_LOCK,
	B_HID_UID_LED_COMPOSE,
	B_HID_UID_LED_KANA,
	B_HID_UID_LED_POWER,
	B_HID_UID_LED_SHIFT,
	B_HID_UID_LED_DO_NOT_DISTURB,
	B_HID_UID_LED_MUTE,
	B_HID_UID_LED_TONE_ENABLE,
	B_HID_UID_LED_HIGH_CUT_FILTER,
	B_HID_UID_LED_LOW_CUT_FILTER,
	B_HID_UID_LED_EQUALIZER_ENABLE,
	B_HID_UID_LED_SOUND_FIELD_ON,
	B_HID_UID_LED_SURROUND_ON,
	B_HID_UID_LED_REPEAT,
	B_HID_UID_LED_STEREO,
	B_HID_UID_LED_SAMPLING_RATE_DETECT,
	B_HID_UID_LED_SPINNING,
	B_HID_UID_LED_CAV,
	B_HID_UID_LED_CAL,
	B_HID_UID_LED_RECORDING_FORMAT_DETECT,
	B_HID_UID_LED_OFF_HOOK,
	B_HID_UID_LED_RING,
	B_HID_UID_LED_MESSAGE_WAITING,
	B_HID_UID_LED_DATA_MODE,
	B_HID_UID_LED_BATTERY_OPERATION,
	B_HID_UID_LED_BATTERY_OK,
	B_HID_UID_LED_BATTERY_LOW,
	B_HID_UID_LED_SPEAKER,
	B_HID_UID_LED_HEAD_SET,
	B_HID_UID_LED_HOLD,
	B_HID_UID_LED_MICROPHONE,
	B_HID_UID_LED_COVERAGE,
	B_HID_UID_LED_NIGHT_MODE,
	B_HID_UID_LED_SEND_CALLS,
	B_HID_UID_LED_CALL_PICKUP,
	B_HID_UID_LED_CONFERENCE,
	B_HID_UID_LED_STAND_BY,
	B_HID_UID_LED_CAMERA_ON,
	B_HID_UID_LED_CAMERA_OFF,
	B_HID_UID_LED_ON_LINE,
	B_HID_UID_LED_OFF_LINE,
	B_HID_UID_LED_BUSY,
	B_HID_UID_LED_READY,
	B_HID_UID_LED_PAPER_OUT,
	B_HID_UID_LED_PAPER_JAM,
	B_HID_UID_LED_REMOTE,
	B_HID_UID_LED_FORWARD,
	B_HID_UID_LED_REVERSE,
	B_HID_UID_LED_STOP,
	B_HID_UID_LED_REWIND,
	B_HID_UID_LED_FAST_FORWARD,
	B_HID_UID_LED_PLAY,
	B_HID_UID_LED_PAUSE,
	B_HID_UID_LED_RECORD,
	B_HID_UID_LED_ERROR,
	B_HID_UID_LED_USAGE_SELECTED_INDICATOR,
	B_HID_UID_LED_USAGE_IN_USE_INDICATOR,
	B_HID_UID_LED_USAGE_MULTI_MODE_INDICATOR,
	B_HID_UID_LED_INDICATOR_ON,
	B_HID_UID_LED_INDICATOR_FLASH,
	B_HID_UID_LED_INDICATOR_SLOW_BLINK,
	B_HID_UID_LED_INDICATOR_FAST_BLINK,
	B_HID_UID_LED_INDICATOR_OFF,
	B_HID_UID_LED_FLASH_ON_TIME,
	B_HID_UID_LED_SLOW_BLINK_ON_TIME,
	B_HID_UID_LED_SLOW_BLINK_OFF_TIME,
	B_HID_UID_LED_FAST_BLINK_ON_TIME,
	B_HID_UID_LED_FAST_BLINK_OFF_TIME,
	B_HID_UID_LED_USAGE_INDICATOR_COLOR,
	B_HID_UID_LED_USAGE_INDICATOR_RED,
	B_HID_UID_LED_USAGE_INDICATOR_GREEN,
	B_HID_UID_LED_USAGE_INDICATOR_AMBER,
	B_HID_UID_LED_GENERIC_INDICATOR,
	B_HID_UID_LED_SYSTEM_SUSPEND,
	B_HID_UID_LED_EXTERNAL_POWER_CONNECTED	
};


#endif // _USB_HID_PAGE_LED_H
