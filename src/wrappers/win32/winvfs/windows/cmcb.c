#include "vfs.h"

//
//	acquire for lazy write
//
BOOLEAN VfsAcqLazyWrite(PVOID context, BOOLEAN wait)
{
	struct vfs_fcb *fcb = NULL;
	struct vfs_ccb *ccb = NULL;
	struct vfs_required_fcb* req_fcb = NULL;

	ASSERT(context);
	ccb = (struct vfs_ccb*)context;
	fcb = ccb->fcb;
	ASSERT(fcb);
	req_fcb = &(fcb->required_fcb);

	if (!ExAcquireResourceExclusiveLite(&(req_fcb->main_resource), wait)) {
		return FALSE;
	} 
		
	//ASSERT(!(fcb->lazy_writer_tid));
	//fcb->lazy_writer_tid = (ULONG)PsGetCurrentThread();

	return TRUE;
}

//
//	release from lazy write
//
void VfsRelLazyWrite(PVOID context)
{
	struct vfs_fcb *fcb = NULL;
	struct vfs_ccb *ccb = NULL;
	struct vfs_required_fcb* req_fcb = NULL;

	ASSERT(context);
	ccb = (struct vfs_ccb*)context;
	fcb = ccb->fcb;
	ASSERT(fcb);
	req_fcb = &(fcb->required_fcb);

	//ASSERT(fcb->lazy_writer_tid == (ULONG)PsGetCurrentThread());
	//fcb->lazy_writer_tid = 0;

	RELEASE(&(req_fcb->main_resource));
}

//
//	acquire for read ahead
//
BOOLEAN VfsAcqReadAhead(PVOID context, BOOLEAN wait)
{
	struct vfs_fcb *fcb = NULL;
	struct vfs_ccb *ccb = NULL;
	struct vfs_required_fcb* req_fcb = NULL;

	ASSERT(context);
	ccb = (struct vfs_ccb*)context;
	fcb = ccb->fcb;
	ASSERT(fcb);
	req_fcb = &(fcb->required_fcb);

	if (!ExAcquireResourceSharedLite(&(req_fcb->main_resource), wait)) {
		return FALSE;
	}
	return TRUE;
}

//
//	release from read ahead
//
void VfsRelReadAhead(PVOID context)
{
	struct vfs_fcb *fcb = NULL;
	struct vfs_ccb *ccb = NULL;
	struct vfs_required_fcb* req_fcb = NULL;

	ASSERT(context);
	ccb = (struct vfs_ccb*)context;
	fcb = ccb->fcb;
	ASSERT(fcb);
	req_fcb = &(fcb->required_fcb);

	//ASSERT(fcb->lazy_writer_tid == (ULONG)PsGetCurrentThread());
	//fcb->lazy_writer_tid = 0;

	RELEASE(&(req_fcb->main_resource));
}
