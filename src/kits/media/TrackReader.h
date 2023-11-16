/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: TrackReader.h
 *  DESCR: The undocumented BTrackReader class, 
 *         used by BSound and the GameSound classes
 ***********************************************************************/

#if !defined(_TRACK_READER_H_)
#define _TRACK_READER_H_

#include <MediaTrack.h>

class BMediaFile;

namespace BPrivate
{

class BTrackReader
{
public:
	BTrackReader(BMediaTrack *, media_raw_audio_format const &);
	BTrackReader(BFile *, media_raw_audio_format const &);
	~BTrackReader();

	status_t 	InitCheck();	
	int64 		CountFrames(void);
	int32_t 		FrameSize(void);
	status_t 	ReadFrames(void *in_buffer, int32_t frame_count);
	status_t 	SeekToFrame(int64 *in_out_frame);

	BMediaTrack * 					Track(void);
	const media_raw_audio_format & 	Format(void) const;
	
private:
	void SetToTrack(BMediaTrack *track);

private:
	int32_t	fFrameSize;
	uint8 *	fBuffer;
	int32_t 	fBufferOffset;
	int32_t	fBufferUsedSize;
	BMediaFile *fMediaFile;
	BMediaTrack *fMediaTrack;
	media_raw_audio_format fFormat;
};

}; //namespace BPrivate

#endif
