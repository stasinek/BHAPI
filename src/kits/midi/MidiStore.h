
#ifndef _MIDI_STORE_H
#define _MIDI_STORE_H

#include <Haiku.h>
#include <Midi.h>
#include <MidiSynthFile.h>

struct entry_ref;

class BFile;
class BList;
class BMidiEvent;

class BMidiStore : public BMidi {
public:

	BMidiStore();
	virtual ~BMidiStore();

	virtual void NoteOff(
		uchar channel, uchar note, uchar velocity, uint32_t time = B_NOW);

	virtual void NoteOn(
		uchar channel, uchar note, uchar velocity, uint32_t time = B_NOW);

	virtual void KeyPressure(
		uchar channel, uchar note, uchar pressure, uint32_t time = B_NOW);

	virtual void ControlChange(
		uchar channel, uchar controlNumber, uchar controlValue,
		uint32_t time = B_NOW);

	virtual void ProgramChange(
		uchar channel, uchar programNumber, uint32_t time = B_NOW);

	virtual void ChannelPressure(
		uchar channel, uchar pressure, uint32_t time = B_NOW);

	virtual void PitchBend(
		uchar channel, uchar lsb, uchar msb, uint32_t time = B_NOW);

	virtual void SystemExclusive(
		void* data, size_t length, uint32_t time = B_NOW);

	virtual void SystemCommon(
		uchar status, uchar data1, uchar data2, uint32_t time = B_NOW);

	virtual void SystemRealTime(uchar status, uint32_t time = B_NOW);

	virtual void TempoChange(int32_t beatsPerMinute, uint32_t time = B_NOW);

	status_t Import(const entry_ref* ref);
	status_t Export(const entry_ref* ref, int32_t format);

	void SortEvents(bool force = false);
	uint32_t CountEvents() const;

	uint32_t CurrentEvent() const;
	void SetCurrentEvent(uint32_t eventNumber);

	uint32_t DeltaOfEvent(uint32_t eventNumber) const;
	uint32_t EventAtDelta(uint32_t time) const;

	uint32_t BeginTime() const;
		
	void SetTempo(int32_t beatsPerMinute);
	int32_t Tempo() const;

private:

	friend class BMidiSynthFile;
	
	virtual void _ReservedMidiStore1();
	virtual void _ReservedMidiStore2();
	virtual void _ReservedMidiStore3();

	virtual void Run();

	void AddEvent(BMidiEvent* event);
	void SprayEvent(const BMidiEvent* event, uint32_t time);
	BMidiEvent* EventAt(int32_t index) const;
	uint32_t GetEventTime(const BMidiEvent* event) const;
	uint32_t TicksToMilliseconds(uint32_t ticks) const;
	uint32_t MillisecondsToTicks(uint32_t ms) const;

	BList* fEvents;
	int32_t fCurrentEvent;
	uint32_t fStartTime;
	int32_t fBeatsPerMinute;
	int16 fTicksPerBeat;
	bool fNeedsSorting;

	void ReadFourCC(char* fourcc);
	uint32_t Read32Bit();
	uint16 Read16Bit();
	uint8 PeekByte();
	uint8 NextByte();
	void SkipBytes(uint32_t length);
	uint32_t ReadVarLength();
	void ReadChunk();
	void ReadTrack();
	void ReadSystemExclusive();
	void ReadMetaEvent();

	void WriteFourCC(char a, char b, char c, char d);
	void Write32Bit(uint32_t val);
	void Write16Bit(uint16 val);
	void WriteByte(uint8 val);
	void WriteVarLength(uint32_t val);
	void WriteTrack();
	void WriteMetaEvent(BMidiEvent* event);

	BFile* fFile;
	uint32_t fByteCount;
	uint32_t fTotalTicks;
	uint16 fNumTracks;
	uint16 fCurrTrack;
	uint16 fFormat;

	uint16 _reserved1[1];

	bool* fInstruments;
	synth_file_hook fHookFunc;
	int32_t fHookArg;
	bool fLooping;
	bool fPaused;
	bool fFinished;
	
	uint32_t _reserved2[12];
};

#endif // _MIDI_STORE_H
