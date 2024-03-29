
#ifndef _MIDI_ENDPOINT_H
#define _MIDI_ENDPOINT_H

#include <Haiku.h>
#include <Message.h>
#include <Midi2Defs.h>
#include <kits/kernel/OS.h>
#include <kits/support/String.h>

class BMidiProducer;
class BMidiConsumer;

namespace BPrivate { class BMidiRosterLooper; }

class BMidiEndpoint 
{
public:

	const char *Name() const;
	void SetName(const char *name);
	
	int32_t ID() const;

	bool IsProducer() const;
	bool IsConsumer() const;
	bool IsRemote() const;
	bool IsLocal() const;
	bool IsPersistent() const;
	bool IsValid() const;

	status_t Release();
	status_t Acquire();

	status_t SetProperties(const BMessage *properties);
	status_t GetProperties(BMessage *properties) const;

	status_t Register();
	status_t Unregister();
						
private:

	friend class BMidiConsumer;
	friend class BMidiLocalConsumer;
	friend class BMidiLocalProducer;
	friend class BMidiProducer;
	friend class BMidiRoster;
	friend class BPrivate::BMidiRosterLooper;
	
	BMidiEndpoint(const char *name);
	virtual	~BMidiEndpoint();

	virtual void _Reserved1();
	virtual void _Reserved2();
	virtual void _Reserved3();
	virtual void _Reserved4();
	virtual void _Reserved5();
	virtual void _Reserved6();
	virtual void _Reserved7();
	virtual void _Reserved8();
	
	status_t SendRegisterRequest(bool);
	status_t SendChangeRequest(BMessage *);

	bool IsRegistered() const;
	bool LockLooper() const;
	void UnlockLooper() const;

	int32_t fId;
	BString fName;	
	int32_t fRefCount;
	BMessage *fProperties;
	bool fIsLocal;
	bool fIsConsumer;
	bool fIsRegistered;
	bool fIsAlive;

	uint32_t _reserved[4];
};

#endif // _MIDI_ENDPOINT_H
