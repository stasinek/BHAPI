/* ++++++++++

   FILE:  BufferMsgs.h

   Copyright (c) 1995-1997 by Be Incorporated.  All Rights Reserved.

+++++ */
#ifndef _BUFFER_MSGS_H
#define _BUFFER_MSGS_H

#include <Haiku.h>

/****************************************************************
This file defines the messages sent between a Subscriber and a
Server.  Changes to the protocol should be noted here and appropriate
modifications made to Subscriber.cpp and any servers (currently
the audio_server is the only one).

BufferMsgs defines a message-based interface, not a class
interface.  A BufferMsgs receives messages from Subscribers via the
BMessenger class.

Here are the messages that must be supported for the base Subscriber
class and the replies that a server will send.  Specific Servers 
may support other messages as well.

====
Acquire a stream-id for subsequent operations.
'resource' is a server-specific value that specifies the resource
to which the client wants access.

Upon success, the server replies with 'stream_id' (used for
subsequent operations).

GET_STREAM_ID int32_t("resource")
=> GET_STREAM_ID int32_t("stream_id")
=> ERROR_RETURN int32_t("error")

====
Acquire access to a stream for subsequent operations.
'stream_id' specifies the stream to which the client wants access.
'will_wait' determines if the client will receive an immediate reply
or if it will block until access is granted.  'sem' is a semaphore
used to indicate that the stream has released a buffer.

Upon success, the server replies with 'subscriber_id' (used for
subsequent operations).

SUBSCRIBE String("name")
		  int32_t("stream_id")
		  int32_t("sem") 
		  Bool("will_wait")
=> SUBSCRIBE int32_t("subscriber_id")
=> ERROR_RETURN int32_t("error")

====
Relinquish access to the stream.

UNSUBSCRIBE int32_t("subscriber_id")
=> UNSUBSCRIBE
=> ERROR_RETURN int32_t("error")

====
Join the stream at the specified position and start receiving buffers
of data.
ENTER_STREAM int32_t("subscriber_id")
			 int32_t("neighbor")
			 Bool("before")
=> ENTER_STREAM
=> ERROR_RETURN int32_t("error")

====
Issue a request to stop receiving buffers.  More buffers may continue
to arrive, but you must keep acquiring_ and releasing_ them until you
get one for which is_last_buffer() is true.  Then you can stop.

EXIT_STREAM int32_t("subscriber_id")
=> EXIT_STREAM
=> ERROR_RETURN int32_t("error")

====
Get information about a particular buffer stream.

GET_STREAM_PARAMS int32_t("stream_id")
=> GET_STREAM_PARAMS int32_t("buffer_size")
					 int32_t("buffer_count")
					 Bool("is_running")
					 int32_t("subscriber_count")
=> ERROR_RETURN int32_t("error")
====
Set information about a particular buffer stream.

SET_STREAM_PARAMS int32_t("stream_id")
				  int32_t("buffer_size")	<<optional>>
				  int32_t("buffer_count")	<<optional>>
				  Bool("is_running")	<<optional>>
=> SET_STREAM_PARAMS int32_t("buffer_size")
					 int32_t("buffer_count")
					 Bool("is_running")
					 int32_t("subscriber_count")
=> ERROR_RETURN int32_t("error")

====
Return the subscriber id of the index'th subscriber sharing the
stream with the given subscriber.

SUBSCRIBER_INFO int32_t("subscriber_id")
=> SUBSCRIBER_INFO String("subscriber_name")
				   int32_t("stream_id")		// granted access to
				   int32_t("position")		// position (if active) or -1
=> ERROR_RETURN int32_t("error")

<end of long comment>
****************************************************************/

/* message values */
enum {
  DEBUG_SERVER = 99,
  SUBSCRIBE,
  UNSUBSCRIBE,
  ENTER_STREAM,
  EXIT_STREAM,
  GET_STREAM_PARAMETERS,
  SET_STREAM_PARAMETERS,
  GET_STREAM_ID,
  SUBSCRIBER_INFO,
  ERROR_RETURN
  };

#endif			// #ifndef _BUFFER_MSGS_H
