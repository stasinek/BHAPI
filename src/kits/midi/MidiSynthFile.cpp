/*
 * Copyright 2006, Haiku.
 *
 * Copyright (c) 2002-2004 Matthijs Hollemans
 * Copyright (c) 2003 Jerome Leveque
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		
 *		Matthijs Hollemans
 *		Jérôme Leveque
 */

#include <kits/debug/Debug.h>
#include <MidiStore.h>
#include <MidiSynthFile.h>
#include <SoftSynth.h>

using namespace BPrivate;


BMidiSynthFile::BMidiSynthFile()
{
	fStore = new BMidiStore();
}


BMidiSynthFile::~BMidiSynthFile()
{
	Stop();
	delete fStore;
}


status_t BMidiSynthFile::LoadFile(const entry_ref* midi_entry_ref)
{
	if (midi_entry_ref == NULL)
		return B_BAD_VALUE;

	EnableInput(true, false);
	fStore->fFinished = true;

	status_t err = fStore->Import(midi_entry_ref);

	if (err == B_OK) {
		for (int t = 0; t < 128; ++t) {
			if (fStore->fInstruments[t]) {
				__be_synth->fSynth->LoadInstrument(t);
			}
		}
	}
	
	return err;
}


void BMidiSynthFile::UnloadFile(void)
{
	delete fStore;
	fStore = new BMidiStore();
}


status_t BMidiSynthFile::Start(void)
{
	fStore->Connect(this);
	fStore->SetCurrentEvent(0);
	return fStore->Start();
}


void BMidiSynthFile::Stop(void)
{
	fStore->Stop();
	fStore->Disconnect(this);
}


void BMidiSynthFile::Fade(void)
{
	Stop();  // really quick fade :P
}


void BMidiSynthFile::Pause(void)
{
	fStore->fPaused = true;
}


void BMidiSynthFile::Resume(void)
{
	fStore->fPaused = false;
}


int32_t 
BMidiSynthFile::Duration(void) const
{
	return fStore->DeltaOfEvent(fStore->CountEvents());
}


int32_t 
BMidiSynthFile::Position(int32_t ticks) const
{
	return fStore->EventAtDelta(ticks);
}


int32_t 
BMidiSynthFile::Seek()
{
	return fStore->CurrentEvent();
}


status_t BMidiSynthFile::GetPatches(
	int16* pArray768, int16* pReturnedCount) const
{
	int16 count = 0;
	
	for (int t = 0; t < 128; ++t) {
		if (fStore->fInstruments[t]) {
			pArray768[count++] = t;
		}
	}

	*pReturnedCount = count;
	return B_OK;
}


void BMidiSynthFile::SetFileHook(synth_file_hook pSongHook, int32_t arg)
{
	fStore->fHookFunc = pSongHook;
	fStore->fHookArg = arg;
}


bool BMidiSynthFile::IsFinished() const
{
	return fStore->fFinished;
}


void BMidiSynthFile::ScaleTempoBy(double tempoFactor)
{
	fStore->SetTempo((int32_t) (Tempo() * tempoFactor));
}


void BMidiSynthFile::SetTempo(int32_t newTempoBPM)
{
	fStore->SetTempo(newTempoBPM);
}


int32_t 
BMidiSynthFile::Tempo(void) const
{
	return fStore->Tempo();
}


void BMidiSynthFile::EnableLooping(bool loop)
{
	fStore->fLooping = loop;
}


void BMidiSynthFile::MuteTrack(int16 track, bool do_mute)
{
	fprintf(stderr, "[midi] MuteTrack is broken; don't use it\n");
}


void BMidiSynthFile::GetMuteMap(char* pTracks) const
{
	fprintf(stderr, "[midi] GetMuteMap is broken; don't use it\n");
}


void BMidiSynthFile::SoloTrack(int16 track, bool do_solo)
{
	fprintf(stderr, "[midi] SoloTrack is broken; don't use it\n");
}


void BMidiSynthFile::GetSoloMap(char* pTracks) const
{
	fprintf(stderr, "[midi] GetSoloMap is broken; don't use it\n");
}


void BMidiSynthFile::_ReservedMidiSynthFile1() { }
void BMidiSynthFile::_ReservedMidiSynthFile2() { }
void BMidiSynthFile::_ReservedMidiSynthFile3() { }

