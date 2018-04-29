﻿/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * BHAPI++ library is a freeware; it may be used and distributed according to
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
 * File: Errors.h
 *
 * --------------------------------------------------------------------------*/
#ifndef BHAPI_ERRORS_H
#define BHAPI_ERRORS_H
#include <Haiku.h>
//-------------------------------------------------------------------------------------------------
/* Error baselines */
#define B_GENERAL_ERROR_BASE		INT_MIN
#define B_OS_ERROR_BASE				(B_GENERAL_ERROR_BASE + 0x1000)
#define B_APP_ERROR_BASE			(B_GENERAL_ERROR_BASE + 0x2000)
#define B_INTERFACE_ERROR_BASE		(B_GENERAL_ERROR_BASE + 0x3000)
#define B_MEDIA_ERROR_BASE			(B_GENERAL_ERROR_BASE + 0x4000)
                                        /* - 0x41ff */
#define B_TRANSLATION_ERROR_BASE	(B_GENERAL_ERROR_BASE + 0x4800)
                                        /* - 0x48ff */
#define B_MIDI_ERROR_BASE			(B_GENERAL_ERROR_BASE + 0x5000)
#define B_STORAGE_ERROR_BASE		(B_GENERAL_ERROR_BASE + 0x6000)
#define B_POSIX_ERROR_BASE			(B_GENERAL_ERROR_BASE + 0x7000)
#define B_MAIL_ERROR_BASE			(B_GENERAL_ERROR_BASE + 0x8000)
#define B_PRINT_ERROR_BASE			(B_GENERAL_ERROR_BASE + 0x9000)
#define B_DEVICE_ERROR_BASE			(B_GENERAL_ERROR_BASE + 0xa000)
//-------------------------------------------------------------------------------------------------
/* Developer-defined errors start at (B_ERRORS_END+1) */
#define B_ERRORS_END				(B_GENERAL_ERROR_BASE + 0xffff)
//-------------------------------------------------------------------------------------------------
/* General Errors */
#define B_NO_MEMORY					(B_GENERAL_ERROR_BASE + 0)
#define B_IO_ERROR					(B_GENERAL_ERROR_BASE + 1)
#define B_PERMISSION_DENIED			(B_GENERAL_ERROR_BASE + 2)
#define B_BAD_INDEX					(B_GENERAL_ERROR_BASE + 3)
#define B_BAD_TYPE					(B_GENERAL_ERROR_BASE + 4)
#define B_BAD_VALUE					(B_GENERAL_ERROR_BASE + 5)
#define B_MISMATCHED_VALUES			(B_GENERAL_ERROR_BASE + 6)
#define B_NAME_NOT_FOUND			(B_GENERAL_ERROR_BASE + 7)
#define B_NAME_IN_USE				(B_GENERAL_ERROR_BASE + 8)
#define B_TIMED_OUT					(B_GENERAL_ERROR_BASE + 9)
#define B_INTERRUPTED				(B_GENERAL_ERROR_BASE + 10)
#define B_WOULD_BLOCK				(B_GENERAL_ERROR_BASE + 11)
#define B_CANCELED					(B_GENERAL_ERROR_BASE + 12)
#define B_NO_INIT					(B_GENERAL_ERROR_BASE + 13)
#define B_NOT_INITIALIZED			(B_GENERAL_ERROR_BASE + 13)
#define B_BUSY						(B_GENERAL_ERROR_BASE + 14)
#define B_NOT_ALLOWED				(B_GENERAL_ERROR_BASE + 15)
#define B_BAD_DATA					(B_GENERAL_ERROR_BASE + 16)
#define B_DONT_DO_THAT				(B_GENERAL_ERROR_BASE + 17)

#define B_ERROR						(-1)
#define B_OK						((int)0)
#define B_NO_ERROR					((int)0)
//-------------------------------------------------------------------------------------------------
/* Kernel Kit Errors */
#define B_BAD_SEM_ID				(B_OS_ERROR_BASE + 0)
#define B_NO_MORE_SEMS				(B_OS_ERROR_BASE + 1)

#define B_BAD_THREAD_ID				(B_OS_ERROR_BASE + 0x100)
#define B_NO_MORE_THREADS			(B_OS_ERROR_BASE + 0x101)
#define B_BAD_THREAD_STATE			(B_OS_ERROR_BASE + 0x102)
#define B_BAD_TEAM_ID				(B_OS_ERROR_BASE + 0x103)
#define B_NO_MORE_TEAMS				(B_OS_ERROR_BASE + 0x104)

#define B_BAD_PORT_ID				(B_OS_ERROR_BASE + 0x200)
#define B_NO_MORE_PORTS				(B_OS_ERROR_BASE + 0x201)

#define B_BAD_IMAGE_ID				(B_OS_ERROR_BASE + 0x300)
#define B_BAD_ADDRESS				(B_OS_ERROR_BASE + 0x301)
#define B_NOT_AN_EXECUTABLE			(B_OS_ERROR_BASE + 0x302)
#define B_MISSING_LIBRARY			(B_OS_ERROR_BASE + 0x303)
#define B_MISSING_SYMBOL			(B_OS_ERROR_BASE + 0x304)
#define B_UNKNOWN_EXECUTABLE		(B_OS_ERROR_BASE + 0x305)
#define B_LEGACY_EXECUTABLE			(B_OS_ERROR_BASE + 0x306)

#define B_DEBUGGER_ALREADY_INSTALLED	(B_OS_ERROR_BASE + 0x400)
//-------------------------------------------------------------------------------------------------
/* Application Kit Errors */
#define B_BAD_REPLY							(B_APP_ERROR_BASE + 0)
#define B_DUPLICATE_REPLY					(B_APP_ERROR_BASE + 1)
#define B_MESSAGE_TO_SELF					(B_APP_ERROR_BASE + 2)
#define B_BAD_HANDLER						(B_APP_ERROR_BASE + 3)
#define B_ALREADY_RUNNING					(B_APP_ERROR_BASE + 4)
#define B_LAUNCH_FAILED						(B_APP_ERROR_BASE + 5)
#define B_AMBIGUOUS_APP_LAUNCH				(B_APP_ERROR_BASE + 6)
#define B_UNKNOWN_MIME_TYPE					(B_APP_ERROR_BASE + 7)
#define B_BAD_SCRIPT_SYNTAX					(B_APP_ERROR_BASE + 8)
#define B_LAUNCH_FAILED_NO_RESOLVE_LINK		(B_APP_ERROR_BASE + 9)
#define B_LAUNCH_FAILED_EXECUTABLE			(B_APP_ERROR_BASE + 10)
#define B_LAUNCH_FAILED_APP_NOT_FOUND		(B_APP_ERROR_BASE + 11)
#define B_LAUNCH_FAILED_APP_IN_TRASH		(B_APP_ERROR_BASE + 12)
#define B_LAUNCH_FAILED_NO_PREFERRED_APP	(B_APP_ERROR_BASE + 13)
#define B_LAUNCH_FAILED_FILES_APP_NOT_FOUND	(B_APP_ERROR_BASE + 14)
#define B_BAD_MIME_SNIFFER_RULE				(B_APP_ERROR_BASE + 15)
#define B_NOT_A_MESSAGE						(B_APP_ERROR_BASE + 16)
#define B_SHUTDOWN_CANCELLED				(B_APP_ERROR_BASE + 17)
#define B_SHUTTING_DOWN						(B_APP_ERROR_BASE + 18)
//-------------------------------------------------------------------------------------------------
/* Storage Kit/File System Errors */
#define B_FILE_ERROR						(B_STORAGE_ERROR_BASE + 0)
#define B_FILE_NOT_FOUND					(B_STORAGE_ERROR_BASE + 1)
/* deprecated: use B_ENTRY_NOT_FOUND instead */
#define B_FILE_EXISTS						(B_STORAGE_ERROR_BASE + 2)
#define B_ENTRY_NOT_FOUND					(B_STORAGE_ERROR_BASE + 3)
#define B_NAME_TOO_LONG						(B_STORAGE_ERROR_BASE + 4)
#define B_NOT_A_DIRECTORY					(B_STORAGE_ERROR_BASE + 5)
#define B_DIRECTORY_NOT_EMPTY				(B_STORAGE_ERROR_BASE + 6)
#define B_DEVICE_FULL						(B_STORAGE_ERROR_BASE + 7)
#define B_READ_ONLY_DEVICE					(B_STORAGE_ERROR_BASE + 8)
#define B_IS_A_DIRECTORY					(B_STORAGE_ERROR_BASE + 9)
#define B_NO_MORE_FDS						(B_STORAGE_ERROR_BASE + 10)
#define B_CROSS_DEVICE_LINK					(B_STORAGE_ERROR_BASE + 11)
#define B_LINK_LIMIT						(B_STORAGE_ERROR_BASE + 12)
#define B_BUSTED_PIPE						(B_STORAGE_ERROR_BASE + 13)
#define B_UNSUPPORTED						(B_STORAGE_ERROR_BASE + 14)
#define B_PARTITION_TOO_SMALL				(B_STORAGE_ERROR_BASE + 15)
#define B_PARTIAL_READ						(B_STORAGE_ERROR_BASE + 16)
#define B_PARTIAL_WRITE						(B_STORAGE_ERROR_BASE + 17)
//-------------------------------------------------------------------------------------------------
/* POSIX Errors */
#ifdef B_USE_POSITIVE_POSIX_ERRORS
#	define B_TO_POSIX_ERROR(error)		(-(error))
#	define B_FROM_POSIX_ERROR(error)	(-(error))
#else
#	define B_TO_POSIX_ERROR(error)		(error)
#	define B_FROM_POSIX_ERROR(error)	(error)
#endif

#define B_POSIX_ENOMEM	B_TO_POSIX_ERROR(B_POSIX_ERROR_BASE + 0)
#define EFPOS			B_TO_POSIX_ERROR(B_POSIX_ERROR_BASE + 14)
#define ESIGPARM		B_TO_POSIX_ERROR(B_POSIX_ERROR_BASE + 15)
#define EPFNOSUPPORT	B_TO_POSIX_ERROR(B_POSIX_ERROR_BASE + 20)
#define ESHUTDOWN		B_TO_POSIX_ERROR(B_POSIX_ERROR_BASE + 31)
#define ENOMSG          B_TO_POSIX_ERROR(B_POSIX_ERROR_BASE + 39)
#define ESTALE          B_TO_POSIX_ERROR(B_POSIX_ERROR_BASE + 40)
#define EHOSTDOWN		B_TO_POSIX_ERROR(B_POSIX_ERROR_BASE + 45)
#define	EBADMSG			B_TO_POSIX_ERROR(B_POSIX_ERROR_BASE + 46)
#define EDQUOT			B_TO_POSIX_ERROR(B_POSIX_ERROR_BASE + 49)
#define EIDRM			B_TO_POSIX_ERROR(B_POSIX_ERROR_BASE + 50)
#define EMULTIHOP		B_TO_POSIX_ERROR(B_POSIX_ERROR_BASE + 51)
#define ENODATA			B_TO_POSIX_ERROR(B_POSIX_ERROR_BASE + 52)
#define ENOLINK			B_TO_POSIX_ERROR(B_POSIX_ERROR_BASE + 53)
#define ENOSR			B_TO_POSIX_ERROR(B_POSIX_ERROR_BASE + 54)
#define ENOSTR			B_TO_POSIX_ERROR(B_POSIX_ERROR_BASE + 55)
#define ETIME			B_TO_POSIX_ERROR(B_POSIX_ERROR_BASE + 58)
#define ETXTBSY			B_TO_POSIX_ERROR(B_POSIX_ERROR_BASE + 59)
#define ENOATTR			B_TO_POSIX_ERROR(B_POSIX_ERROR_BASE + 60)

/* New error codes that can be mapped to POSIX errors */
#define	B_BUFFER_OVERFLOW			B_FROM_POSIX_ERROR(EOVERFLOW)
#define B_TOO_MANY_ARGS				B_FROM_POSIX_ERROR(E2BIG)
#define	B_FILE_TOO_LARGE			B_FROM_POSIX_ERROR(EFBIG)
#define B_RESULT_NOT_REPRESENTABLE	B_FROM_POSIX_ERROR(ERANGE)
#define	B_DEVICE_NOT_FOUND			B_FROM_POSIX_ERROR(ENODEV)
#define B_NOT_SUPPORTED				B_FROM_POSIX_ERROR(EOPNOTSUPP)
//-------------------------------------------------------------------------------------------------
/* Media Kit Errors */
#define B_STREAM_NOT_FOUND				(B_MEDIA_ERROR_BASE + 0)
#define B_SERVER_NOT_FOUND				(B_MEDIA_ERROR_BASE + 1)
#define B_RESOURCE_NOT_FOUND			(B_MEDIA_ERROR_BASE + 2)
#define B_RESOURCE_UNAVAILABLE			(B_MEDIA_ERROR_BASE + 3)
#define B_BAD_SUBSCRIBER				(B_MEDIA_ERROR_BASE + 4)
#define B_SUBSCRIBER_NOT_ENTERED		(B_MEDIA_ERROR_BASE + 5)
#define B_BUFFER_NOT_AVAILABLE			(B_MEDIA_ERROR_BASE + 6)
#define B_LAST_BUFFER_ERROR				(B_MEDIA_ERROR_BASE + 7)

#define B_MEDIA_SYSTEM_FAILURE			(B_MEDIA_ERROR_BASE + 100)
#define B_MEDIA_BAD_NODE				(B_MEDIA_ERROR_BASE + 101)
#define B_MEDIA_NODE_BUSY				(B_MEDIA_ERROR_BASE + 102)
#define B_MEDIA_BAD_FORMAT				(B_MEDIA_ERROR_BASE + 103)
#define B_MEDIA_BAD_BUFFER				(B_MEDIA_ERROR_BASE + 104)
#define B_MEDIA_TOO_MANY_NODES			(B_MEDIA_ERROR_BASE + 105)
#define B_MEDIA_TOO_MANY_BUFFERS		(B_MEDIA_ERROR_BASE + 106)
#define B_MEDIA_NODE_ALREADY_EXISTS		(B_MEDIA_ERROR_BASE + 107)
#define B_MEDIA_BUFFER_ALREADY_EXISTS	(B_MEDIA_ERROR_BASE + 108)
#define B_MEDIA_CANNOT_SEEK				(B_MEDIA_ERROR_BASE + 109)
#define B_MEDIA_CANNOT_CHANGE_RUN_MODE	(B_MEDIA_ERROR_BASE + 110)
#define B_MEDIA_APP_ALREADY_REGISTERED	(B_MEDIA_ERROR_BASE + 111)
#define B_MEDIA_APP_NOT_REGISTERED		(B_MEDIA_ERROR_BASE + 112)
#define B_MEDIA_CANNOT_RECLAIM_BUFFERS	(B_MEDIA_ERROR_BASE + 113)
#define B_MEDIA_BUFFERS_NOT_RECLAIMED	(B_MEDIA_ERROR_BASE + 114)
#define B_MEDIA_TIME_SOURCE_STOPPED		(B_MEDIA_ERROR_BASE + 115)
#define B_MEDIA_TIME_SOURCE_BUSY		(B_MEDIA_ERROR_BASE + 116)
#define B_MEDIA_BAD_SOURCE				(B_MEDIA_ERROR_BASE + 117)
#define B_MEDIA_BAD_DESTINATION			(B_MEDIA_ERROR_BASE + 118)
#define B_MEDIA_ALREADY_CONNECTED		(B_MEDIA_ERROR_BASE + 119)
#define B_MEDIA_NOT_CONNECTED			(B_MEDIA_ERROR_BASE + 120)
#define B_MEDIA_BAD_CLIP_FORMAT			(B_MEDIA_ERROR_BASE + 121)
#define B_MEDIA_ADDON_FAILED			(B_MEDIA_ERROR_BASE + 122)
#define B_MEDIA_ADDON_DISABLED			(B_MEDIA_ERROR_BASE + 123)
#define B_MEDIA_CHANGE_IN_PROGRESS		(B_MEDIA_ERROR_BASE + 124)
#define B_MEDIA_STALE_CHANGE_COUNT		(B_MEDIA_ERROR_BASE + 125)
#define B_MEDIA_ADDON_RESTRICTED		(B_MEDIA_ERROR_BASE + 126)
#define B_MEDIA_NO_HANDLER				(B_MEDIA_ERROR_BASE + 127)
#define B_MEDIA_DUPLICATE_FORMAT		(B_MEDIA_ERROR_BASE + 128)
#define B_MEDIA_REALTIME_DISABLED		(B_MEDIA_ERROR_BASE + 129)
#define B_MEDIA_REALTIME_UNAVAILABLE	(B_MEDIA_ERROR_BASE + 130)
//-------------------------------------------------------------------------------------------------
/* Mail Kit Errors */
#define B_MAIL_NO_DAEMON				(B_MAIL_ERROR_BASE + 0)
#define B_MAIL_UNKNOWN_USER				(B_MAIL_ERROR_BASE + 1)
#define B_MAIL_WRONG_PASSWORD			(B_MAIL_ERROR_BASE + 2)
#define B_MAIL_UNKNOWN_HOST				(B_MAIL_ERROR_BASE + 3)
#define B_MAIL_ACCESS_ERROR				(B_MAIL_ERROR_BASE + 4)
#define B_MAIL_UNKNOWN_FIELD			(B_MAIL_ERROR_BASE + 5)
#define B_MAIL_NO_RECIPIENT				(B_MAIL_ERROR_BASE + 6)
#define B_MAIL_INVALID_MAIL				(B_MAIL_ERROR_BASE + 7)
//-------------------------------------------------------------------------------------------------
/* Printing Errors */
#define B_NO_PRINT_SERVER				(B_PRINT_ERROR_BASE + 0)
//-------------------------------------------------------------------------------------------------
/* Device Kit Errors */
#define B_DEV_INVALID_IOCTL				(B_DEVICE_ERROR_BASE + 0)
#define B_DEV_NO_MEMORY					(B_DEVICE_ERROR_BASE + 1)
#define B_DEV_BAD_DRIVE_NUM				(B_DEVICE_ERROR_BASE + 2)
#define B_DEV_NO_MEDIA					(B_DEVICE_ERROR_BASE + 3)
#define B_DEV_UNREADABLE				(B_DEVICE_ERROR_BASE + 4)
#define B_DEV_FORMAT_ERROR				(B_DEVICE_ERROR_BASE + 5)
#define B_DEV_TIMEOUT					(B_DEVICE_ERROR_BASE + 6)
#define B_DEV_RECALIBRATE_ERROR			(B_DEVICE_ERROR_BASE + 7)
#define B_DEV_SEEK_ERROR				(B_DEVICE_ERROR_BASE + 8)
#define B_DEV_ID_ERROR					(B_DEVICE_ERROR_BASE + 9)
#define B_DEV_READ_ERROR				(B_DEVICE_ERROR_BASE + 10)
#define B_DEV_WRITE_ERROR				(B_DEVICE_ERROR_BASE + 11)
#define B_DEV_NOT_READY					(B_DEVICE_ERROR_BASE + 12)
#define B_DEV_MEDIA_CHANGED				(B_DEVICE_ERROR_BASE + 13)
#define B_DEV_MEDIA_CHANGE_REQUESTED	(B_DEVICE_ERROR_BASE + 14)
#define B_DEV_RESOURCE_CONFLICT			(B_DEVICE_ERROR_BASE + 15)
#define B_DEV_CONFIGURATION_ERROR		(B_DEVICE_ERROR_BASE + 16)
#define B_DEV_DISABLED_BY_USER			(B_DEVICE_ERROR_BASE + 17)
#define B_DEV_DOOR_OPEN					(B_DEVICE_ERROR_BASE + 18)

#define B_DEV_INVALID_PIPE				(B_DEVICE_ERROR_BASE + 19)
#define B_DEV_CRC_ERROR					(B_DEVICE_ERROR_BASE + 20)
#define B_DEV_STALLED					(B_DEVICE_ERROR_BASE + 21)
#define B_DEV_BAD_PID					(B_DEVICE_ERROR_BASE + 22)
#define B_DEV_UNEXPECTED_PID			(B_DEVICE_ERROR_BASE + 23)
#define B_DEV_DATA_OVERRUN				(B_DEVICE_ERROR_BASE + 24)
#define B_DEV_DATA_UNDERRUN				(B_DEVICE_ERROR_BASE + 25)
#define B_DEV_FIFO_OVERRUN				(B_DEVICE_ERROR_BASE + 26)
#define B_DEV_FIFO_UNDERRUN				(B_DEVICE_ERROR_BASE + 27)
#define B_DEV_PENDING					(B_DEVICE_ERROR_BASE + 28)
#define B_DEV_MULTIPLE_ERRORS			(B_DEVICE_ERROR_BASE + 29)
#define B_DEV_TOO_LATE					(B_DEVICE_ERROR_BASE + 30)
//-------------------------------------------------------------------------------------------------
/* Translation Kit Errors */
#define B_TRANSLATION_BASE_ERROR		(B_TRANSLATION_ERROR_BASE + 0)
#define B_NO_TRANSLATOR					(B_TRANSLATION_ERROR_BASE + 1)
#define B_ILLEGAL_DATA					(B_TRANSLATION_ERROR_BASE + 2)

#define B_TO_POSITIVE_ERROR(error)	__to_positive_error(error)
#define B_TO_NEGATIVE_ERROR(error)	__to_negative_error(error)
//-------------------------------------------------------------------------------------------------
#ifdef __cplusplus
namespace bhapi {
#endif
BHAPI_IMPEXP int __to_positive_error(int error);
BHAPI_IMPEXP int __to_negative_error(int error);
#ifdef __cplusplus
}
#endif
//-------------------------------------------------------------------------------------------------
#endif /* BHAPI_ERRORS_H */
//-------------------------------------------------------------------------------------------------

