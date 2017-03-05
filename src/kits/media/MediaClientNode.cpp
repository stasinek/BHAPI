/*
 * Copyright 2015, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "MediaClientNode.h"

#include <MediaClient.h>
#include <MediaConnection.h>
#include <scheduler.h>
#include <TimeSource.h>

#include <string.h>

#include "debug.h"

#define B_NEW_BUFFER (BTimedEventQueue::B_USER_EVENT + 1)


BMediaClientNode::BMediaClientNode(const char* name,
	BMediaClient* owner, media_type type)
	:
	BMediaNode(name),
	BBufferConsumer(type),
	BBufferProducer(type),
	BMediaEventLooper(),
	fOwner(owner)
{
	CALLED();

	// Configure the node to do the requested jobs
	if (fOwner->Kind() & B_MEDIA_PLAYER)
		AddNodeKind(B_BUFFER_PRODUCER);
	if (fOwner->Kind() & B_MEDIA_RECORDER)
		AddNodeKind(B_BUFFER_CONSUMER);
	if (fOwner->Kind() & B_MEDIA_CONTROLLABLE)
		AddNodeKind(B_CONTROLLABLE);
}


status_t
BMediaClientNode::SendBuffer(BBuffer* buffer, BMediaConnection* conn)
{
	return BBufferProducer::SendBuffer(buffer, conn->Source(), conn->Destination());
}


BMediaAddOn*
BMediaClientNode::AddOn(int32* id) const
{
	CALLED();

	return fOwner->AddOn(id);
}


void
BMediaClientNode::NodeRegistered()
{
	CALLED();

	Run();
}


void
BMediaClientNode::SetRunMode(run_mode mode)
{
	CALLED();

	int32 priority;
	if (mode == BMediaNode::B_OFFLINE)
		priority = B_OFFLINE_PROCESSING;
	else {
		switch(ConsumerType()) {
			case B_MEDIA_RAW_AUDIO:
			case B_MEDIA_ENCODED_AUDIO:
				priority = B_AUDIO_RECORDING;
				break;

			case B_MEDIA_RAW_VIDEO:
			case B_MEDIA_ENCODED_VIDEO:
				priority = B_VIDEO_RECORDING;
				break;

			default:
				priority = B_DEFAULT_MEDIA_PRIORITY;
		}
	}

	SetPriority(suggest_thread_priority(priority));
	BMediaNode::SetRunMode(mode);
}


void
BMediaClientNode::Start(bigtime_t performanceTime)
{
	CALLED();

	BMediaEventLooper::Start(performanceTime);
	fOwner->fRunning = true;
}


void
BMediaClientNode::Stop(bigtime_t performanceTime, bool immediate)
{
	CALLED();

	BMediaEventLooper::Stop(performanceTime, immediate);
	fOwner->fRunning = false;
}


void
BMediaClientNode::Seek(bigtime_t mediaTime, bigtime_t performanceTime)
{
	CALLED();

	BMediaEventLooper::Seek(mediaTime, performanceTime);
}


void
BMediaClientNode::TimeWarp(bigtime_t realTime, bigtime_t performanceTime)
{
	CALLED();

	BMediaEventLooper::TimeWarp(realTime, performanceTime);
}


status_t
BMediaClientNode::HandleMessage(int32 message,
	const void* data, size_t size)
{
	CALLED();

	return B_ERROR;
}


status_t
BMediaClientNode::AcceptFormat(const media_destination& dest,
	media_format* format)
{
	CALLED();

	BMediaInput* conn = fOwner->FindInput(dest);
	if (conn == NULL)
		return B_MEDIA_BAD_DESTINATION;

	if (format_is_compatible(*format, conn->AcceptedFormat()))
		return B_OK;

	*format = conn->AcceptedFormat();

	return B_MEDIA_BAD_FORMAT;
}


status_t
BMediaClientNode::GetNextInput(int32* cookie,
	media_input* input)
{
	CALLED();

	if (fOwner->CountInputs() == 0)
		return B_BAD_INDEX;

	if (*cookie < 0 || *cookie >= fOwner->CountInputs()) {
		*cookie = -1;
		input = NULL;
	} else {
		BMediaInput* conn = fOwner->InputAt(*cookie);
		if (conn != NULL) {
			*input = conn->MediaInput();
			*cookie += 1;
			return B_OK;
		}
	}
	return B_BAD_INDEX;
}


void
BMediaClientNode::DisposeInputCookie(int32 cookie)
{
	CALLED();

}


void
BMediaClientNode::BufferReceived(BBuffer* buffer)
{
	CALLED();

	EventQueue()->AddEvent(media_timed_event(buffer->Header()->start_time,
		BTimedEventQueue::B_HANDLE_BUFFER, buffer,
		BTimedEventQueue::B_RECYCLE_BUFFER));
}


status_t
BMediaClientNode::GetLatencyFor(const media_destination& dest,
	bigtime_t* latency, media_node_id* timesource)
{
	CALLED();

	BMediaInput* conn = fOwner->FindInput(dest);
	if (conn == NULL)
		return B_MEDIA_BAD_DESTINATION;

	*latency = fOwner->fMaxLatency;
	*timesource = TimeSource()->ID();
	return B_OK;
}


status_t
BMediaClientNode::Connected(const media_source& source,
	const media_destination& dest, const media_format& format,
	media_input* outInput)
{
	CALLED();

	BMediaInput* conn = fOwner->FindInput(dest);
	if (conn == NULL)
		return B_MEDIA_BAD_DESTINATION;

	conn->fConnection.source = source;
	conn->SetAcceptedFormat(format);

	conn->Connected(format);

	*outInput = conn->MediaInput();
	return B_OK;
}


void
BMediaClientNode::Disconnected(const media_source& source,
	const media_destination& dest)
{
	CALLED();

	BMediaInput* conn = fOwner->FindInput(dest);
	if (conn == NULL)
		return;

	conn->Disconnected();
}


status_t
BMediaClientNode::FormatChanged(const media_source& source,
	const media_destination& dest,
	int32 tag, const media_format& format)
{
	CALLED();

	BMediaInput* conn = fOwner->FindInput(dest);
	if (conn == NULL)
		return B_MEDIA_BAD_DESTINATION;

	return conn->FormatChanged(format);
}


status_t
BMediaClientNode::FormatSuggestionRequested(media_type type,
	int32 quality, media_format* format)
{
	CALLED();

	if (type != ConsumerType()
			&& type != ProducerType()) {
		return B_MEDIA_BAD_FORMAT;
	}

	if (fOwner->fNotifyHook != NULL) {
		status_t result = B_OK;
		(*fOwner->fNotifyHook)(fOwner->fNotifyCookie,
			BMediaClient::B_FORMAT_SUGGESTION,
			type, quality, format, &result);
		return result;
	} else {
		// In that case we return just a very generic format.
		media_format outFormat;
		outFormat.type = fOwner->MediaType();
		*format = outFormat;
		return B_OK;
	}
}


status_t
BMediaClientNode::FormatProposal(const media_source& source,
	media_format* format)
{
	CALLED();

	BMediaOutput* conn = fOwner->FindOutput(source);
	if (conn == NULL)
		return B_MEDIA_BAD_DESTINATION;

	return conn->FormatProposal(format);
}


status_t
BMediaClientNode::FormatChangeRequested(const media_source& source,
	const media_destination& dest, media_format* format,
	int32* _deprecated_)
{
	CALLED();

	BMediaOutput* conn = fOwner->FindOutput(source);
	if (conn == NULL)
		return B_MEDIA_BAD_DESTINATION;

	return conn->FormatChangeRequested(format);
}


void
BMediaClientNode::LateNoticeReceived(const media_source& source,
	bigtime_t late, bigtime_t when)
{
	CALLED();

}


status_t
BMediaClientNode::GetNextOutput(int32* cookie, media_output* output)
{
	CALLED();

	if (fOwner->CountOutputs() == 0)
		return B_BAD_INDEX;

	if (*cookie < 0 || *cookie >= fOwner->CountOutputs()) {
		*cookie = -1;
		output = NULL;
	} else {
		BMediaOutput* conn = fOwner->OutputAt(*cookie);
		if (conn != NULL) {
			*output = conn->MediaOutput();
			*cookie += 1;
			return B_OK;
		}
	}
	return B_BAD_INDEX;
}


status_t
BMediaClientNode::DisposeOutputCookie(int32 cookie)
{
	CALLED();

	return B_OK;
}


status_t
BMediaClientNode::SetBufferGroup(const media_source& source, BBufferGroup* group)
{
	CALLED();

	BMediaOutput* conn = fOwner->FindOutput(source);
	if (conn == NULL)
		return B_MEDIA_BAD_SOURCE;

	if (group == conn->fBufferGroup)
		return B_OK;

	delete conn->fBufferGroup;

	if (group != NULL) {
		conn->fBufferGroup = group;
		return B_OK;
	}

	bigtime_t latency = 0;
	GetLatency(&latency);
	int32 count = int32(latency / conn->BufferDuration() + 2);

	if (count < 3)
		count = 3;

	conn->fBufferGroup = new BBufferGroup(conn->BufferSize(), count);
	if (conn->fBufferGroup == NULL)
		return B_NO_MEMORY;

	return conn->fBufferGroup->InitCheck();
}


status_t
BMediaClientNode::PrepareToConnect(const media_source& source,
	const media_destination& dest, media_format* format,
	media_source* out_source, char *name)
{
	CALLED();

	BMediaOutput* conn = fOwner->FindOutput(source);
	if (conn == NULL)
		return B_MEDIA_BAD_SOURCE;

	if (conn->Destination() != media_destination::null)
		return B_MEDIA_ALREADY_CONNECTED;

	if (fOwner->MediaType() != B_MEDIA_UNKNOWN_TYPE
			&& format->type != fOwner->MediaType()) {
		return B_MEDIA_BAD_FORMAT;
	}

	conn->fConnection.destination = dest;

	status_t err = conn->PrepareToConnect(format);
	if (err != B_OK)
		return err;

	*out_source = conn->Source();
	strcpy(name, Name());

	return B_OK;
}


void
BMediaClientNode::Connect(status_t status, const media_source& source,
	const media_destination& dest, const media_format& format,
	char* name)
{
	CALLED();

	BMediaOutput* conn = fOwner->FindOutput(source);
	if (conn == NULL)
		return;

	// Reset the connection to reuse it
	if (status != B_OK) {
		conn->Disconnect();
		return;
	}

	conn->fConnection.destination = dest;
	conn->SetAcceptedFormat(format);
	strcpy(name, Name());

	// TODO: Allocate buffers, add correct latency estimate
	// and buffer duration mode.

	conn->Connected(format);
}


void
BMediaClientNode::Disconnect(const media_source& source,
	const media_destination& dest)
{
	CALLED();

	BMediaOutput* conn = fOwner->FindOutput(source);
	if (conn == NULL)
		return;

	if (conn->Destination() == dest) {
		conn->Disconnect();
		conn->Disconnected();
	}
}


void
BMediaClientNode::EnableOutput(const media_source& source,
	bool enabled, int32* _deprecated_)
{
	CALLED();

	BMediaOutput* conn = fOwner->FindOutput(source);
	if (conn != NULL) {
		conn->fOutputEnabled = enabled;
		return;
	}
}


status_t
BMediaClientNode::GetLatency(bigtime_t* outLatency)
{
	CALLED();

	// TODO: finish latency handling
	*outLatency = fOwner->fMinLatency;
	return B_OK;
}


void
BMediaClientNode::LatencyChanged(const media_source& source,
	const media_destination& dest, bigtime_t latency, uint32 flags)
{
	CALLED();
}


void
BMediaClientNode::ProducerDataStatus(const media_destination& dest,
	int32 status, bigtime_t when)
{
	CALLED();
}


void
BMediaClientNode::HandleEvent(const media_timed_event* event,
	bigtime_t late, bool realTimeEvent)
{
	CALLED();

	switch (event->type) {
		// This event is used for inputs which consumes buffers
		// or binded connections which also sent them to an output.
		case BTimedEventQueue::B_HANDLE_BUFFER:
			_HandleBuffer((BBuffer*)event->pointer);
			break;

		// This is used for connections which produce buffers only.
		case B_NEW_BUFFER:
			_ProduceNewBuffer(event, late);
			break;

		case BTimedEventQueue::B_START:
		{
			if (RunState() != B_STARTED)
				_HandleStart(event->event_time);
			break;
		}

		case BTimedEventQueue::B_STOP:
			_HandleStop(event->event_time, true);
			break;

		case BTimedEventQueue::B_SEEK:
			_HandleSeek(event->event_time, event->bigdata);
			break;

		case BTimedEventQueue::B_WARP:
			_HandleTimeWarp(event->event_time, event->bigdata);
			break;
	}
}


BMediaClientNode::~BMediaClientNode()
{
	CALLED();

	Quit();
}


void
BMediaClientNode::_HandleBuffer(BBuffer* buffer)
{
	CALLED();

	media_destination dest;
	dest.id = buffer->Header()->destination;
	BMediaInput* conn = fOwner->FindInput(dest);

	if (conn != NULL)
		conn->BufferReceived(buffer);
	// TODO: this should be logged someway
}


void
BMediaClientNode::_ProduceNewBuffer(const media_timed_event* event,
	bigtime_t late)
{
	CALLED();

	if (RunState() != BMediaEventLooper::B_STARTED)
		return;

	// We get the data through the event
	// so that we know which connection

	// event.pointer == connection
}


void
BMediaClientNode::_HandleStart(bigtime_t performanceTime)
{
	CALLED();

	if (fOwner->fNotifyHook != NULL) {
		(*fOwner->fNotifyHook)(fOwner->fNotifyCookie,
			BMediaClient::B_WILL_START,
			performanceTime);
	}
}


void
BMediaClientNode::_HandleStop(bigtime_t performanceTime,
	bool immediate)
{
	CALLED();

	if (fOwner->fNotifyHook != NULL) {
		(*fOwner->fNotifyHook)(fOwner->fNotifyCookie,
			BMediaClient::B_WILL_STOP,
			performanceTime, immediate);
	}
	EventQueue()->FlushEvents(0, BTimedEventQueue::B_ALWAYS, true,
		BTimedEventQueue::B_HANDLE_BUFFER);
}


void
BMediaClientNode::_HandleSeek(bigtime_t mediaTime,
	bigtime_t performanceTime)
{
	CALLED();

	if (fOwner->fNotifyHook != NULL) {
		(*fOwner->fNotifyHook)(fOwner->fNotifyCookie,
			BMediaClient::B_WILL_SEEK,
			performanceTime, mediaTime);
	}
}


void
BMediaClientNode::_HandleTimeWarp(bigtime_t realTime,
	bigtime_t performanceTime)
{
	CALLED();

	if (fOwner->fNotifyHook != NULL) {
		(*fOwner->fNotifyHook)(fOwner->fNotifyCookie,
			BMediaClient::B_WILL_TIMEWARP,
			realTime, performanceTime);
	}
}
