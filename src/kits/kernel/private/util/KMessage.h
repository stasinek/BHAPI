/*
 * Copyright 2005-2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef _KERNEL_UTIL_KMESSAGE_H
#define _KERNEL_UTIL_KMESSAGE_H


#include <kits/support/String.h>

#include <kits/kernel/OS.h>
#include <HaikuConstants.h>


#ifdef __cplusplus


class BMessage;


namespace BPrivate {

class KMessageField;
class MessageAdapter;


class KMessage {
public:
			enum {
				KMESSAGE_OWNS_BUFFER		= 0x01,
				KMESSAGE_INIT_FROM_BUFFER	= 0x02,
				KMESSAGE_CLONE_BUFFER		= 0x04,
				KMESSAGE_READ_ONLY			= 0x08,

				KMESSAGE_FLAG_MASK			= 0x07,
			};

public:
								KMessage();
								KMessage(uint32_t what);
								~KMessage();

			status_t			SetTo(uint32_t what, uint32_t flags = 0);
			status_t			SetTo(void* buffer, int32_t bufferSize,
									uint32_t what, uint32_t flags = 0);
			status_t			SetTo(const void* buffer, int32_t bufferSize = -1,
									uint32_t flags = 0);
									// KMESSAGE_INIT_FROM_BUFFER and
									// KMESSAGE_READ_ONLY are implied
			void				Unset();

			void				SetWhat(uint32_t what);
			uint32_t				What() const;

			const void*			Buffer() const;
			int32_t				BufferCapacity() const;
			int32_t				ContentSize() const;

			status_t			AddField(const char* name, type_code type,
									int32_t elementSize = -1,
									KMessageField* field = NULL);
			status_t			FindField(const char* name,
									KMessageField* field) const;
			status_t			FindField(const char* name, type_code type,
									KMessageField* field) const;
			status_t			GetNextField(KMessageField* field) const;
			bool				IsEmpty() const;

			status_t			AddData(const char* name, type_code type,
									const void* data, int32_t numBytes,
									bool isFixedSize = true);
			status_t			AddArray(const char* name, type_code type,
									const void* data, int32_t elementSize,
									int32_t elementCount);
	inline	status_t			AddBool(const char* name, bool value);
	inline	status_t			AddInt8(const char* name, int8 value);
	inline	status_t			AddInt16(const char* name, int16 value);
	inline	status_t			AddInt32(const char* name, int32_t value);
	inline	status_t			AddInt64(const char* name, int64 value);
	inline	status_t			AddPointer(const char* name, const void* value);
	inline	status_t			AddString(const char* name, const char* value);

			status_t			FindData(const char* name, type_code type,
									const void** data, int32_t* numBytes) const;
			status_t			FindData(const char* name, type_code type,
									int32_t index, const void** data,
									int32_t* numBytes) const;
	inline	status_t			FindBool(const char* name, bool* value) const;
	inline	status_t			FindBool(const char* name, int32_t index,
									bool* value) const;
	inline	status_t			FindInt8(const char* name, int8* value) const;
	inline	status_t			FindInt8(const char* name, int32_t index,
									int8* value) const;
	inline	status_t			FindInt16(const char* name, int16* value) const;
	inline	status_t			FindInt16(const char* name, int32_t index,
									int16* value) const;
	inline	status_t			FindInt32(const char* name, int32_t* value) const;
	inline	status_t			FindInt32(const char* name, int32_t index,
									int32_t* value) const;
	inline	status_t			FindInt64(const char* name, int64* value) const;
	inline	status_t			FindInt64(const char* name, int32_t index,
									int64* value) const;
	inline	status_t			FindPointer(const char* name,
									void** value) const;
	inline	status_t			FindPointer(const char* name, int32_t index,
									void** value) const;
	inline	status_t			FindString(const char* name,
									const char** value) const;
	inline	status_t			FindString(const char* name, int32_t index,
									const char** value) const;

	inline	bool				GetBool(const char* name,
									bool defaultValue) const;
	inline	bool				GetBool(const char* name, int32_t index,
									bool defaultValue) const;
	inline	int8				GetInt8(const char* name,
									int8 defaultValue) const;
	inline	int8				GetInt8(const char* name, int32_t index,
									int8 defaultValue) const;
	inline	int16				GetInt16(const char* name,
									int16 defaultValue) const;
	inline	int16				GetInt16(const char* name, int32_t index,
									int16 defaultValue) const;
	inline	int32_t				GetInt32(const char* name,
									int32_t defaultValue) const;
	inline	int32_t				GetInt32(const char* name, int32_t index,
									int32_t defaultValue) const;
	inline	int64				GetInt64(const char* name,
									int64 defaultValue) const;
	inline	int64				GetInt64(const char* name, int32_t index,
									int64 defaultValue) const;
	inline	void*				GetPointer(const char* name,
									const void* defaultValue) const;
	inline	void*				GetPointer(const char* name, int32_t index,
									const void* defaultValue) const;
	inline	const char*			GetString(const char* name,
									const char* defaultValue) const;
	inline	const char*			GetString(const char* name, int32_t index,
									const char* defaultValue) const;

	// fixed size fields only
			status_t			SetData(const char* name, type_code type,
									const void* data, int32_t numBytes);
	inline	status_t			SetBool(const char* name, bool value);
	inline	status_t			SetInt8(const char* name, int8 value);
	inline	status_t			SetInt16(const char* name, int16 value);
	inline	status_t			SetInt32(const char* name, int32_t value);
	inline	status_t			SetInt64(const char* name, int64 value);
	inline	status_t			SetPointer(const char* name, const void* value);

	// message delivery
			team_id				Sender() const;
			int32_t				TargetToken() const;
			port_id				ReplyPort() const;
			int32_t				ReplyToken() const;

			void				SetDeliveryInfo(int32_t targetToken,
									port_id replyPort, int32_t replyToken,
									team_id senderTeam);

			status_t			SendTo(port_id targetPort,
									int32_t targetToken = -1,
									port_id replyPort = -1,
									int32_t replyToken = -1,
									bigtime_t timeout = -1,
									team_id senderTeam = -1);
			status_t			SendTo(port_id targetPort, int32_t targetToken,
									KMessage* reply,
									bigtime_t deliveryTimeout = -1,
									bigtime_t replyTimeout = -1,
									team_id senderTeam = -1);
			status_t			SendReply(KMessage* message,
									port_id replyPort = -1,
									int32_t replyToken = -1,
									bigtime_t timeout = -1,
									team_id senderTeam = -1);
			status_t			SendReply(KMessage* message, KMessage* reply,
									bigtime_t deliveryTimeout = -1,
									bigtime_t replyTimeout = -1,
									team_id senderTeam = -1);
			status_t			ReceiveFrom(port_id fromPort,
									bigtime_t timeout = -1,
									port_message_info* messageInfo = NULL);

			void				Dump(void (*printFunc)(const char*, ...)) const;

private:
			friend class KMessageField;
			friend class MessageAdapter;
			friend class ::BMessage;	// not so nice, but makes things easier

			struct Header {
				uint32_t		magic;
				int32_t		size;
				uint32_t		what;
				team_id		sender;
				int32_t		targetToken;
				port_id		replyPort;
				int32_t		replyToken;
			};

			struct FieldHeader;
			struct FieldValueHeader;

private:
			Header*				_Header() const;
			int32_t				_BufferOffsetFor(const void* data) const;
			FieldHeader*		_FirstFieldHeader() const;
			FieldHeader*		_LastFieldHeader() const;
			FieldHeader*		_FieldHeaderForOffset(int32_t offset) const;
			status_t			_AddField(const char* name, type_code type,
									int32_t elementSize, KMessageField* field);
			status_t			_AddFieldData(KMessageField* field,
									const void* data, int32_t elementSize,
									int32_t elementCount);

			status_t			_InitFromBuffer(bool sizeFromBuffer);
			void				_InitBuffer(uint32_t what);

			void				_CheckBuffer();	// debugging only

			status_t			_AllocateSpace(int32_t size, bool alignAddress,
									bool alignSize, void** address,
									int32_t* alignedSize);
			int32_t				_CapacityFor(int32_t size);

	template<typename T>
	inline	status_t			_FindType(const char* name, type_code type,
									int32_t index, T* value) const;

	template<typename T>
	inline	T					_GetType(const char* name, type_code type,
									int32_t index, const T& defaultValue) const;

private:
			Header				fHeader;	// pointed to by fBuffer, if nothing
											// is allocated
			void*				fBuffer;
			int32_t				fBufferCapacity;
			uint32_t				fFlags;
			int32_t				fLastFieldOffset;

	static	const uint32_t		kMessageHeaderMagic;
};


class KMessageField {
public:
								KMessageField();

			void				Unset();

			KMessage*			Message() const;

			const char*			Name() const;
			type_code			TypeCode() const;
			bool				HasFixedElementSize() const;
			int32_t				ElementSize() const;
									// only if HasFixedElementSize()

			status_t			AddElement(const void* data, int32_t size = -1);
			status_t			AddElements(const void* data, int32_t count,
									int32_t elementSize = -1);
			const void*			ElementAt(int32_t index,
									int32_t* size = NULL) const;
			int32_t				CountElements() const;

private:
			friend class KMessage;

private:
			void				SetTo(KMessage* message, int32_t headerOffset);

			KMessage::FieldHeader* _Header() const;

private:
			KMessage*			fMessage;
			int32_t				fHeaderOffset;
};

} // namespace BPrivate


using BPrivate::KMessage;
using BPrivate::KMessageField;


// #pragma mark - inline functions


status_t KMessage::AddBool(const char* name, bool value)
{
	return AddData(name, B_BOOL_TYPE, &value, sizeof(bool), true);
}


status_t KMessage::AddInt8(const char* name, int8 value)
{
	return AddData(name, B_INT8_TYPE, &value, sizeof(int8), true);
}


status_t KMessage::AddInt16(const char* name, int16 value)
{
	return AddData(name, B_INT16_TYPE, &value, sizeof(int16), true);
}


status_t KMessage::AddInt32(const char* name, int32_t value)
{
	return AddData(name, B_INT32_TYPE, &value, sizeof(int32_t), true);
}


status_t KMessage::AddInt64(const char* name, int64 value)
{
	return AddData(name, B_INT64_TYPE, &value, sizeof(int64), true);
}


status_t KMessage::AddPointer(const char* name, const void* value)
{
	return AddData(name, B_POINTER_TYPE, &value, sizeof(value), true);
}


// AddString
status_t KMessage::AddString(const char* name, const char* value)
{
	if (!value)
		return B_BAD_VALUE;
	return AddData(name, B_STRING_TYPE, value, strlen(value) + 1, false);
}


// #pragma mark -


template<typename T>
status_t KMessage::_FindType(const char* name, type_code type, int32_t index,
	T* value) const
{
	const void* data;
	int32_t size;
	status_t error = FindData(name, type, index, &data, &size);
	if (error != B_OK)
		return error;

	if (size != sizeof(T))
		return B_BAD_DATA;

	*value = *(T*)data;

	return B_OK;
}


status_t KMessage::FindBool(const char* name, bool* value) const
{
	return FindBool(name, 0, value);
}


status_t KMessage::FindBool(const char* name, int32_t index, bool* value) const
{
	return _FindType(name, B_BOOL_TYPE, index, value);
}


status_t KMessage::FindInt8(const char* name, int8* value) const
{
	return FindInt8(name, 0, value);
}


status_t KMessage::FindInt8(const char* name, int32_t index, int8* value) const
{
	return _FindType(name, B_INT8_TYPE, index, value);
}


status_t KMessage::FindInt16(const char* name, int16* value) const
{
	return FindInt16(name, 0, value);
}


status_t KMessage::FindInt16(const char* name, int32_t index, int16* value) const
{
	return _FindType(name, B_INT16_TYPE, index, value);
}


status_t KMessage::FindInt32(const char* name, int32_t* value) const
{
	return FindInt32(name, 0, value);
}


status_t KMessage::FindInt32(const char* name, int32_t index, int32_t* value) const
{
	return _FindType(name, B_INT32_TYPE, index, value);
}


status_t KMessage::FindInt64(const char* name, int64* value) const
{
	return FindInt64(name, 0, value);
}


status_t KMessage::FindInt64(const char* name, int32_t index, int64* value) const
{
	return _FindType(name, B_INT64_TYPE, index, value);
}


status_t KMessage::FindPointer(const char* name, void** value) const
{
	return FindPointer(name, 0, value);
}


status_t KMessage::FindPointer(const char* name, int32_t index, void** value) const
{
	return _FindType(name, B_POINTER_TYPE, index, value);
}


status_t KMessage::FindString(const char* name, const char** value) const
{
	return FindString(name, 0, value);
}


status_t KMessage::FindString(const char* name, int32_t index, const char** value) const
{
	int32_t size;
	return FindData(name, B_STRING_TYPE, index, (const void**)value, &size);
}


template<typename T>
inline T
KMessage::_GetType(const char* name, type_code type, int32_t index,
	const T& defaultValue) const
{
	T value;
	if (_FindType(name, type, index, &value) == B_OK)
		return value;
	return defaultValue;
}


inline bool KMessage::GetBool(const char* name, bool defaultValue) const
{
	return _GetType(name, B_BOOL_TYPE, 0, defaultValue);
}


inline bool KMessage::GetBool(const char* name, int32_t index, bool defaultValue) const
{
	return _GetType(name, B_BOOL_TYPE, index, defaultValue);
}


int8
KMessage::GetInt8(const char* name, int8 defaultValue) const
{
	return _GetType(name, B_INT8_TYPE, 0, defaultValue);
}


int8
KMessage::GetInt8(const char* name, int32_t index, int8 defaultValue) const
{
	return _GetType(name, B_INT8_TYPE, index, defaultValue);
}


int16
KMessage::GetInt16(const char* name, int16 defaultValue) const
{
	return _GetType(name, B_INT16_TYPE, 0, defaultValue);
}


int16
KMessage::GetInt16(const char* name, int32_t index, int16 defaultValue) const
{
	return _GetType(name, B_INT16_TYPE, index, defaultValue);
}


int32_t KMessage::GetInt32(const char* name, int32_t defaultValue) const
{
	return _GetType(name, B_INT32_TYPE, 0, defaultValue);
}


int32_t KMessage::GetInt32(const char* name, int32_t index, int32_t defaultValue) const
{
	return _GetType(name, B_INT32_TYPE, index, defaultValue);
}


int64 KMessage::GetInt64(const char* name, int64 defaultValue) const
{
	return _GetType(name, B_INT64_TYPE, 0, defaultValue);
}


int64 KMessage::GetInt64(const char* name, int32_t index, int64 defaultValue) const
{
	return _GetType(name, B_INT64_TYPE, index, defaultValue);
}


void*   KMessage::GetPointer(const char* name, const void* defaultValue) const
{
	return const_cast<void*>(_GetType(name, B_POINTER_TYPE, 0, defaultValue));
}


void*   KMessage::GetPointer(const char* name, int32_t index,
	const void* defaultValue) const
{
	return const_cast<void*>(_GetType(name, B_POINTER_TYPE, index,
		defaultValue));
}


const char*  KMessage::GetString(const char* name, int32_t index,
	const char* defaultValue) const
{
	// don't use _GetType() here, since it checks field size == sizeof(T)
	int32_t size;
	const char* value;
	if (FindData(name, B_STRING_TYPE, index, (const void**)&value, &size)
			== B_OK) {
		return value;
	}
	return defaultValue;
}


const char*  KMessage::GetString(const char* name, const char* defaultValue) const
{
	return GetString(name, 0, defaultValue);
}


status_t KMessage::SetBool(const char* name, bool value)
{
	return SetData(name, B_BOOL_TYPE, &value, sizeof(bool));
}


status_t KMessage::SetInt8(const char* name, int8 value)
{
	return SetData(name, B_INT8_TYPE, &value, sizeof(int8));
}


status_t KMessage::SetInt16(const char* name, int16 value)
{
	return SetData(name, B_INT16_TYPE, &value, sizeof(int16));
}


status_t KMessage::SetInt32(const char* name, int32_t value)
{
	return SetData(name, B_INT32_TYPE, &value, sizeof(int32_t));
}


status_t KMessage::SetInt64(const char* name, int64 value)
{
	return SetData(name, B_INT64_TYPE, &value, sizeof(int64));
}


status_t KMessage::SetPointer(const char* name, const void* value)
{
	return SetData(name, B_POINTER_TYPE, &value, sizeof(value));
}


#else	// !__cplusplus


typedef struct KMessage {
	struct Header {
		uint32_t		magic;
		int32_t		size;
		uint32_t		what;
		team_id		sender;
		int32_t		targetToken;
		port_id		replyPort;
		int32_t		replyToken;
	}				fHeader;
	void*			fBuffer;
	int32_t			fBufferCapacity;
	uint32_t			fFlags;
	int32_t			fLastFieldOffset;
} KMessage;


#endif	// !__cplusplus


#endif	// KMESSAGE_H
