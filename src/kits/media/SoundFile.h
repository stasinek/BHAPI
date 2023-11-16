/*
 * Copyright 2009, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _SOUND_FILE_H
#define _SOUND_FILE_H


#include <Entry.h>
#include <File.h>
#include <MediaDefs.h>


// file formats
enum {
	B_UNKNOWN_FILE,
	B_AIFF_FILE,
	B_WAVE_FILE,
	B_UNIX_FILE
};


class BSoundFile  {
public:
								BSoundFile();
								BSoundFile(const entry_ref* ref,
									uint32_t openMode);
	virtual						~BSoundFile();

			status_t			InitCheck() const;

			status_t			SetTo(const entry_ref* ref, uint32_t openMode);

			int32_t				FileFormat() const;
			int32_t				SamplingRate() const;
			int32_t				CountChannels() const;
			int32_t				SampleSize() const;
			int32_t				ByteOrder() const;
			int32_t				SampleFormat() const;
			int32_t				FrameSize() const;
			off_t				CountFrames() const;

			bool				IsCompressed() const;
			int32_t				CompressionType() const;
			char*				CompressionName() const;

	virtual	int32_t				SetFileFormat(int32_t format);
	virtual	int32_t				SetSamplingRate(int32_t fps);
	virtual	int32_t				SetChannelCount(int32_t samplesPerFrame);
	virtual	int32_t				SetSampleSize(int32_t bytesPerSample);
	virtual	int32_t				SetByteOrder(int32_t byteOrder);
	virtual	int32_t				SetSampleFormat(int32_t format);
	virtual	int32_t				SetCompressionType(int32_t type);
	virtual	char*				SetCompressionName(char* name);
	virtual	bool				SetIsCompressed(bool compressed);
	virtual	off_t				SetDataLocation(off_t offset);
	virtual	off_t				SetFrameCount(off_t count);

			size_t				ReadFrames(char* buffer, size_t count);
			size_t				WriteFrames(char* buffer, size_t count);
	virtual	off_t				SeekToFrame(off_t index);
			off_t 				FrameIndex() const;
			off_t				FramesRemaining() const;

			BFile*				fSoundFile;

private:

	virtual	void				_ReservedSoundFile1();
	virtual	void				_ReservedSoundFile2();
	virtual	void				_ReservedSoundFile3();

			void				_init_raw_stats();
			status_t			_ref_to_file(const entry_ref* ref);

			int32_t				fFileFormat;
			int32_t				fSamplingRate;
			int32_t 				fChannelCount;
			int32_t				fSampleSize;
			int32_t				fByteOrder;
			int32_t				fSampleFormat;

			off_t				fByteOffset;
									// offset to first sample

			off_t				fFrameCount;
			off_t				fFrameIndex;

			bool				fIsCompressed;
			int32_t				fCompressionType;
			char*				fCompressionName;
			status_t			fCStatus;
			BMediaFile*			fMediaFile;
			BMediaTrack*		fMediaTrack;

			uint32_t			_reserved[2];
};

#endif	// _SOUND_FILE_H
