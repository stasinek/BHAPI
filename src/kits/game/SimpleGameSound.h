/*
 *  Copyright 2001-2010, Haiku Inc. All Rights Reserved.
 *  Distributed under the terms of the MIT License.
 */
#ifndef _SIMPLEGAMESOUND_H
#define _SIMPLEGAMESOUND_H


#include <GameSoundDefs.h>
#include <GameSound.h>


struct entry_ref;


class BSimpleGameSound : public BGameSound {
public:
								BSimpleGameSound(const entry_ref* file,
									BGameSoundDevice* device = NULL);
								BSimpleGameSound(const char* file,
									BGameSoundDevice* device = NULL);
								BSimpleGameSound(const void* data, 
									size_t frameCount,
									const gs_audio_format* format, 
									BGameSoundDevice* device = NULL);
								BSimpleGameSound(const BSimpleGameSound& other);

	virtual						~BSimpleGameSound();

	virtual	BGameSound*			Clone() const;

	virtual	status_t 			Perform(int32_t selector, void* data);

			status_t			SetIsLooping(bool looping);
			bool				IsLooping() const;
	
private:
								BSimpleGameSound();
			BSimpleGameSound&	operator=(const BSimpleGameSound& other);
									// not implemented

			status_t	 		Init(const entry_ref* file);
			status_t 			Init(const void* data, int64 frameCount,
									const gs_audio_format* format);

	virtual	status_t			_Reserved_BSimpleGameSound_0(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_1(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_2(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_3(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_4(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_5(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_6(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_7(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_8(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_9(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_10(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_11(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_12(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_13(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_14(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_15(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_16(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_17(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_18(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_19(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_20(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_21(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_22(int32_t arg, ...);
	virtual	status_t			_Reserved_BSimpleGameSound_23(int32_t arg, ...);

private:
			uint32_t				_reserved[12];
};


#endif	// _SIMPLE_GAME_SOUND_H
