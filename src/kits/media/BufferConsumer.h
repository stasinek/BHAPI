/*
 * Copyright 2009, Haiku Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _BUFFER_CONSUMER_H
#define _BUFFER_CONSUMER_H


#include <MediaDefs.h>
#include <MediaNode.h>


class BBuffer;
class BBufferGroup;
class BRegion;

namespace BPrivate {
	class BufferCache;
	namespace media {
		class BMediaRosterEx;
	}
}


class BBufferConsumer : public virtual BMediaNode {
protected:
	virtual						~BBufferConsumer();

public:
			media_type			ConsumerType();

	static	status_t			RegionToClipData(const BRegion* region,
									int32_t* format, int32_t* size, void* data);

protected:
	explicit					BBufferConsumer(media_type type);

	static	void				NotifyLateProducer(
									const media_source& whatSource,
									bigtime_t howMuch,
									bigtime_t performanceTime);
			status_t			SetVideoClippingFor(const media_source& output,
									const media_destination& destination,
									const int16* shorts, int32_t shortCount,
									const media_video_display_info& display,
									void* userData, int32_t* changeTag,
									void* _reserved = NULL);
			status_t			SetOutputEnabled(const media_source& source,
									const media_destination& destination,
									bool enabled, void* userData,
									int32_t* changeTag, void* _reserved = NULL);
			status_t			RequestFormatChange(const media_source& source,
									const media_destination& destination,
									const media_format& toFormat,
									void* userData, int32_t* changeTag,
									void* _reserved = NULL);
			status_t			RequestAdditionalBuffer(
									const media_source& source,
									BBuffer* previousBuffer,
									void* _reserved = NULL);
			status_t			RequestAdditionalBuffer(
									const media_source& source,
									bigtime_t startTime,
									void* _reserved = NULL);
			status_t			SetOutputBuffersFor(const media_source& source,
									const media_destination& destination,
									BBufferGroup* group, void* userData,
									int32_t* changeTag, bool willReclaim = false,
									void* _reserved = NULL);
			status_t			SendLatencyChange(const media_source& source,
									const media_destination& destination,
									bigtime_t newLatency, uint32_t flags = 0);

protected:
	virtual	status_t			HandleMessage(int32_t message, const void* data,
									size_t size);

	virtual	status_t			AcceptFormat(
									const media_destination& destination,
									media_format* format) = 0;
	virtual	status_t			GetNextInput(int32_t* cookie,
									media_input* _input) = 0;
	virtual	void				DisposeInputCookie(int32_t cookie) = 0;
	virtual	void				BufferReceived(BBuffer* buffer) = 0;
	virtual	void				ProducerDataStatus(
									const media_destination& forWhom,
									int32_t status,
									bigtime_t atPerformanceTime) = 0;
	virtual	status_t			GetLatencyFor(const media_destination& forWhom,
									bigtime_t* _latency,
									media_node_id* _timesource) = 0;
	virtual	status_t			Connected(const media_source& producer,
									const media_destination& where,
									const media_format& withFormat,
									media_input* _input) = 0;
	virtual	void				Disconnected(const media_source& producer,
									const media_destination& where) = 0;
	virtual	status_t			FormatChanged(const media_source& producer,
									const media_destination& consumer,
									int32_t changeTag,
									const media_format& format) = 0;

	virtual	status_t			SeekTagRequested(
									const media_destination& destination,
									bigtime_t targetTime, uint32_t flags,
									media_seek_tag* _seekTag,
									bigtime_t* _taggedTime, uint32_t* _flags);

private:
	friend class BMediaNode;
	friend class BBufferProducer;
	friend class BMediaRoster;
	friend class BPrivate::media::BMediaRosterEx;

								BBufferConsumer();
								BBufferConsumer(const BBufferConsumer& other);
			BBufferConsumer&	operator=(const BBufferConsumer& other);

	// deprecated methods following
	static	status_t			SetVideoClippingFor(const media_source& output,
									const int16* shorts, int32_t shortCount,
									const media_video_display_info& display,
									int32_t* changeTag);
	static	status_t			RequestFormatChange(const media_source& source,
									const media_destination& destination,
									media_format* toFormat, int32_t* changeTag);
	static	status_t			SetOutputEnabled(const media_source& source,
									bool enabled, int32_t* changeTag);

			status_t			_Reserved_BufferConsumer_0(void*);
									// used for SeekTagRequested()
	virtual	status_t			_Reserved_BufferConsumer_1(void*);
	virtual	status_t			_Reserved_BufferConsumer_2(void*);
	virtual	status_t			_Reserved_BufferConsumer_3(void*);
	virtual	status_t			_Reserved_BufferConsumer_4(void*);
	virtual	status_t			_Reserved_BufferConsumer_5(void*);
	virtual	status_t			_Reserved_BufferConsumer_6(void*);
	virtual	status_t			_Reserved_BufferConsumer_7(void*);
	virtual	status_t			_Reserved_BufferConsumer_8(void*);
	virtual	status_t			_Reserved_BufferConsumer_9(void*);
	virtual	status_t			_Reserved_BufferConsumer_10(void*);
	virtual	status_t			_Reserved_BufferConsumer_11(void*);
	virtual	status_t			_Reserved_BufferConsumer_12(void*);
	virtual	status_t			_Reserved_BufferConsumer_13(void*);
	virtual	status_t			_Reserved_BufferConsumer_14(void*);
	virtual	status_t			_Reserved_BufferConsumer_15(void*);

private:
			media_type			fConsumerType;
			BPrivate::BufferCache* fBufferCache;
			BBufferGroup*		fDeleteBufferGroup;
			uint32_t				_reserved[14];
};


#endif	// _BUFFER_CONSUMER_H
