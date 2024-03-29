/*
 * Copyright 2012-2014, Rene Gollent, rene@gollent.com.
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */

#include <Worker.h>

#include <AutoDeleter.h>
#include <AutoLocker.h>


// pragma mark - JobKey


JobKey::~JobKey()
{
}


// pragma mark - SimpleJobKey


SimpleJobKey::SimpleJobKey(const void* object, uint32_t type)
	:
	object(object),
	type(type)
{
}


SimpleJobKey::SimpleJobKey(const SimpleJobKey& other)
	:
	object(other.object),
	type(other.type)
{
}


size_t
SimpleJobKey::HashValue() const
{
	return (size_t)(addr_t)object ^ (size_t)type;
}


bool SimpleJobKey::operator==(const JobKey& other) const
{
	const SimpleJobKey* otherKey = dynamic_cast<const SimpleJobKey*>(&other);
	return otherKey != NULL && object == otherKey->object
		&& type == otherKey->type;
}


SimpleJobKey&
SimpleJobKey::operator=(const SimpleJobKey& other)
{
	object = other.object;
	type = other.type;
	return *this;
}


// #pragma mark - JobListener


JobListener::~JobListener()
{
}


void JobListener::JobStarted(Job* job)
{
}


void JobListener::JobDone(Job* job)
{
}


void JobListener::JobWaitingForInput(Job* job)
{
}


void JobListener::JobFailed(Job* job)
{
}


void JobListener::JobAborted(Job* job)
{
}


// #pragma mark - Job


Job::Job()
	:
	fWorker(NULL),
	fState(JOB_STATE_UNSCHEDULED),
	fDependency(NULL),
	fWaitStatus(JOB_DEPENDENCY_NOT_FOUND),
	fListeners(10)
{
}


Job::~Job()
{
}


job_wait_status
Job::WaitFor(const JobKey& key)
{
	return fWorker->WaitForJob(this, key);
}


status_t Job::WaitForUserInput()
{
	return fWorker->WaitForUserInput(this);
}


void Job::SetDescription(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	fDescription.SetToFormatVarArgs(format, args);
}


void Job::SetWorker(Worker* worker)
{
	fWorker = worker;
}


void Job::SetState(job_state state)
{
	fState = state;
}


void Job::SetDependency(Job* job)
{
	fDependency = job;
}


void Job::SetWaitStatus(job_wait_status status)
{
	fWaitStatus = status;
	switch (fWaitStatus) {
		case JOB_DEPENDENCY_ACTIVE:
		case JOB_USER_INPUT_WAITING:
			fState = JOB_STATE_WAITING;
			break;
		default:
			fState = JOB_STATE_ACTIVE;
			break;
	}
}


status_t Job::AddListener(JobListener* listener)
{
	return fListeners.AddItem(listener) ? B_OK : B_NO_MEMORY;
}


void Job::RemoveListener(JobListener* listener)
{
	fListeners.RemoveItem(listener);
}


void Job::NotifyListeners()
{
	int32_t count = fListeners.CountItems();
	for (int32_t i = count - 1; i >= 0; i--) {
		JobListener* listener = fListeners.ItemAt(i);
		switch (fState) {
			case JOB_STATE_ACTIVE:
				listener->JobStarted(this);
				break;
			case JOB_STATE_WAITING:
				if (fWaitStatus == JOB_USER_INPUT_WAITING)
					listener->JobWaitingForInput(this);
				break;
			case JOB_STATE_SUCCEEDED:
				listener->JobDone(this);
				break;
			case JOB_STATE_FAILED:
				listener->JobFailed(this);
				break;
			case JOB_STATE_ABORTED:
			default:
				listener->JobAborted(this);
				break;
		}
	}
}


// #pragma mark - Worker


Worker::Worker()
	:
	fLock("worker"),
	fWorkerThread(-1),
	fTerminating(false)
{
}


Worker::~Worker()
{
	ShutDown();

	if (fWorkerThread >= 0)
		wait_for_thread(fWorkerThread, NULL);
}


status_t Worker::Init()
{
	// check lock
	status_t error = fLock.InitCheck();
	if (error != B_OK)
		return error;

	// init jobs table
	error = fJobs.Init();
	if (error != B_OK)
		return error;

	// create semaphore for the worker
	fWorkToDoSem = create_sem(0, "work to do");
	if (fWorkToDoSem < 0)
		return fWorkToDoSem;

	// spawn worker thread
	fWorkerThread = spawn_thread(_WorkerLoopEntry, "worker", B_NORMAL_PRIORITY,
		this);
	if (fWorkerThread < 0)
		return fWorkerThread;

	resume_thread(fWorkerThread);

	return B_OK;
}


void Worker::ShutDown()
{
	AutoLocker<Worker> locker(this);

	if (fTerminating)
		return;

	fTerminating = true;

	// abort all jobs
	Job* job = fJobs.Clear(true);
	while (job != NULL) {
		Job* nextJob = job->fNext;
		_AbortJob(job, false);
		job = nextJob;

	}

	// let the work thread terminate
	delete_sem(fWorkToDoSem);
	fWorkToDoSem = -1;
}


status_t Worker::ScheduleJob(Job* job, JobListener* listener)
{
	if (job == NULL)
		return B_NO_MEMORY;

	BReference<Job> jobReference(job, true);
	AutoLocker<Worker> locker(this);

	if (fTerminating)
		return B_ERROR;

	if (listener != NULL) {
		status_t error = job->AddListener(listener);
		if (error != B_OK)
			return error;
	}

	bool notify = fUnscheduledJobs.IsEmpty() && fAbortedJobs.IsEmpty();

	job->SetWorker(this);
	job->SetState(JOB_STATE_UNSCHEDULED);
	fJobs.Insert(job);
	fUnscheduledJobs.Add(jobReference.Detach());

	if (notify)
		release_sem(fWorkToDoSem);

	return B_OK;
}


void Worker::AbortJob(const JobKey& key)
{
	AutoLocker<Worker> locker(this);

	Job* job = fJobs.Lookup(key);
	if (job == NULL)
		return;

	_AbortJob(job, true);
}


Job*
Worker::GetJob(const JobKey& key)
{
	AutoLocker<Worker> locker(this);
	return fJobs.Lookup(key);
}


status_t Worker::ResumeJob(Job* job)
{
	AutoLocker<Worker> locker(this);

	for (JobList::Iterator it = fSuspendedJobs.GetIterator(); it.Next();) {
		if (it.Current() == job) {
			it.Remove();
			job->SetState(JOB_STATE_UNSCHEDULED);
			fUnscheduledJobs.Add(job);
			release_sem(fWorkToDoSem);
			return B_OK;
		}
	}

	return B_ENTRY_NOT_FOUND;
}


bool Worker::HasPendingJobs()
{
	AutoLocker<Worker> locker(this);
	return !fJobs.IsEmpty();
}


status_t Worker::AddListener(const JobKey& key, JobListener* listener)
{
	AutoLocker<Worker> locker(this);

	Job* job = fJobs.Lookup(key);
	if (job == NULL)
		return B_ENTRY_NOT_FOUND;

	return job->AddListener(listener);
}


void Worker::RemoveListener(const JobKey& key, JobListener* listener)
{
	AutoLocker<Worker> locker(this);

	if (Job* job = fJobs.Lookup(key))
		job->RemoveListener(listener);
}


job_wait_status
Worker::WaitForJob(Job* waitingJob, const JobKey& key)
{
	AutoLocker<Worker> locker(this);

	// don't wait when the game is over anyway
	if (fTerminating || waitingJob->State() == JOB_STATE_ABORTED)
		return JOB_DEPENDENCY_ABORTED;

	Job* job = fJobs.Lookup(key);
	if (job == NULL)
		return JOB_DEPENDENCY_NOT_FOUND;

	waitingJob->SetWaitStatus(JOB_DEPENDENCY_ACTIVE);
	waitingJob->SetDependency(job);
	job->DependentJobs().Add(waitingJob);

	return waitingJob->WaitStatus();
}


status_t Worker::WaitForUserInput(Job* waitingJob)
{
	AutoLocker<Worker> locker(this);

	if (fTerminating || waitingJob->State() == JOB_STATE_ABORTED)
		return B_INTERRUPTED;

	waitingJob->SetWaitStatus(JOB_USER_INPUT_WAITING);
	waitingJob->NotifyListeners();
	fSuspendedJobs.Add(waitingJob);

	return B_OK;
}


/*static*/ status_t Worker::_WorkerLoopEntry(void* data)
{
	return ((Worker*)data)->_WorkerLoop();
}


status_t Worker::_WorkerLoop()
{
	_ProcessJobs();

	// clean up aborted jobs
	AutoLocker<Worker> locker(this);
	while (Job* job = fAbortedJobs.RemoveHead())
		_FinishJob(job);

	return B_OK;
}


void Worker::_ProcessJobs()
{
	while (true) {
		AutoLocker<Worker> locker(this);

		// wait for next job
		if (fUnscheduledJobs.IsEmpty() && fAbortedJobs.IsEmpty()) {
			locker.Unlock();

			status_t error = acquire_sem(fWorkToDoSem);
			if (error != B_OK) {
				if (error == B_INTERRUPTED) {
					locker.Lock();
					continue;
				}
				break;
			}

			locker.Lock();
		}

		// clean up aborted jobs
		while (Job* job = fAbortedJobs.RemoveHead())
			_FinishJob(job);

		// process the next job
		if (Job* job = fUnscheduledJobs.RemoveHead()) {
			job->SetState(JOB_STATE_ACTIVE);
			job->NotifyListeners();

			locker.Unlock();
			status_t error = job->Do();
			locker.Lock();

			if (job->State() == JOB_STATE_ACTIVE) {
				job->SetState(
					error == B_OK ? JOB_STATE_SUCCEEDED : JOB_STATE_FAILED);
			} else if (job->State() == JOB_STATE_WAITING)
				continue;

			_FinishJob(job);
		}
	}
}


void Worker::_AbortJob(Job* job, bool removeFromTable)
{
	switch (job->State()) {
		case JOB_STATE_ABORTED:
			return;

		case JOB_STATE_UNSCHEDULED:
			fUnscheduledJobs.Remove(job);
			fAbortedJobs.Add(job);
			break;

		case JOB_STATE_WAITING:
		{
			Job* dependency = job->Dependency();
			if (dependency != NULL)
				dependency->DependentJobs().Remove(job);
			job->SetDependency(NULL);
			break;
		}
		case JOB_STATE_ACTIVE:
		case JOB_STATE_FAILED:
		case JOB_STATE_SUCCEEDED:
		default:
			break;
	}

	job->SetState(JOB_STATE_ABORTED);
	if (removeFromTable)
		fJobs.Remove(job);
}


void Worker::_FinishJob(Job* job)
{
	// wake up dependent jobs
	if (!job->DependentJobs().IsEmpty()) {
		job_wait_status waitStatus;
		switch (job->State()) {
			case JOB_STATE_ABORTED:
				waitStatus = JOB_DEPENDENCY_ABORTED;
				break;
			case JOB_STATE_FAILED:
				waitStatus = JOB_DEPENDENCY_FAILED;
				break;
			case JOB_STATE_SUCCEEDED:
				waitStatus = JOB_DEPENDENCY_SUCCEEDED;
				break;

			case JOB_STATE_UNSCHEDULED:
			case JOB_STATE_WAITING:
			case JOB_STATE_ACTIVE:
			default:
				// should never happen
				waitStatus = JOB_DEPENDENCY_NOT_FOUND;
				break;
		}

		while (Job* dependentJob = job->DependentJobs().RemoveHead()) {
			dependentJob->SetDependency(NULL);
			dependentJob->SetWaitStatus(waitStatus);
			fUnscheduledJobs.Add(dependentJob);
		}

		release_sem(fWorkToDoSem);
	}

	if (job->State() != JOB_STATE_ABORTED)
		fJobs.Remove(job);
	job->NotifyListeners();
	job->ReleaseReference();
}
