/*
 *  Copyright 2001-2002, Haiku Inc. All Rights Reserved.
 *  Distributed under the terms of the MIT License.
 *
 * Author:
 *		Christopher ML Zumwalt May (zummy@users.sf.net)
 */
#ifndef _GAMESOUND_H
#define _GAMESOUND_H


#include <GameSoundDefs.h>

#include <new>


class BGameSoundDevice;


class BGameSound {
public:
								BGameSound(BGameSoundDevice* device = NULL);
	virtual						~BGameSound();

	virtual	BGameSound*			Clone() const = 0;
			status_t			InitCheck() const;

	// BGameSound attributes
			BGameSoundDevice*	Device() const;
			gs_id				ID() const;
			const gs_audio_format& Format() const;
				//	only valid after Init()

	// Playing sounds
	virtual	status_t			StartPlaying();
	virtual	bool				IsPlaying();
	virtual	status_t			StopPlaying();

	// Modifing the playback
			status_t			SetGain(float gain, bigtime_t duration = 0);
				//	ramp duration in seconds
			status_t			SetPan(float pan, bigtime_t duration = 0);
				//	ramp duration in seconds

			float				Gain();
			float				Pan();

	virtual	status_t			SetAttributes(gs_attribute* attributes,
									size_t attributeCount);
	virtual	status_t			GetAttributes(gs_attribute* attributes,
									size_t attributeCount);

			void* 				operator new(size_t size);
			void*				operator new(size_t size,
									const std::nothrow_t&) throw();
			void				operator delete(void* ptr);
			void				operator delete(void* ptr, 
									const std::nothrow_t&) throw();

	static	status_t			SetMemoryPoolSize(size_t poolSize);
	static	status_t			LockMemoryPool(bool lockInCore);
	static	int32_t				SetMaxSoundCount(int32_t maxCount);

	virtual	status_t	 		Perform(int32_t selector, void* data);

protected:
			status_t			SetInitError(status_t initError);
			status_t			Init(gs_id handle);

								BGameSound(const BGameSound& other);
			BGameSound&			operator=(const BGameSound& other);

private:
								BGameSound();
									// not implemented

	virtual	status_t			_Reserved_BGameSound_0(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_1(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_2(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_3(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_4(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_5(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_6(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_7(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_8(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_9(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_10(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_11(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_12(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_13(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_14(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_15(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_16(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_17(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_18(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_19(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_20(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_21(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_22(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_23(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_24(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_25(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_26(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_27(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_28(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_29(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_30(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_31(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_32(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_33(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_34(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_35(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_36(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_37(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_38(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_39(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_40(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_41(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_42(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_43(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_44(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_45(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_46(int32_t arg, ...);
	virtual	status_t			_Reserved_BGameSound_47(int32_t arg, ...);

private:
			BGameSoundDevice*	fDevice;
			status_t			fInitError;
			
			gs_audio_format		fFormat;		
			gs_id				fSound;

			uint32_t				_reserved[16];
};


#endif	// _GAME_SOUND_H
