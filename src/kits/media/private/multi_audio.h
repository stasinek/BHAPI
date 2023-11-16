/* multi_audio.h */

/* Interface description for drivers implementing studio-level audio I/O with */
/* possible auxillary functions (transport, time code, etc). */
/* Copyright © 1998-1999 Be Incorporated. All rights reserved. */

/* This is the first release candidate for the API. Unless we hear feedback that */
/* forces a change before the end of August, we will try to stay binary compatible */
/* with this interface. */
/* Send feedback to trinity@be.com [1999-07-02] */

#if !defined(_MULTI_AUDIO_H)
#define _MULTI_AUDIO_H

#include <Drivers.h>

#ifndef ASSERT
#include <kits/debug/Debug.h>
#endif

#define B_MULTI_DRIVER_BASE (B_AUDIO_DRIVER_BASE+20)

enum {	/* open() modes */
	/*	O_RDONLY is 0, O_WRONLY is 1, O_RDWR is 2	*/
	B_MULTI_CONTROL = 3
};

/* ioctl codes */
enum {
	/* multi_description */
	B_MULTI_GET_DESCRIPTION = B_MULTI_DRIVER_BASE,
	/* multi_event_handling */
	B_MULTI_GET_EVENT_INFO,
	B_MULTI_SET_EVENT_INFO,
	B_MULTI_GET_EVENT,
	/* multi_channel_enable */
	B_MULTI_GET_ENABLED_CHANNELS,
	B_MULTI_SET_ENABLED_CHANNELS,
	/* multi_format_info */
	B_MULTI_GET_GLOBAL_FORMAT,
	B_MULTI_SET_GLOBAL_FORMAT,		/* always sets for all channels, always implemented */
	/* multi_channel_formats */
	B_MULTI_GET_CHANNEL_FORMATS,
	B_MULTI_SET_CHANNEL_FORMATS,	/* only implemented if possible */
	/* multi_mix_value_info */
	B_MULTI_GET_MIX,
	B_MULTI_SET_MIX,
	/* multi_mix_channel_info */
	B_MULTI_LIST_MIX_CHANNELS,
	/* multi_mix_control_info */
	B_MULTI_LIST_MIX_CONTROLS,
	/* multi_mix_connection_info */
	B_MULTI_LIST_MIX_CONNECTIONS,
	/* multi_buffer_list */
	B_MULTI_GET_BUFFERS,			/* Fill out the struct for the first time; doesn't start anything. */
	B_MULTI_SET_BUFFERS,			/* Set what buffers to use, if the driver supports soft buffers. */
	/* bigtime_t */
	B_MULTI_SET_START_TIME,			/* When to actually start */
	/* multi_buffer_info */
	B_MULTI_BUFFER_EXCHANGE,		/* stop and go are derived from this being called */
	B_MULTI_BUFFER_FORCE_STOP,		/* force stop of playback */
	/* extension protocol */
	B_MULTI_LIST_EXTENSIONS,		/* get a list of supported extensions */
	B_MULTI_GET_EXTENSION,			/* get the value of an extension (or return error if not supported) */
	B_MULTI_SET_EXTENSION,			/* set the value of an extension */
	/* multi_mode_list */
	B_MULTI_LIST_MODES,				/* get a list of possible modes (multi_mode_list * arg) */
	B_MULTI_GET_MODE,				/* get the current mode (int32_t * arg) */
	B_MULTI_SET_MODE				/* set a new mode (int32_t * arg) */
};

/* sample rate values */
/* various fixed sample rates we support (for hard-sync clocked values) */
#define B_SR_8000 0x1
#define B_SR_11025 0x2
#define B_SR_12000 0x4
#define B_SR_16000 0x8
#define B_SR_22050 0x10
#define B_SR_24000 0x20
#define B_SR_32000 0x40
#define B_SR_44100 0x80
#define B_SR_48000 0x100
#define B_SR_64000 0x200
#define B_SR_88200 0x400
#define B_SR_96000 0x800
#define B_SR_176400 0x1000
#define B_SR_192000 0x2000
#define B_SR_384000 0x4000
#define B_SR_1536000 0x10000
/* continuously variable sample rate (typically board-generated) */
#define B_SR_CVSR 0x10000000UL
/* sample rate parameter global to all channels (input and output rates respectively) */
#define B_SR_IS_GLOBAL 0x80000000UL
/* output sample rate locked to input sample rate (output_rates only; the common case!) */
#define B_SR_SAME_AS_INPUT 0x40000000UL


/* format values */
/* signed char */
#define B_FMT_8BIT_S 0x01
/* unsigned char -- this is special case */
#define B_FMT_8BIT_U 0x02
/* traditional 16 bit signed format (host endian) */
#define B_FMT_16BIT 0x10
/* left-adjusted in 32 bit signed word */
#define B_FMT_18BIT 0x20
#define B_FMT_20BIT 0x40
#define B_FMT_24BIT 0x100
#define B_FMT_32BIT 0x1000
/* 32-bit floating point, -1.0 to 1.0 */
#define B_FMT_FLOAT 0x20000
/* 64-bit floating point, -1.0 to 1.0 */
#define B_FMT_DOUBLE 0x40000
/* 80-bit floating point, -1.0 to 1.0 */
#define B_FMT_EXTENDED 0x80000
/* bit stream */
#define B_FMT_BITSTREAM 0x1000000
/* format parameter global to all channels (input and output formats respectively) */
#define B_FMT_IS_GLOBAL 0x80000000UL
/* output format locked to input format (output_formats) */
#define B_FMT_SAME_AS_INPUT 0x40000000UL

/*	possible sample lock sources */
#define B_MULTI_LOCK_INPUT_CHANNEL 0x0	/* lock_source_data is channel id */
#define B_MULTI_LOCK_INTERNAL 0x1
#define B_MULTI_LOCK_WORDCLOCK 0x2
#define B_MULTI_LOCK_SUPERCLOCK 0x4
#define B_MULTI_LOCK_LIGHTPIPE 0x8
#define B_MULTI_LOCK_VIDEO 0x10			/* or blackburst */
#define B_MULTI_LOCK_FIRST_CARD 0x20	/* if you have more than one card */
#define B_MULTI_LOCK_MTC 0x40
#define B_MULTI_LOCK_SPDIF 0x80

/* possible timecode sources */
#define B_MULTI_TIMECODE_MTC 0x1
#define B_MULTI_TIMECODE_VTC 0x2
#define B_MULTI_TIMECODE_SMPTE 0x4
#define B_MULTI_TIMECODE_SUPERCLOCK 0x8
#define B_MULTI_TIMECODE_FIREWIRE 0x10

/* interface_flags values */
/* Available functions on this device. */
#define B_MULTI_INTERFACE_PLAYBACK 0x1
#define B_MULTI_INTERFACE_RECORD 0x2
#define B_MULTI_INTERFACE_TRANSPORT 0x4
#define B_MULTI_INTERFACE_TIMECODE 0x8
/* "Soft" buffers means you can change the pointer values and the driver will still be happy. */
#define B_MULTI_INTERFACE_SOFT_PLAY_BUFFERS 0x10000
#define B_MULTI_INTERFACE_SOFT_REC_BUFFERS 0x20000
/* Whether the data stream is interrupted when changing channel enables. */
#define B_MULTI_INTERFACE_CLICKS_WHEN_ENABLING_OUTPUTS 0x40000
#define B_MULTI_INTERFACE_CLICKS_WHEN_ENABLING_INPUTS 0x80000

#define B_CURRENT_INTERFACE_VERSION 0x4502
#define B_MINIMUM_INTERFACE_VERSION 0x4502

typedef struct multi_description multi_description;
typedef struct multi_channel_info multi_channel_info;
struct multi_description {

	size_t			info_size;			/* sizeof(multi_description) */
	uint32_t			interface_version;	/* current version of interface that's implemented */
	uint32_t			interface_minimum;	/* minimum version required to understand driver */

	char			friendly_name[32];	/* name displayed to user (C string) */
	char			vendor_info[32];	/* name used internally by vendor (C string) */

	int32_t			output_channel_count;
	int32_t			input_channel_count;
	int32_t			output_bus_channel_count;
	int32_t			input_bus_channel_count;
	int32_t			aux_bus_channel_count;

	int32_t			request_channel_count;		/* how many channel_infos are there */
	multi_channel_info *
					channels;

	uint32_t			output_rates;
	uint32_t			input_rates;
	float			min_cvsr_rate;
	float			max_cvsr_rate;
	uint32_t			output_formats;
	uint32_t			input_formats;
	uint32_t			lock_sources;
	uint32_t			timecode_sources;

	uint32_t			interface_flags;
	bigtime_t		start_latency;		/* how much in advance driver needs SET_START_TIME */

	uint32_t			_reserved_[11];

	char			control_panel[64];	/* MIME type of control panel application */
};

#if !defined(_MEDIA_DEFS_H)	/* enum in MediaDefs.h */
/* designation values */
/* mono channels have no designation */
#define B_CHANNEL_LEFT 0x1
#define B_CHANNEL_RIGHT 0x2
#define B_CHANNEL_CENTER 0x4			/* 5.1+ or fake surround */
#define B_CHANNEL_SUB 0x8				/* 5.1+ */
#define B_CHANNEL_REARLEFT 0x10			/* quad surround or 5.1+ */
#define B_CHANNEL_REARRIGHT 0x20		/* quad surround or 5.1+ */
#define B_CHANNEL_FRONT_LEFT_CENTER 0x40
#define B_CHANNEL_FRONT_RIGHT_CENTER 0x80
#define B_CHANNEL_BACK_CENTER 0x100		/* 6.1 or fake surround */
#define B_CHANNEL_SIDE_LEFT 0x200
#define B_CHANNEL_SIDE_RIGHT 0x400
#define B_CHANNEL_TOP_CENTER 0x800
#define B_CHANNEL_TOP_FRONT_LEFT 0x1000
#define B_CHANNEL_TOP_FRONT_CENTER 0x2000
#define B_CHANNEL_TOP_FRONT_RIGHT 0x4000
#define B_CHANNEL_TOP_BACK_LEFT 0x8000
#define B_CHANNEL_TOP_BACK_CENTER 0x10000
#define B_CHANNEL_TOP_BACK_RIGHT 0x20000
#endif

#define B_CHANNEL_MONO_BUS 0x4000000
#define B_CHANNEL_STEREO_BUS 0x2000000		/* + left/right */
#define B_CHANNEL_SURROUND_BUS 0x1000000	/* multichannel */

/* If you have interactions where some inputs can not be used when some */
/* outputs are used, mark both inputs and outputs with this flag. */
#define B_CHANNEL_INTERACTION 0x80000000UL
/* If input channel #n is simplexed with output channel #n, they should both */
/* have this flag set (different from the previous flag, which is more vague). */
#define B_CHANNEL_SIMPLEX 0x40000000UL

/* connector values */
/* analog connectors */
#define B_CHANNEL_RCA 0x1
#define B_CHANNEL_XLR 0x2
#define B_CHANNEL_TRS 0x4
#define B_CHANNEL_QUARTER_INCH_MONO 0x8
#define B_CHANNEL_MINI_JACK_STEREO 0x10
#define B_CHANNEL_QUARTER_INCH_STEREO 0x20
#define B_CHANNEL_ANALOG_HEADER 0x100	/* internal on card */
#define B_CHANNEL_SNAKE 0x200			/* or D-sub */
/* digital connectors (stereo) */
#define B_CHANNEL_OPTICAL_SPDIF 0x1000
#define B_CHANNEL_COAX_SPDIF 0x2000
#define B_CHANNEL_COAX_EBU 0x4000
#define B_CHANNEL_XLR_EBU 0x8000
#define B_CHANNEL_TRS_EBU 0x10000
#define B_CHANNEL_SPDIF_HEADER 0x20000	/* internal on card */
/* multi-channel digital connectors */
#define B_CHANNEL_LIGHTPIPE 0x100000
#define B_CHANNEL_TDIF 0x200000
#define B_CHANNEL_FIREWIRE 0x400000
#define B_CHANNEL_USB 0x800000
/* If you have multiple output connectors, only one of which can */
/* be active at a time. */
#define B_CHANNEL_EXCLUSIVE_SELECTION 0x80000000UL


typedef enum {
	B_MULTI_NO_CHANNEL_KIND,
	B_MULTI_OUTPUT_CHANNEL = 0x1,
	B_MULTI_INPUT_CHANNEL = 0x2,
	B_MULTI_OUTPUT_BUS = 0x4,
	B_MULTI_INPUT_BUS = 0x8,
	B_MULTI_AUX_BUS = 0x10
} channel_kind;

struct multi_channel_info {
	int32_t			channel_id;
	channel_kind	kind;
	uint32_t			designations;
	uint32_t			connectors;
	uint32_t			_reserved_[4];
};


/* Constants */
#define B_MULTI_EVENT_MINMAX		16
/* Event flags/masks */
#define B_MULTI_EVENT_TRANSPORT 	0x40000000UL
#define B_MULTI_EVENT_HAS_TIMECODE 	0x80000000UL

/* possible transport events */
#define B_MULTI_EVENT_NONE			0x00000000UL
#define B_MULTI_EVENT_START 		0x40010000UL
#define B_MULTI_EVENT_LOCATION 		0x40020000UL		/* location when shuttling or locating */
#define B_MULTI_EVENT_SHUTTLING 	0x40040000UL
#define B_MULTI_EVENT_STOP 			0x40080000UL
#define B_MULTI_EVENT_RECORD 		0x40100000UL
#define B_MULTI_EVENT_PAUSE 		0x40200000UL
#define B_MULTI_EVENT_RUNNING 		0x40400000UL		/* location when running */
/* possible device events */
enum {
	B_MULTI_EVENT_STARTED				= 0x1,
	B_MULTI_EVENT_STOPPED				= 0x2,
	B_MULTI_EVENT_CHANNEL_FORMAT_CHANGED= 0x4,
	B_MULTI_EVENT_BUFFER_OVERRUN 		= 0x8,
	B_MULTI_EVENT_SIGNAL_LOST 			= 0x10,
	B_MULTI_EVENT_SIGNAL_DETECTED 		= 0x20, 
	B_MULTI_EVENT_CLOCK_LOST 			= 0x40,
	B_MULTI_EVENT_CLOCK_DETECTED 		= 0x80,
	B_MULTI_EVENT_NEW_MODE 				= 0x100,
	B_MULTI_EVENT_CONTROL_CHANGED 		= 0x200
};

typedef struct multi_get_event_info multi_get_event_info;
struct multi_get_event_info {
	size_t		info_size;		/* sizeof(multi_get_event_info) */
	uint32_t		supported_mask;	/* what events h/w supports */
	uint32_t		current_mask;	/* current driver value */
	uint32_t		queue_size;		/* current queue size */
	uint32_t		event_count;	/* number of events currently in queue*/
	uint32_t		_reserved[3];
};

typedef struct multi_set_event_info multi_set_event_info;
struct multi_set_event_info {
	size_t		info_size;		/* sizeof(multi_set_event_info) */
	uint32_t		in_mask;		/* what events to wait for */
	int32_t		semaphore;		/* semaphore app will wait on */
	uint32_t		queue_size;		/* minimum number of events to save */
	uint32_t		_reserved[4];
};

typedef struct multi_get_event multi_get_event;
struct multi_get_event {
	size_t		info_size;		/* sizeof(multi_get_event) */
	uint32_t		event;
	bigtime_t	timestamp;		/* real time at which event was received */
	int32_t		count;			/* used for configuration events */
	union {
			int32_t		channels[100];
			uint32_t		clocks;
			int32_t		mode;
			int32_t		controls[100];
			struct { /* transport event */
				float		out_rate;		/* what rate it's now playing at */
				int32_t		out_hours;		/* location at the time given */
				int32_t		out_minutes;
				int32_t		out_seconds;
				int32_t		out_frames;
			}transport;
			char			_reserved_[400];		
	#if defined(__cplusplus)
		};
	#else
		} u;
	#endif
	uint32_t	_reserved_1[10];
};

typedef struct multi_channel_enable multi_channel_enable;
struct multi_channel_enable {
	size_t			info_size;			/* sizeof(multi_channel_enable) */
	/* this must have bytes for all channels (see multi_description) */
	/* channel 0 is lowest bit of first byte */
	uchar *			enable_bits;

	uint32_t			lock_source;
	int32_t			lock_data;
	uint32_t			timecode_source;
	uint32_t *		connectors;			/* which connector(s) is/are active, per channel */
};

#include <stdio.h>

#if defined(__cplusplus)
	inline void B_SET_CHANNEL(void * bits, int channel, bool value)
	{
		ASSERT(channel>=0);
		(((uchar *)(bits))[((channel)&0x7fff)>>3] =
			(((uchar *)(bits))[((channel)&0x7fff)>>3] & ~(1<<((channel)&0x7))) |
			((value) ? (1<<((channel)&0x7)) : 0));
	}
	inline bool B_TEST_CHANNEL(const void * bits, int channel)
	{
		return ((((uchar *)(bits))[((channel)&0x7fff)>>3] >> ((channel)&0x7)) & 1);
	}
#else
	#define B_SET_CHANNEL(bits, channel, value) \
		ASSERT(channel>=0); \
		(((uchar *)(bits))[((channel)&0x7fff)>>3] = \
			(((uchar *)(bits))[((channel)&0x7fff)>>3] & ~(1<<((channel)&0x7))) | \
			((value) ? (1<<((channel)&0x7)) : 0))
	#define B_TEST_CHANNEL(bits, channel) \
		((((uchar *)(bits))[((channel)&0x7fff)>>3] >> ((channel)&0x7)) & 1)
#endif

typedef struct multi_channel_formats multi_channel_formats;
typedef struct multi_format_info multi_format_info;
typedef struct _multi_format _multi_format;

struct _multi_format {
	uint32_t			rate;
	float			cvsr;
	uint32_t			format;
	uint32_t			_reserved_[3];
};
enum {	/* timecode kinds */
	B_MULTI_NO_TIMECODE,
	B_MULTI_TIMECODE_30,			/* MIDI */
	B_MULTI_TIMECODE_30_DROP_2,		/* NTSC */
	B_MULTI_TIMECODE_30_DROP_4,		/* Brazil */
	B_MULTI_TIMECODE_25,			/* PAL */
	B_MULTI_TIMECODE_24				/* Film */
};
struct multi_format_info {
	size_t			info_size;			/* sizeof(multi_format_info) */
	bigtime_t		output_latency;
	bigtime_t		input_latency;
	int32_t			timecode_kind;
	uint32_t			_reserved_[7];
	_multi_format	input;
	_multi_format	output;
};
struct multi_channel_formats {
	size_t			info_size;			/* sizeof(multi_channel_formats) */
	int32_t			request_channel_count;
	int32_t			request_first_channel;
	int32_t			returned_channel_count;
	int32_t			timecode_kind;
	int32_t			_reserved_[4];
	_multi_format *
					channels;
	bigtime_t *		latencies;			/* DMA/hardware latencies; client calculates for buffers */
};


typedef struct multi_mix_value multi_mix_value;
struct multi_mix_value {
	int32_t			id;
	union {
		float			gain;
		uint32_t			mux;	/* bitmask of mux points */
		bool			enable;
		uint32_t			_reserved_[2];
#if defined(__cplusplus)
	};
#else
	} u;
#endif
	int32_t			ramp;
	uint32_t			_reserved_2[2];
};

typedef struct multi_mix_value_info multi_mix_value_info;
struct multi_mix_value_info {
	size_t			info_size;		/* sizeof(multi_mix_value_info) */
	int32_t			item_count;
	multi_mix_value *
					values;
	int32_t			at_frame;		/* time at which to start the change */
};

//	only one of these should be set
#define B_MULTI_MIX_JUNCTION 0x1
#define B_MULTI_MIX_GAIN 0x2
#define B_MULTI_MIX_MUX 0x4
#define B_MULTI_MIX_ENABLE 0x8
#define B_MULTI_MIX_KIND_MASK 0xffff
//	any combination of these can be set
#define B_MULTI_MIX_RAMP 0x10000

typedef struct multi_mix_control multi_mix_control;
struct multi_mix_control {
	int32_t			id;				/* unique for device -- not same id as any channel/bus ! */
	uint32_t			flags;			/* including kind */
	int32_t			master;			/* or 0 if it's not slaved */
	union {
		struct {
			float			min_gain;		/* dB */
			float			max_gain;		/* dB */
			float			granularity;	/* dB */
		}			gain;
		struct {
			uint32_t			_reserved;
		}			mux;
		struct {
			uint32_t			_reserved;
		}			enable;
		uint32_t		_reserved[12];
#if defined(__cplusplus)
	};
#else
	}				u;
#endif
	char			name[32];
};

typedef struct multi_mix_channel_info multi_mix_channel_info;
struct multi_mix_channel_info {
	size_t			info_size;		/* sizeof(multi_mix_channel_info) */
	int32_t			channel_count;
	int32_t *			channels;		/* allocated by caller, lists requested channels */
	int32_t			max_count;		/* in: control ids per channel */
	int32_t			actual_count;	/* out: actual max # controls for any individual requested channel */
	int32_t **		controls;
};

typedef struct multi_mix_control_info multi_mix_control_info;
struct multi_mix_control_info {
	size_t			info_size;		/* sizeof(multi_mix_control_info) */
	int32_t			control_count;	/* in: number of controls */
	multi_mix_control *
					controls;		/* allocated by caller, returns control description for each */
};

typedef struct multi_mix_connection multi_mix_connection;
struct multi_mix_connection {
	int32_t			from;
	int32_t			to;
	uint32_t			_reserved_[2];
};

typedef struct multi_mix_connection_info multi_mix_connection_info;
struct multi_mix_connection_info {
	size_t			info_size;
	int32_t			max_count;		/* in: available space */
	int32_t			actual_count;	/* out: actual count */
	multi_mix_connection *
					connections;	/* allocated by caller, returns connections */
};


/* possible flags values for what is available (in and out) */
#define B_MULTI_BUFFER_PLAYBACK 0x1
#define B_MULTI_BUFFER_RECORD 0x2
#define B_MULTI_BUFFER_METERING 0x4
#define B_MULTI_BUFFER_TIMECODE 0x40000

typedef struct multi_buffer_list multi_buffer_list;
typedef struct buffer_desc buffer_desc;
/* This struct is used to query the driver about what buffers it will use, */
/* and to tell it what buffers to use if it supports soft buffers. */
struct multi_buffer_list {

	size_t			info_size;				/* sizeof(multi_buffer_list) */
	uint32_t			flags;

	int32_t			request_playback_buffers;
	int32_t			request_playback_channels;
	uint32_t			request_playback_buffer_size;		/* frames per buffer */
	int32_t			return_playback_buffers;			/* playback_buffers[b][] */
	int32_t			return_playback_channels;			/* playback_buffers[][c] */
	uint32_t			return_playback_buffer_size;		/* frames */
	buffer_desc **	playback_buffers;
	void *			_reserved_1;

	int32_t			request_record_buffers;
	int32_t			request_record_channels;
	uint32_t			request_record_buffer_size;			/* frames per buffer */
	int32_t			return_record_buffers;
	int32_t			return_record_channels;
	uint32_t			return_record_buffer_size;			/* frames */
	buffer_desc **	record_buffers;
	void *			_reserved_2;

};

struct buffer_desc {
	char *			base;		/* pointer to first sample for channel for buffer */
	size_t			stride;		/* offset to next sample */
	uint32_t			_reserved_[2];
};


/* This struct is used when actually queuing data to be played, and/or */
/* receiving data from a recorder. */
typedef struct multi_buffer_info multi_buffer_info;
struct multi_buffer_info {

	size_t			info_size;			/* sizeof(multi_buffer_info) */
	uint32_t			flags;

	bigtime_t		played_real_time;
	bigtime_t		played_frames_count;
	int32_t			_reserved_0;
	int32_t			playback_buffer_cycle;

	bigtime_t		recorded_real_time;
	bigtime_t		recorded_frames_count;
	int32_t			_reserved_1;
	int32_t			record_buffer_cycle;

	int32_t			meter_channel_count;
	char *			meters_peak;	/* in the same format as the data; allocated by caller */
	char *			meters_average;	/* in the same format as the data; allocated by caller */

	/*	timecode sent and received at buffer swap	*/
	int32_t			hours;
	int32_t			minutes;
	int32_t			seconds;
	int32_t			tc_frames;		/* for timecode frames as opposed to sample frames */
	int32_t			at_frame_delta;	/* how far into buffer (or before buffer for negative) */

};


typedef struct multi_mode_info multi_mode_info;
typedef struct multi_mode_list multi_mode_list;

struct multi_mode_list {
	size_t			info_size;		/* sizeof(multi_mode_list) */
	int32_t			in_request_count;
	int32_t			out_actual_count;
	int32_t			out_current_mode;
	multi_mode_info *
					io_modes;
};

struct multi_mode_info {
	int32_t			mode_id;
	uint32_t			flags;
	char			mode_name[64];
	int32_t			input_channel_count;
	int32_t			output_channel_count;
	float			best_frame_rate_in;
	float			best_frame_rate_out;
	uint32_t			sample_formats_in;
	uint32_t			sample_formats_out;
	char			_reserved[160];
};


/*	This extension protocol can grow however much you want.	*/
/*	Good extensions should be put into this header; really	*/
/*	good extensions should become part of the regular API.	*/
/*	For developer-developed extensions, use all lowercase	*/
/*	and digits (no upper case). If we then bless a third-	*/
/*	party extension, we can just upper-case the selector.	*/

typedef struct multi_extension_list multi_extension_list;
typedef struct multi_extension_info multi_extension_info;
struct multi_extension_info {
	uint32_t			code;
	uint32_t			flags;
	char			name[24];
};

#define B_MULTI_MAX_EXTENSION_COUNT 31
struct multi_extension_list	{ /* MULTI_LIST_EXTENSIONS */
	size_t			info_size;		/* sizeof(multi_extension_list) */
	uint32_t			max_count;
	int32_t			actual_count;	/* return # of actual extensions */
	multi_extension_info *
					extensions;		/* allocated by caller */
};

typedef struct multi_extension_cmd multi_extension_cmd;
struct multi_extension_cmd {	/* MULTI_GET_EXTENSION and MULTI_SET_EXTENSION */
	size_t			info_size;		/* sizeof(multi_extension_cmd) */
	uint32_t			code;
	uint32_t			_reserved_1;
	void *			in_data;
	size_t			in_size;
	void *			out_data;
	size_t			out_size;
};

enum {
	B_MULTI_EX_CLOCK_GENERATION = 'CLGE',
	B_MULTI_EX_DIGITAL_FORMAT = 'DIFO',
	B_MULTI_EX_OUTPUT_NOMINAL = 'OUNO',
	B_MULTI_EX_INPUT_NOMINAL = 'INNO'
};

typedef struct multi_ex_clock_generation multi_ex_clock_generation;
struct multi_ex_clock_generation {
	int32_t			channel;	/* if specific, or -1 for all */
	uint32_t			clock;		/* WORDCLOCK or SUPERCLOCK, typically */
};

typedef struct multi_ex_digital_format multi_ex_digital_format;
struct multi_ex_digital_format {
	int32_t			channel;	/* if specific, or -1 for all */
	uint32_t			format; 	/* B_CHANNEL_*_SPDIF or B_CHANNEL_*_EBU */
};

enum {
	B_MULTI_NOMINAL_MINUS_10 = 1, 
	B_MULTI_NOMINAL_PLUS_4
};

typedef struct multi_ex_nominal_level multi_ex_nominal_level;
struct multi_ex_nominal_level {
	int32_t			channel;	/* if specific, or -1 for all */
	int32_t			level;
};

#endif /* _MULTI_AUDIO_H */

