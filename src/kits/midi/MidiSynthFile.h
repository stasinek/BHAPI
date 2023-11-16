
#ifndef _MIDI_SYNTH_FILE_H
#define _MIDI_SYNTH_FILE_H

#include <Haiku.h>
#include <MidiSynth.h>
#include <Entry.h>

typedef void (*synth_file_hook)(int32_t arg);

class BMidiStore;

class BMidiSynthFile : public BMidiSynth {
public:

	BMidiSynthFile();
	~BMidiSynthFile();

	status_t LoadFile(const entry_ref* midi_entry_ref);
	void UnloadFile(void);

	virtual status_t Start(void);
	virtual void Stop(void);

	void Fade(void);
	void Pause(void);
	void Resume(void);

	int32_t Duration(void) const;
	int32_t Position(int32_t ticks) const;
	int32_t Seek();

	status_t GetPatches(int16* pArray768, int16* pReturnedCount) const;

	void SetFileHook(synth_file_hook pSongHook, int32_t arg);

	bool IsFinished(void) const;

	void ScaleTempoBy(double tempoFactor);
	void SetTempo(int32_t newTempoBPM);
	int32_t Tempo(void) const;

	void EnableLooping(bool loop);

	void MuteTrack(int16 track, bool do_mute);
	void GetMuteMap(char* pTracks) const;

	void SoloTrack(int16 track, bool do_solo);
	void GetSoloMap(char* pTracks) const;

private:

	friend class BSynth;							   

	virtual void _ReservedMidiSynthFile1();
	virtual void _ReservedMidiSynthFile2();
	virtual void _ReservedMidiSynthFile3();

	BMidiStore* fStore;
	
	int32_t _reserved[3];
};

#endif // _MIDI_SYNTH_FILE
