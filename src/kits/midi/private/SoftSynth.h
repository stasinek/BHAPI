/*
 * Copyright 2006-2014, Haiku.
 * 
 * Copyright (c) 2004-2005 Matthijs Hollemans
 * Copyright (c) 2003 Jerome Leveque
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		Jérôme Duval
 *		Jérôme Leveque
 *		Matthijs Hollemans
 *		Pete Goodeve
 */

#ifndef _SOFT_SYNTH_H
#define _SOFT_SYNTH_H

/*
	This version of SoftSynth is a wrapper libfluidsynth.so.
 */

#include <fluidsynth.h>
#include <Midi.h>
#include <SoundPlayer.h>
#include <Synth.h>

class BMidiConsumer;
class BMidiSynth;
class BSynth;

namespace BPrivate {

class BSoftSynth {
public:
	bool InitCheck();

	void Unload();
	bool IsLoaded() const;

	status_t SetDefaultInstrumentsFile();
	status_t SetInstrumentsFile(const char* path);

	status_t LoadAllInstruments();
	status_t LoadInstrument(int16 instrument);
	status_t UnloadInstrument(int16 instrument);
	status_t RemapInstrument(int16 from, int16 to);
	void FlushInstrumentCache(bool startStopCache);

	void SetVolume(double volume);
	double Volume(void) const;

	status_t SetSamplingRate(int32_t rate);
	int32_t SamplingRate() const;

	status_t SetInterpolation(interpolation_mode mode);
	interpolation_mode Interpolation() const;

	status_t EnableReverb(bool enabled);
	bool IsReverbEnabled() const;
	void SetReverb(reverb_mode mode);
	reverb_mode Reverb() const;

	status_t SetMaxVoices(int32_t max);
	int16 MaxVoices(void) const;

	status_t SetLimiterThreshold(int32_t threshold);
	int16 LimiterThreshold(void) const;

	void Pause(void);
	void Resume(void);

	void NoteOff(uchar, uchar, uchar, uint32_t);
	void NoteOn(uchar, uchar, uchar, uint32_t);
	void KeyPressure(uchar, uchar, uchar, uint32_t);
	void ControlChange(uchar, uchar, uchar, uint32_t);
	void ProgramChange(uchar, uchar, uint32_t);
 	void ChannelPressure(uchar, uchar, uint32_t);
	void PitchBend(uchar, uchar, uchar, uint32_t);
	void SystemExclusive(void*, size_t, uint32_t);
	void SystemCommon(uchar, uchar, uchar, uint32_t);
	void SystemRealTime(uchar, uint32_t);
	void TempoChange(int32_t, uint32_t);
	void AllNotesOff(bool, uint32_t);

private:

	friend class ::BSynth;
	friend class ::BMidiSynth;

	BSoftSynth();
	~BSoftSynth();

	void _Init();
	void _Done();
	static void PlayBuffer(void* cookie, void* data, size_t size,
			const media_raw_audio_format& format);
	
	bool fInitCheck;
	char* fInstrumentsFile;
	int32_t fSampleRate;
	interpolation_mode fInterpMode;
	int16 fMaxVoices;
	int16 fLimiterThreshold;
	reverb_mode fReverbMode;
	bool fReverbEnabled;

	fluid_synth_t* fSynth;
	fluid_settings_t* fSettings;

	BSoundPlayer* fSoundPlayer;

	float* fMonitor;
	size_t fMonitorSize;
	int16 fMonitorChans;
};

} // namespace BPrivate

#endif // _SOFT_SYNTH_H
