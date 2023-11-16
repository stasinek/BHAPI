/*
 * Copyright 2005, Ingo Weinhold, bonefish@users.sf.net. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef MESSAGING_SERVICE_H
#define MESSAGING_SERVICE_H


//! kernel-side implementation-private definitions for the messaging service


#include <MessagingServiceDefs.h>

#include <lock.h>


namespace BPrivate {

// MessagingArea
class MessagingArea {
public:
	~MessagingArea();

	static MessagingArea *Create(sem_id lockSem, sem_id counterSem);

	static bool CheckCommandSize(int32_t dataSize);

	void InitHeader();

	bool Lock();
	void Unlock();

	area_id ID() const;
	int32_t Size() const;
	bool IsEmpty() const;

	void *AllocateCommand(uint32_t commandWhat, int32_t dataSize, bool &wasEmpty);
	void CommitCommand();

	void SetNextArea(MessagingArea *area);
	MessagingArea *NextArea() const;

private:
	MessagingArea();

	messaging_command *_CheckCommand(int32_t offset, int32_t &size);

	messaging_area_header	*fHeader;
	area_id					fID;
	int32_t					fSize;
	sem_id					fLockSem;
	sem_id					fCounterSem;
	MessagingArea			*fNextArea;
};

// MessagingService
class MessagingService {
public:
	MessagingService();
	~MessagingService();

	status_t InitCheck() const;

	bool Lock();
	void Unlock();

	status_t RegisterService(sem_id lockingSem, sem_id counterSem,
		area_id &areaID);
	status_t UnregisterService();

	status_t SendMessage(const void *message, int32_t messageSize,
		const messaging_target *targets, int32_t targetCount);

private:
	status_t _AllocateCommand(int32_t commandWhat, int32_t size,
		MessagingArea *&area, void *&data, bool &wasEmpty);

	recursive_lock	fLock;
	team_id			fServerTeam;
	sem_id			fLockSem;
	sem_id			fCounterSem;
	MessagingArea	*fFirstArea;
	MessagingArea	*fLastArea;
};

}	// namespace BPrivate

using BPrivate::MessagingArea;
using BPrivate::MessagingService;

#endif	// MESSAGING_SERVICE_H
