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

#include <MidiRoster.h>
#include <MidiConsumer.h>
#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <Node.h>
#include <NodeInfo.h>
#include <Path.h>
#include <PathFinder.h>

#include <kits/support/String.h>
#include <stdlib.h>

#include <MidiSettings.h>

#include <kits/debug/Debug.h>
#include <MidiGlue.h>   // for MAKE_BIGTIME
#include <SoftSynth.h>

using namespace BPrivate;

struct ReverbSettings {
	double room, damp, width, level;
} gReverbSettings[] = {
		 {0.0, 0.0, 0.0, 0.0},   //  B_REVERB_NONE
		 {0.2, 0.0, 0.5, 0.9},   //  B_REVERB_CLOSET
		 {0.5, 0.0, 0.9, 0.9},   //  B_REVERB_GARAGE
		 {0.7, 0.25, 0.9, 0.95}, //  B_REVERB_BALLROOM
		 {0.99, 0.3, 1.0, 1.0},  //  B_REVERB_CAVERN
		 {1.03, 0.6, 1.0, 1.0}   //  B_REVERB_DUNGEON
};


BSoftSynth::BSoftSynth()
: 	fInitCheck(false),
	fSynth(NULL),
	fSettings(NULL),
	fSoundPlayer(NULL),
	fMonitor(NULL),
	fMonitorSize(0),
	fMonitorChans(-1)
{
	fInstrumentsFile = NULL;
	SetDefaultInstrumentsFile();

	fSampleRate = 44100;
	fInterpMode = B_LINEAR_INTERPOLATION;
	fMaxVoices = 256;
	fLimiterThreshold = 7;
	fReverbEnabled = true;
	fReverbMode = B_REVERB_BALLROOM;
}


BSoftSynth::~BSoftSynth()
{
	// Note: it is possible that we don't get deleted. When BSynth is
	// created, it is assigned to the global variable  __be_synth. While
	// BSynth is alive, it keeps a copy of BSoftSynth around too. Not
	// a big deal, but the Midi Kit will complain (on stdout) that we 
	// didn't release our endpoints.

	delete[] fMonitor;
	Unload();
}


bool BSoftSynth::InitCheck()
{
	if (!fSynth)
		_Init();
	return fInitCheck;
}


void BSoftSynth::Unload(void)
{
	_Done();
	free(fInstrumentsFile);
	fInstrumentsFile = NULL;
}


bool BSoftSynth::IsLoaded(void) const
{
	return fInstrumentsFile != NULL;
}


status_t BSoftSynth::SetDefaultInstrumentsFile()
{
	// TODO: Duplicated code, check MidiSettingsView::_LoadSettings() and
	// MidiSettingsView::_RetrieveSoftSynthList()
	// We first search for a setting file (or symlink to it)
	// in the user settings directory

	struct BPrivate::midi_settings settings;
	if (BPrivate::read_midi_settings(&settings) == B_OK) {
		if (SetInstrumentsFile(settings.soundfont_file) == B_OK)
			return B_OK;
	}

	// Try a well-known (and usually present on a default install) soft synth
	BPath path;
	if (find_directory(B_SYNTH_DIRECTORY, &path, false, NULL) == B_OK) {
		path.Append("synth/TimGM6mb.sf2");
		if (SetInstrumentsFile(path.Path()) == B_OK)
			return B_OK;
	}

	// Just use the first soundfont we find
	BStringList paths;
	status_t status = BPathFinder::FindPaths(B_FIND_PATH_DATA_DIRECTORY,
			"synth", paths);

	if (status != B_OK)
		return B_ERROR;

	for (int32_t i = 0; i < paths.CountStrings(); i++) {
		BDirectory directory(paths.StringAt(i).String());
		BEntry entry;
		if (directory.InitCheck() != B_OK)
			continue;
		while (directory.GetNextEntry(&entry) == B_OK) {
			BNode node(&entry);
			BNodeInfo nodeInfo(&node);
			char mimeType[B_MIME_TYPE_LENGTH];
			// TODO: For some reason the mimetype check fails.
			// maybe because the file hasn't yet been sniffed and recognized?
			if (nodeInfo.GetType(mimeType) == B_OK
				/*&& !strcmp(mimeType, "audio/x-soundfont")*/) {
				BPath fullPath = paths.StringAt(i).String();
				fullPath.Append(entry.Name());
				if (SetInstrumentsFile(fullPath.Path()) == B_OK)
					return B_OK;
			}
		}
	}

	// TODO: Write the settings file ?
	
	return B_ERROR;
}


status_t BSoftSynth::SetInstrumentsFile(const char* path)
{
	if (path == NULL)
		return B_BAD_VALUE;
	
	if (!BEntry(path).Exists())
		return B_FILE_NOT_FOUND;

	if (IsLoaded())
		Unload();
	
	fInstrumentsFile = strdup(path);
	return B_OK;
}


status_t BSoftSynth::LoadAllInstruments()
{
	InitCheck();
	return B_OK;
}


status_t BSoftSynth::LoadInstrument(int16 instrument)
{
	UNIMPLEMENTED
	return B_OK;
}


status_t BSoftSynth::UnloadInstrument(int16 instrument)
{
	UNIMPLEMENTED
	return B_OK;
}


status_t BSoftSynth::RemapInstrument(int16 from, int16 to)
{
	UNIMPLEMENTED
	return B_OK;
}


void BSoftSynth::FlushInstrumentCache(bool startStopCache)
{
	// TODO: we may decide not to support this function because it's weird!
	
	UNIMPLEMENTED
}


void BSoftSynth::SetVolume(double volume)
{
	if (InitCheck())
		if (volume >= 0.0) {
			fluid_synth_set_gain(fSynth, volume);
		}
}


double 
BSoftSynth::Volume(void) const
{
	return fluid_synth_get_gain(fSynth);
}


status_t BSoftSynth::SetSamplingRate(int32_t rate)
{
	if (rate == 22050 || rate == 44100 || rate == 48000) {
		fSampleRate = rate;
		return B_OK;
	}
	
	return B_BAD_VALUE;
}


int32_t 
BSoftSynth::SamplingRate() const
{
	return fSampleRate;
}


status_t BSoftSynth::SetInterpolation(interpolation_mode mode)
{
	// not used because our synth uses the same format than the soundplayer
	fInterpMode = mode;
	return B_OK;
}


interpolation_mode 
BSoftSynth::Interpolation() const
{
	return fInterpMode;
}


status_t BSoftSynth::EnableReverb(bool enabled)
{
	fReverbEnabled = enabled;
	fluid_synth_set_reverb_on(fSynth, enabled);
	return B_OK;
}


bool BSoftSynth::IsReverbEnabled() const
{
	return fReverbEnabled;
}


void BSoftSynth::SetReverb(reverb_mode mode)
{
	if (mode < B_REVERB_NONE || mode > B_REVERB_DUNGEON)
		return;

	fReverbMode = mode;
	if (fSynth) {
		// We access the table using "mode - 1" because B_REVERB_NONE == 1
		ReverbSettings *rvb = &gReverbSettings[mode - 1];
		fluid_synth_set_reverb(fSynth, rvb->room, rvb->damp, rvb->width,
				rvb->level);
	}
}


reverb_mode 
BSoftSynth::Reverb() const
{
	return fReverbMode;
}


status_t BSoftSynth::SetMaxVoices(int32_t max)
{
	if (max > 0 && max <= 4096) {
		fMaxVoices = max;
		return B_OK;
	}
	
	return B_BAD_VALUE;
}


int16 
BSoftSynth::MaxVoices(void) const
{
	return fMaxVoices;
}


status_t BSoftSynth::SetLimiterThreshold(int32_t threshold)
{
	// not used
	if (threshold > 0 && threshold <= 32) {
		fLimiterThreshold = threshold;
		return B_OK;
	}
	
	return B_BAD_VALUE;
}


int16 
BSoftSynth::LimiterThreshold(void) const
{
	return fLimiterThreshold;
}


void BSoftSynth::Pause(void)
{
	UNIMPLEMENTED
}


void BSoftSynth::Resume(void)
{
	UNIMPLEMENTED
}


void BSoftSynth::NoteOff(
	uchar channel, uchar note, uchar velocity, uint32_t time)
{
	if (InitCheck()) {
		snooze_until(MAKE_BIGTIME(time), B_SYSTEM_TIMEBASE);
		fluid_synth_noteoff(fSynth, channel - 1, note);	// velocity isn't used in FS
	}
}


void BSoftSynth::NoteOn(
	uchar channel, uchar note, uchar velocity, uint32_t time)
{
	if (InitCheck()) {
		snooze_until(MAKE_BIGTIME(time), B_SYSTEM_TIMEBASE);
		fluid_synth_noteon(fSynth, channel - 1, note, velocity);
	}
}


void BSoftSynth::KeyPressure(
	uchar channel, uchar note, uchar pressure, uint32_t time)
{
	if (InitCheck()) {
		snooze_until(MAKE_BIGTIME(time), B_SYSTEM_TIMEBASE);
		// unavailable
	}
}


void BSoftSynth::ControlChange(
	uchar channel, uchar controlNumber, uchar controlValue, uint32_t time)
{
	if (InitCheck()) {
		snooze_until(MAKE_BIGTIME(time), B_SYSTEM_TIMEBASE);
		fluid_synth_cc(fSynth, channel - 1, controlNumber, controlValue);
	}
}


void BSoftSynth::ProgramChange(
	uchar channel, uchar programNumber, uint32_t time)
{
	if (InitCheck()) {
		snooze_until(MAKE_BIGTIME(time), B_SYSTEM_TIMEBASE);
		fluid_synth_program_change(fSynth, channel - 1, programNumber);
	}
}


void BSoftSynth::ChannelPressure(uchar channel, uchar pressure, uint32_t time)
{
	if (InitCheck()) {
		snooze_until(MAKE_BIGTIME(time), B_SYSTEM_TIMEBASE);
		//unavailable
	}
}


void BSoftSynth::PitchBend(uchar channel, uchar lsb, uchar msb, uint32_t time)
{
	if (InitCheck()) {
		snooze_until(MAKE_BIGTIME(time), B_SYSTEM_TIMEBASE);
		// fluid_synth only accepts an int
		fluid_synth_pitch_bend(fSynth, channel - 1,
			((uint32_t)(msb & 0x7f) << 7) | (lsb & 0x7f));
	}
}


void BSoftSynth::SystemExclusive(void* data, size_t length, uint32_t time)
{
	if (InitCheck()) {
		snooze_until(MAKE_BIGTIME(time), B_SYSTEM_TIMEBASE);
		// unsupported
	}
}


void BSoftSynth::SystemCommon(
	uchar status, uchar data1, uchar data2, uint32_t time)
{
	if (InitCheck()) {
		snooze_until(MAKE_BIGTIME(time), B_SYSTEM_TIMEBASE);
		// unsupported
	}
}


void BSoftSynth::SystemRealTime(uchar status, uint32_t time)
{
	if (InitCheck()) {
		snooze_until(MAKE_BIGTIME(time), B_SYSTEM_TIMEBASE);
		// unsupported
	}
}


void BSoftSynth::TempoChange(int32_t beatsPerMinute, uint32_t time)
{
	if (InitCheck()) {
		snooze_until(MAKE_BIGTIME(time), B_SYSTEM_TIMEBASE);
		// unsupported
	}
}


void BSoftSynth::AllNotesOff(bool justChannel, uint32_t time)
{
	if (InitCheck()) {
		snooze_until(MAKE_BIGTIME(time), B_SYSTEM_TIMEBASE);

		// from BMidi::AllNotesOff
		for (uchar channel = 1; channel <= 16; ++channel) {
			fluid_synth_cc(fSynth, channel - 1, B_ALL_NOTES_OFF, 0);
	
			if (!justChannel) {
				for (uchar note = 0; note <= 0x7F; ++note) {
					fluid_synth_noteoff(fSynth, channel - 1, note);
				}
			}
		}
	}
}


void BSoftSynth::_Init()
{
	status_t err;
	fInitCheck = false;
	
	_Done();

	fSettings = new_fluid_settings();
	fluid_settings_setnum(fSettings, (char*)"synth.sample-rate", fSampleRate);
	fluid_settings_setint(fSettings, (char*)"synth.polyphony", fMaxVoices);

	fSynth = new_fluid_synth(fSettings);
	if (!fSynth)
		return;
	
	err = fluid_synth_sfload(fSynth, fInstrumentsFile, 1); 
	if (err < B_OK) {
		fprintf(stderr, "error in fluid_synth_sfload\n");
		return;
	}

	SetReverb(fReverbMode);

	media_raw_audio_format format = media_raw_audio_format::wildcard;
	format.channel_count = 2;
	format.frame_rate = fSampleRate;
	format.format = media_raw_audio_format::B_AUDIO_FLOAT;

	fSoundPlayer = new BSoundPlayer(&format, "Soft Synth", &PlayBuffer, NULL, this);
	err = fSoundPlayer->InitCheck();
	if (err != B_OK) {
		fprintf(stderr, "error in BSoundPlayer\n");
		return;
	}

	fSoundPlayer->SetHasData(true);
	fSoundPlayer->Start();

	fInitCheck = true;
}


void BSoftSynth::_Done()
{
	if (fSoundPlayer) {
		fSoundPlayer->SetHasData(false);
		fSoundPlayer->Stop();
		delete fSoundPlayer;
		fSoundPlayer = NULL;
	}
	if (fSynth) {
		delete_fluid_synth(fSynth);
		fSynth = NULL;
	}
	if (fSettings) {
		delete_fluid_settings(fSettings);
		fSettings = NULL;
	}
}


void BSoftSynth::PlayBuffer(void* cookie, void* data, size_t size,
		const media_raw_audio_format& format)
{
	BSoftSynth* synth = (BSoftSynth*)cookie;

	if (synth->fMonitorSize == 0) {
		synth->fMonitor = (float*)new void*[size];
		synth->fMonitorSize = size;
		synth->fMonitorChans = format.channel_count;
	}

	// we use float samples
	if (synth->fSynth) {
		fluid_synth_write_float(
			synth->fSynth, size / sizeof(float) / format.channel_count,
			data, 0, format.channel_count,
			data, 1, format.channel_count);

		memcpy(synth->fMonitor, data, size);
	}
}

