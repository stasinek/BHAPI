/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: SoundFile.cpp
 *  DESCR: 
 ***********************************************************************/
#include <MediaFile.h>
#include <MediaTrack.h>
#include <SoundFile.h>
#include <kits/debug/Debug.h>

/*************************************************************
 * public BSoundFile
 *************************************************************/

BSoundFile::BSoundFile()
{
	_init_raw_stats();
}


BSoundFile::BSoundFile(const entry_ref *ref,
					   uint32_t open_mode)
{
	_init_raw_stats();
	SetTo(ref,open_mode);
}

/* virtual */ 
BSoundFile::~BSoundFile()
{
	delete fSoundFile;
	delete fMediaFile;
		// fMediaTrack will be deleted by the BMediaFile destructor
}


status_t BSoundFile::InitCheck() const
{
	if (!fSoundFile) {
		return B_NO_INIT;
	}
	return fSoundFile->InitCheck();
}


status_t BSoundFile::SetTo(const entry_ref *ref,
				  uint32_t open_mode)
{
	if (fMediaTrack) {
		BMediaTrack * track = fMediaTrack;
		fMediaTrack = 0;
		fMediaFile->ReleaseTrack(track);
	}
	if (fMediaFile) {
		BMediaFile * file = fMediaFile;
		fMediaFile = 0;
		delete file;
	}		
	if (fSoundFile) {
		BFile * file = fSoundFile;
		fSoundFile = 0;
		delete file;
	}
	if (open_mode == B_READ_ONLY) {
		return _ref_to_file(ref);
	} else {
		UNIMPLEMENTED();
		return B_ERROR;
	}
}


int32_t BSoundFile::FileFormat() const
{
	return fFileFormat;
}


int32_t BSoundFile::SamplingRate() const
{
	return fSamplingRate;
}


int32_t BSoundFile::CountChannels() const
{
	return fChannelCount;
}


int32_t BSoundFile::SampleSize() const
{
	return fSampleSize;
}


int32_t BSoundFile::ByteOrder() const
{
	return fByteOrder;
}


int32_t BSoundFile::SampleFormat() const
{
	return fSampleFormat;
}


int32_t BSoundFile::FrameSize() const
{
	return fSampleSize * fChannelCount;
}


off_t
BSoundFile::CountFrames() const
{
	return fFrameCount;
}


bool BSoundFile::IsCompressed() const
{
	return fIsCompressed;
}


int32_t BSoundFile::CompressionType() const
{
	return fCompressionType;
}


char *
BSoundFile::CompressionName() const
{
	return fCompressionName;
}


/* virtual */ int32_t BSoundFile::SetFileFormat(int32_t format)
{
	fFileFormat = format;
	return fFileFormat;
}


/* virtual */ int32_t BSoundFile::SetSamplingRate(int32_t fps)
{
	fSamplingRate = fps;
	return fSamplingRate;
}


/* virtual */ int32_t BSoundFile::SetChannelCount(int32_t spf)
{
	fChannelCount = spf;
	return fChannelCount;
}


/* virtual */ int32_t BSoundFile::SetSampleSize(int32_t bps)
{
	fSampleSize = bps;
	return fSampleSize;
}


/* virtual */ int32_t BSoundFile::SetByteOrder(int32_t bord)
{
	fByteOrder = bord;
	return fByteOrder;
}


/* virtual */ int32_t BSoundFile::SetSampleFormat(int32_t fmt)
{
	fSampleFormat = fmt;
	return fSampleFormat;
}


/* virtual */ int32_t BSoundFile::SetCompressionType(int32_t type)
{
	return 0;
}


/* virtual */ char *
BSoundFile::SetCompressionName(char *name)
{
	return NULL;
}


/* virtual */ bool BSoundFile::SetIsCompressed(bool tf)
{
	return false;
}


/* virtual */ off_t
BSoundFile::SetDataLocation(off_t offset)
{
	UNIMPLEMENTED();

	return 0;
}


/* virtual */ off_t
BSoundFile::SetFrameCount(off_t count)
{
	fFrameCount = count;
	return fFrameCount;
}


size_t
BSoundFile::ReadFrames(char *buf,
					   size_t count)
{
	UNIMPLEMENTED();

	return 0;
}


size_t
BSoundFile::WriteFrames(char *buf,
						size_t count)
{
	UNIMPLEMENTED();

	return 0;
}


/* virtual */ off_t
BSoundFile::SeekToFrame(off_t n)
{
	UNIMPLEMENTED();

	return 0;
}


off_t
BSoundFile::FrameIndex() const
{
	return fFrameIndex;
}


off_t
BSoundFile::FramesRemaining() const
{
	return fFrameCount - FrameIndex();
}

/*************************************************************
 * private BSoundFile
 *************************************************************/


void BSoundFile::_ReservedSoundFile1() {}
void BSoundFile::_ReservedSoundFile2() {}
void BSoundFile::_ReservedSoundFile3() {}

void BSoundFile::_init_raw_stats()
{
	fSoundFile = 0;
	fMediaFile = 0;
	fMediaTrack = 0;	
	fFileFormat = B_UNKNOWN_FILE;
	fSamplingRate = 44100;
	fChannelCount = 2;
	fSampleSize = 2;
	fByteOrder = B_BIG_ENDIAN;
	fSampleFormat = B_LINEAR_SAMPLES;
	fFrameCount = 0;
	fFrameIndex = 0;
	fIsCompressed = false;
	fCompressionType = -1;
	fCompressionName = NULL;
}


status_t BSoundFile::_ref_to_file(const entry_ref *ref)
{
	status_t status;
	BFile * file = new BFile(ref, B_READ_ONLY);
	status = file->InitCheck();
	if (status != B_OK) {
		fSoundFile = file;
		return status;
	}	
	BMediaFile * media = new BMediaFile(file);
	status = media->InitCheck();
	if (status != B_OK) {
		delete media;
		delete file;
		return status;
	}
	media_file_format mfi;
	media->GetFileFormatInfo(&mfi);
	switch (mfi.family) {
		case B_AIFF_FORMAT_FAMILY: fFileFormat = B_AIFF_FILE; break;
		case B_WAV_FORMAT_FAMILY:  fFileFormat = B_WAVE_FILE; break;
		default:                fFileFormat = B_UNKNOWN_FILE; break;
	}
	int trackNum = 0;
	BMediaTrack * track = 0;
	media_format mf;
	while (trackNum < media->CountTracks()) {
		track = media->TrackAt(trackNum);
		status = track->EncodedFormat(&mf);
		if (status != B_OK) {
			media->ReleaseTrack(track);
			delete media;
			delete file;
			return status;
		}		
		if (mf.IsAudio()) {
			break;
		}
		media->ReleaseTrack(track);
		track = 0;
	}
	if (track == 0) {
		delete media;
		delete file;
		return B_ERROR;
	}
	media_raw_audio_format * raw = 0;
	if (mf.type == B_MEDIA_ENCODED_AUDIO) {
		raw = &mf.u.encoded_audio.output;
	}
	if (mf.type == B_MEDIA_RAW_AUDIO) {
		raw = &mf.u.raw_audio;
	}

	if (raw == NULL) {
		delete media;
		delete file;
		return B_ERROR;
	}

	fSamplingRate = (int)raw->frame_rate;
	fChannelCount = raw->channel_count;
	fSampleSize = raw->format & 0xf;
	fByteOrder = raw->byte_order;
	switch (raw->format) {
		case media_raw_audio_format::B_AUDIO_FLOAT: 
			fSampleFormat = B_FLOAT_SAMPLES;
			break;
		case media_raw_audio_format::B_AUDIO_INT:
		case media_raw_audio_format::B_AUDIO_SHORT: 
		case media_raw_audio_format::B_AUDIO_UCHAR:
		case media_raw_audio_format::B_AUDIO_CHAR:
			fSampleFormat = B_LINEAR_SAMPLES;
			break;
		default:
			fSampleFormat = B_UNDEFINED_SAMPLES;
	}
	fByteOffset = 0;
	fFrameCount = track->CountFrames();
	fFrameIndex = 0;
	if (mf.type == B_MEDIA_ENCODED_AUDIO) {
		fIsCompressed = true;
		fCompressionType = mf.u.encoded_audio.encoding;
	}
	fMediaFile = media;
	fMediaTrack = track;
	return B_OK;
}


