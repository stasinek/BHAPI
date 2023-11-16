/*
 * Copyright 2013, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef SEMAPHORE_INFO_H
#define SEMAPHORE_INFO_H

#include <kits/kernel/OS.h>
#include <kits/support/String.h>

#include <Types.h>


class SemaphoreInfo {
public:
								SemaphoreInfo();
								SemaphoreInfo(const SemaphoreInfo& other);
								SemaphoreInfo(team_id team, sem_id semaphore,
									const BString& name, int32_t count,
									thread_id latestHolder);

			void				SetTo(team_id team, sem_id semaphore,
									const BString& name, int32_t count,
									thread_id latestHolder);

			team_id				TeamID() const	{ return fTeam; }
			area_id				SemID() const	{ return fSemaphore; }
			const BString&		Name() const	{ return fName; }

			int32_t				Count() const	{ return fCount; }
			thread_id			LatestHolder() const
												{ return fLatestHolder; }
private:
			team_id				fTeam;
			sem_id				fSemaphore;
			BString				fName;
			int32_t				fCount;
			thread_id			fLatestHolder;
};


#endif // AREA_INFO_H
