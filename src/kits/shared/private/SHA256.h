/*
 * Copyright 2008, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef SHA_256_H
#define SHA_256_H


#include <Haiku.h>


#define SHA_DIGEST_LENGTH	32


class SHA256 {
public:
								SHA256();
								~SHA256();

			void				Init();
			void				Update(const void* buffer, size_t size);
			const uint8*		Digest();
			size_t				DigestLength() const
									{ return SHA_DIGEST_LENGTH; }

private:
			void				_ProcessChunk();

private:
			uint32_t				fHash[8];
			uint32_t				fDigest[8];
			uint32_t				fBuffer[64];
			size_t				fBytesInBuffer;
			size_t				fMessageSize;
			bool				fDigested;
};


#endif	// SHA_256_H
