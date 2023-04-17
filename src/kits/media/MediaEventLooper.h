/*
 * Copyright 2009, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _MEDIA_EVENT_LOOPER_H
#define _MEDIA_EVENT_LOOPER_H


#include <MediaNode.h>
#include <TimedEventQueue.h>
#include <Haiku.h>


/*! BMediaEventLooper spawns a thread which calls WaitForMessage, pushes
    BMediaNode messages onto its BTimedEventQueues. informs you when it
    is time to handle an event. Report your event latency, push other events
    onto the queue and override HandleEvent to do your work.
*/
class BMediaEventLooper : public virtual BMediaNode {
protected:
    enum run_state {
        B_IN_DISTRESS		= -1,
        B_UNREGISTERED,
        B_STOPPED,
        B_STARTED,
        B_QUITTING,
        B_TERMINATED,
        B_USER_RUN_STATES	= 0x4000
    };

protected:
    explicit					BMediaEventLooper(
                                    uint32_t apiVersion = B_BEOS_VERSION);
    virtual						~BMediaEventLooper();

protected:
    // BMediaNode interface
    virtual	void				NodeRegistered();
    virtual	void				Start(bigtime_t performanceTime);
    virtual	void				Stop(bigtime_t performanceTime,
                                    bool immediate);
    virtual	void				Seek(bigtime_t mediaTime,
                                    bigtime_t performanceTime);
    virtual	void				TimeWarp(bigtime_t atRealTime,
                                    bigtime_t toPerformanceTime);
    virtual	status_t			AddTimer(bigtime_t atPerformanceTime,
                                    int32_t cookie);
    virtual	void 				SetRunMode(run_mode mode);

protected:
    // BMediaEventLooper Hook functions

    // NOTE: You must override this method to handle your events!
    // You should not call HandleEvent directly.
    virtual void				HandleEvent(const media_timed_event* event,
                                    bigtime_t lateness,
                                    bool realTimeEvent = false) = 0;

    // Override this method to properly clean up any custom events you have
    // added to your event queue.
    virtual void				CleanUpEvent(const media_timed_event* event);

    // NOTE: Called in offline mode to determine the current time of the node.
    // Update your internal information whenever it changes.
    virtual	bigtime_t			OfflineTime();

    // NOTE: Override this method only if you know what you are doing!
    // The default control loop function waits for messages, pops events
    // off the queue and calls DispatchEvent.
    virtual	void				ControlLoop();

            thread_id			ControlThread();

protected:
            BTimedEventQueue* 	EventQueue();
            BTimedEventQueue*	RealTimeQueue();

            int32_t				Priority() const;
            int32_t				RunState() const;
            bigtime_t			EventLatency() const;
            bigtime_t			BufferDuration() const;
            bigtime_t			SchedulingLatency() const;

    // NOTE: Use the priority constants from OS.h or suggest_thread_priority
    // from scheduler.h. The passed priority will be clamped to be in range 5
    // to 120.
            status_t			SetPriority(int32_t priority);
            void				SetRunState(run_state state);
            void				SetEventLatency(bigtime_t latency);
            void				SetBufferDuration(bigtime_t duration);
            void				SetOfflineTime(bigtime_t offTime);

    // Spawns and resumes the control thread - must be called from
    // NodeRegistered().
            void				Run();

    // Quits the control thread - must be called from your destructor.
            void				Quit();

    // Calls HandleEvent and does BMediaEventLooper event work
            void				DispatchEvent(const media_timed_event* event,
                                    bigtime_t lateness,
                                    bool realTimeEvent = false);

private:
    static	int32_t				_ControlThreadStart(void* cookie);
    static	void				_CleanUpEntry(const media_timed_event* event,
                                    void* context);
            void				_DispatchCleanUp(
                                    const media_timed_event* event);

private:
            BTimedEventQueue	fEventQueue;
            BTimedEventQueue	fRealTimeQueue;
            thread_id			fControlThread;
            int32_t				fCurrentPriority;
            int32_t				fSetPriority;
            vint32_t				fRunState;
            bigtime_t			fEventLatency;
            bigtime_t			fSchedulingLatency;
            bigtime_t			fBufferDuration;
            bigtime_t			fOfflineTime;
            uint32_t				fApiVersion;

protected:
    virtual	status_t 	DeleteHook(BMediaNode * node);

private:
    // FBC padding and forbidden methods
                                BMediaEventLooper(const BMediaEventLooper&);
            BMediaEventLooper&	operator=(const BMediaEventLooper&);

    virtual	status_t		 	_Reserved_BMediaEventLooper_0(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_1(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_2(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_3(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_4(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_5(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_6(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_7(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_8(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_9(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_10(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_11(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_12(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_13(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_14(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_15(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_16(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_17(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_18(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_19(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_20(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_21(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_22(int32_t arg, ...);
    virtual	status_t 			_Reserved_BMediaEventLooper_23(int32_t arg, ...);

    bool						_reserved_bool_[4];
    uint32_t						_reserved_BMediaEventLooper_[12];
};

#endif // _MEDIA_EVENT_LOOPER_H
