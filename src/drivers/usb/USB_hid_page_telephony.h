/*
 * Copyright 2004-2010, Haiku Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _USB_HID_PAGE_TELEPHONY_H
#define _USB_HID_PAGE_TELEPHONY_H


/* Reference:
 *		HID Usage Page 0x0B: TELEPHONY
 *		HID Usage Tables Ver. 1.12
 *		http://www.usb.org/developers/devclass_docs/Hut1_12.pdf
 */

// Usage IDs
enum {
	B_HID_UID_TEL_PHONE = 0x1,
	B_HID_UID_TEL_ANSWERING_MACHINE,
	B_HID_UID_TEL_MESSAGE_CONTROLS,
	B_HID_UID_TEL_HANDSET,
	B_HID_UID_TEL_HEADSET,
	B_HID_UID_TEL_TELEPHONY_KEY_PAD,
	B_HID_UID_TEL_PROGRAMMABLE_BUTTON,
	
	B_HID_UID_TEL_HOOK_SWITCH = 0x20,
	B_HID_UID_TEL_FLASH,
	B_HID_UID_TEL_FEATURE,
	B_HID_UID_TEL_HOLD,
	B_HID_UID_TEL_REDIAL,
	B_HID_UID_TEL_TRANSFER,
	B_HID_UID_TEL_DROP,
	B_HID_UID_TEL_PARK,
	B_HID_UID_TEL_FORWARD_CALLS,
	B_HID_UID_TEL_ALTERNATE_FUNCTION,
	B_HID_UID_TEL_LINE,
	B_HID_UID_TEL_SPEAKER_PHONE,
	B_HID_UID_TEL_CONFERENCE,
	B_HID_UID_TEL_RING_ENABLE,
	B_HID_UID_TEL_RING_SELECT,
	B_HID_UID_TEL_PHONE_MUTE,
	B_HID_UID_TEL_CALLER_ID,
	B_HID_UID_TEL_SEND,
	
	B_HID_UID_TEL_SPEED_DIAL = 0x50,
	B_HID_UID_TEL_STORE_NUMBER,
	B_HID_UID_TEL_RECALL_NUMBER,
	B_HID_UID_TEL_PHONE_DIRECTORY,

	B_HID_UID_TEL_VOICE_MAIL = 0x70,
	B_HID_UID_TEL_SCREEN_CALLS,
	B_HID_UID_TEL_DO_NOT_DISTURB,
	B_HID_UID_TEL_MESSAGE,
	B_HID_UID_TEL_ANSWER_ON_OFF,
	
	B_HID_UID_TEL_INSIDE_DIAL_TONE = 0x90,
	B_HID_UID_TEL_OUTSIDE_DIAL_TONE,
	B_HID_UID_TEL_INSIDE_RING_TONE,
	B_HID_UID_TEL_OUTSIDE_RING_TONE,
	B_HID_UID_TEL_PRIORITY_RING_TONE,
	B_HID_UID_TEL_INSIDE_RING_BACK,
	B_HID_UID_TEL_PRIORITY_RING_BACK,
	B_HID_UID_TEL_LINE_BUSY_TONE,
	B_HID_UID_TEL_REORDER_TONE,
	B_HID_UID_TEL_CALL_WAITING_TONE,
	B_HID_UID_TEL_CONFIRMATION_TONE_1,
	B_HID_UID_TEL_CONFIRMATION_TONE_2,
	B_HID_UID_TEL_TONES_OFF,
	B_HID_UID_TEL_OUTSIDE_RINGBACK,
	B_HID_UID_TEL_RINGER,
	
	B_HID_UID_TEL_PHONE_KEY_0 = 0xb0,
	B_HID_UID_TEL_PHONE_KEY_1,
	B_HID_UID_TEL_PHONE_KEY_2,
	B_HID_UID_TEL_PHONE_KEY_3,
	B_HID_UID_TEL_PHONE_KEY_4,
	B_HID_UID_TEL_PHONE_KEY_5,
	B_HID_UID_TEL_PHONE_KEY_6,
	B_HID_UID_TEL_PHONE_KEY_7,
	B_HID_UID_TEL_PHONE_KEY_8,
	B_HID_UID_TEL_PHONE_KEY_9,
	
	B_HID_UID_TEL_PHONE_KEY_STAR = 0xba,
	B_HID_UID_TEL_PHONE_KEY_POUND,
	B_HID_UID_TEL_PHONE_KEY_A,
	B_HID_UID_TEL_PHONE_KEY_B,
	B_HID_UID_TEL_PHONE_KEY_C,
	B_HID_UID_TEL_PHONE_KEY_D	
};


#endif // _USB_HID_PAGE_TELEPHONY_H
