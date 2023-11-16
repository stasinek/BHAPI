/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
 *
 * BHAPI++ library is a freeware; it may be used and distributed according to
 * the terms of The MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File: Job.h
 *
 * --------------------------------------------------------------------------*/
/*
 * Copyright 2011-2015, Haiku, Inc.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_SUPPORT_JOB_H
#define BHAPI_SUPPORT_JOB_H

#include <ObjectList.h>
#include <kits/support/String.h>

namespace BSupportKit {
class BJob;
struct BJobStateListener {
	virtual						~BJobStateListener();

								// these default implementations do nothing
	virtual	void				JobStarted(BJob* job);
	virtual	void				JobProgress(BJob* job);
	virtual	void				JobSucceeded(BJob* job);
	virtual	void				JobFailed(BJob* job);
	virtual	void				JobAborted(BJob* job);
};

enum BJobState {
	B_JOB_STATE_WAITING_TO_RUN,
	B_JOB_STATE_STARTED,
	B_JOB_STATE_IN_PROGRESS,
	B_JOB_STATE_SUCCEEDED,
	B_JOB_STATE_FAILED,
	B_JOB_STATE_ABORTED,
};

class BJob {
public:
								BJob(const BString& title);
	virtual						~BJob();

			status_t			InitCheck() const;

	virtual	status_t			Run();

			const BString&		Title() const;
			BJobState			State() const;
			status_t			Result() const;
			const BString&		ErrorString() const;

			uint32_t				TicketNumber() const;

			status_t			AddStateListener(BJobStateListener* listener);
			status_t			RemoveStateListener(
									BJobStateListener* listener);

			bool				IsRunnable() const;
			status_t			AddDependency(BJob* job);
			status_t			RemoveDependency(BJob* job);
			int32_t				CountDependencies() const;

			BJob*				DependantJobAt(int32_t index) const;

			class Private;

protected:
	virtual	status_t			Execute() = 0;
	virtual	void				Cleanup(status_t jobResult);

			void				SetState(BJobState state);
			void				SetErrorString(const BString&);

			void				NotifyStateListeners();

private:
	friend class Private;

			void				_SetTicketNumber(uint32_t ticketNumber);
			void				_ClearTicketNumber();

private:
			status_t			fInitStatus;
			BString				fTitle;

			BJobState			fState;
			status_t			fResult;
			BString				fErrorString;

			uint32_t				fTicketNumber;

	typedef	BObjectList<BJob>	JobList;
			JobList				fDependencies;
			JobList				fDependantJobs;

	typedef	BObjectList<BJobStateListener>	StateListenerList;
			StateListenerList	fStateListeners;
};

}	// namespace BSupportKit
#endif // _SUPPORT_JOB_H_
