#ifndef _MESSAGE_UTILS_H_
#define _MESSAGE_UTILS_H_

#include <kits/netByteOrder.h>
#include <kits/support/DataIO.h>
#include <kits/storage/Entry.h>
#include <kits/netMessage.h>
#include <Haiku.h>


namespace BPrivate {	// Only putting these here because Be did

status_t entry_ref_flatten(char* buffer, size_t* size, const entry_ref* ref);
status_t entry_ref_unflatten(entry_ref* ref, const char* buffer, size_t size);
status_t entry_ref_swap(char* buffer, size_t size);

uint32_t CalculateChecksum(const uint8 *buffer, int32_t size);

}	// namespace BPrivate


template<class T>
inline void byte_swap(T &/*data*/)
{
	// Specialize for data types which actually swap
}


inline void write_helper(BDataIO *stream, const void *data, size_t size)
{
	status_t error = stream->Write(data, size);
	if (error < B_OK)
		throw error;
}


class TReadHelper {
public:
							TReadHelper(BDataIO *stream)
								:	fStream(stream),
									fError(B_OK),
									fSwap(false)
							{
							}

							TReadHelper(BDataIO *stream, bool swap)
								:	fStream(stream),
									fError(B_OK),
									fSwap(swap)
							{
							}

		template<class T>
		inline void			operator()(T &data)
							{
								fError = fStream->Read((void *)&data, sizeof(T));
								if (fError > B_OK) {
									if (IsSwapping())
										byte_swap(data);
									return;
								}

								if (fError == 0)
									fError = B_ERROR;
								throw fError;
							}

		template<class T>
		inline void			operator()(T data, size_t len)
							{
								fError = fStream->Read((void *)data, len);
								if (fError >= B_OK)
									return;

								throw fError;
							}

		status_t			Status() const { return fError >= B_OK ? B_OK : fError; };

		void				SetSwap(bool yesNo) { fSwap = yesNo; };
		bool				IsSwapping() const { return fSwap; };

private:
		BDataIO				*fStream;
		status_t			fError;
		bool				fSwap;
};


class TChecksumHelper {
public:
							TChecksumHelper(uchar* buffer)
								:	fBuffer(buffer),
									fBufPtr(buffer)
							{
							}

		template<class T>
		inline void			Cache(const T &data)
							{
								*((T*)fBufPtr) = data;
								fBufPtr += sizeof (T);
							}

		int32_t    			CheckSum();

private:
		uchar				*fBuffer;
		uchar				*fBufPtr;
};


template<>
inline void byte_swap(double &data)
{
	data = __swap_double(data);
}


template<>
inline void byte_swap(float &data)
{
	data = __swap_float(data);
}


template<>
inline void byte_swap(int64 &data)
{
	data = __swap_int64(data);
}


template<>
inline void byte_swap(uint64 &data)
{
	data = __swap_int64(data);
}


template<>
inline void byte_swap(int32_t &data)
{
	data = __swap_int32_t(data);
}


template<>
inline void byte_swap(uint32_t &data)
{
	data = __swap_int32_t(data);
}


template<>
inline void byte_swap(int16 &data)
{
	data = __swap_int16(data);
}


template<>
inline void byte_swap(uint16 &data)
{
	data = __swap_int16(data);
}


template<>
inline void byte_swap(entry_ref &data)
{
	byte_swap(data.device);
	byte_swap(data.directory);
}

#endif	// _MESSAGE_UTILS_H_
