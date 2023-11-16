#include "vfs.h"

//
//	IRP_MJ_CLEANUP
//
NTSTATUS VfsCleanup(PDEVICE_OBJECT device, PIRP irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	struct vfs_irp_context *irp_context = NULL;
	BOOLEAN top_level = FALSE;

	VfsDbgPrintCall(device, irp);

	FsRtlEnterFileSystem();
	top_level = VfsIsIrpTopLevel(irp);

	__try {
		irp_context = VfsAllocIrpContext(irp, device);
		ASSERT(irp_context);

		status = VfsCommonCleanup(irp_context, irp, TRUE);
	}
	__except(VfsExceptionFilter(irp_context, GetExceptionInformation())) {
		status = VfsExceptionHandler(irp_context, irp);
	}

	if (top_level)
		IoSetTopLevelIrp(NULL);

	FsRtlExitFileSystem();

	return status;
}

//
//	cleanup code
//
NTSTATUS VfsCommonCleanup(struct vfs_irp_context *irp_context, PIRP irp, BOOLEAN first_attempt)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stack_location = NULL;
	PFILE_OBJECT file_obj = NULL;
	struct vfs_vcb *vcb = NULL;
	struct vfs_fcb *fcb = NULL;
	struct vfs_ccb *ccb = NULL;
	PERESOURCE resource_acquired = NULL;
	IO_STATUS_BLOCK iosb;
	BOOLEAN complete_irp = TRUE;
	BOOLEAN post_request = FALSE;
	BOOLEAN vcb_acquired = FALSE;
	BOOLEAN can_wait = FALSE;
	BOOLEAN block_for_resource = FALSE;
	int i=1;

	__try {
		// always succed for fs device
		CHECK_OUT(irp_context->target_device == vfs_data.device, STATUS_SUCCESS);

		// stack location
		stack_location = IoGetCurrentIrpStackLocation(irp);
		ASSERT(stack_location);

		// file object we're to clean
		file_obj = stack_location->FileObject;
		ASSERT(file_obj);

		// always succed for this kind - FsContext2 should only be NULL for the stream object we created
		// but i dont understand how this routine is called
		//if (!file_obj->FsContext2) {
		//	DbgPrint("CLEANUP stream object\n");
		//	TRY_RETURN(STATUS_SUCCESS);
		//}

		// so. lets identify the file
		VfsGetFcbCcbVcbFromFileObject(file_obj, &fcb, &ccb, &vcb);

		// just to be sure ...
		vcb = (struct vfs_vcb*)(irp_context->target_device->DeviceExtension);
		ASSERT(vcb);

		// 1. sync cleanup requests with create requests by acquiring the vcb resource exclusively

		// if we're not called for the first time it means that we're called in the context of the system thread
		// we have created for async processing. so, try a few times(3) to get the file lock before requeing the request
		if (!ExAcquireResourceExclusiveLite(&(vcb->vcb_resource), FALSE)) {
			post_request = TRUE;
			TRY_RETURN(STATUS_PENDING);
		} else
			vcb_acquired = TRUE;
		/*block_for_resource = !first_attempt;
		i=1;
		while (!vcb_acquired) {
			if (!ExAcquireResourceExclusiveLite(&(vcb->vcb_resource), FALSE)) {
				if (block_for_resource && i != 1000) {
					LARGE_INTEGER delay;
					delay.QuadPart = -500*i;
					KeDelayExecutionThread(KernelMode, FALSE, &delay);
				} else {
					post_request = TRUE;
					TRY_RETURN(STATUS_PENDING);
				}
			} else
				vcb_acquired = TRUE;
			i *= 10;
		}*/

		// 2. get the main resource for the file obj to sync with othe user initiated I/O on the same file stream
		// remember, that there may be other threads that hold a handle / reference to the file
		// same trick used to get the lock

		if (fcb && fcb->identifier.type == VFS_NODE_TYPE_FCB) {
			i=1;
			if (!ExAcquireResourceExclusiveLite(&(fcb->required_fcb.main_resource), FALSE)) {
				post_request = TRUE;
				TRY_RETURN(STATUS_PENDING);
			} else
				resource_acquired = &(fcb->required_fcb.main_resource);
			/*while (!resource_acquired) {
				if (!ExAcquireResourceExclusiveLite(&(fcb->required_fcb.main_resource), FALSE)) {
					if (block_for_resource && i != 1000) {
						LARGE_INTEGER delay;
						delay.QuadPart = -500*i;
						KeDelayExecutionThread(KernelMode, FALSE, &delay);
					} else {
						post_request = TRUE;
						TRY_RETURN(STATUS_PENDING);
					}
				} else
					resource_acquired = &(fcb->required_fcb.main_resource);
				i *= 10;
			}*/

			// 3. if this were a network file system we would treat oplocks here -- not the case
			// this is some kind of cache syncronization between the server and the client

			// 4. if this is a dir, i should invoke FsRtlNotifyCleanup() now
			// -- this doesent work -- mutex bug
			//if (S_ISDIR(fcb->inode->i_mode))
			//	FsRtlNotifyCleanup(&vcb->notify_irp_mutex, &vcb->next_notify_irp, file_obj->FsContext2);

			// 5. unlock byte ranges that were locked - only if this is a file, right now -- later

			// 6. update appropriate time stamps values -- iput does it -- i don't need to do anything yet
			// also, check if this was accesed through fast io

			// 7. if this is the last cleanup request and if there are if there are any pending truncate/delete 
			// requests i should treat them now -- TODO
			// - check if the link count is 0 and acquire PagingIoResource exclusively - set file size and valid data length to 0
			// - invoke MmFlushImageSection to purge any pages containing mapped data

			// - if current link is being deleted, remove the directory entry
			// - invoke FsRtlNotifyFullReportChange()

			if (FLAG_ON(fcb->flags, VFS_FCB_DELETE_ON_CLOSE) && !fcb->handle_count && vcb->super_block->s_op->delete_inode)
				vcb->super_block->s_op->delete_inode(fcb->inode);

			// 8. decrement open file handle count for the fcb
			ASSERT(fcb->handle_count);
			fcb->handle_count--;
			vcb->open_count--;

			// we flush the cache, just to make sure everything gets to disk right now
			// this is not efficient - cache will be flushed some other time after we uninitialized the cache map ...
			// to REMOVE - after debug
			if (file_obj->PrivateCacheMap != NULL)
				CcFlushCache(file_obj->SectionObjectPointer, NULL, 0, &iosb);

			// -- added by sadyc, is ot ok ?? --todo:
			CcPurgeCacheSection( file_obj->SectionObjectPointer,NULL, 0, FALSE );

			//
			//	--- danny --- temporary: becauze we don't have cache
			// we do have cache now :)) --sadyc
			//if (!fcb->handle_count)
			//	MmForceSectionClosed(&fcb->required_fcb.section_object, FALSE);

			// 9. uninitialize cache map even if caching hasnt been initialized
			// TODO - the 2nd parameter should be the new file size if the file has been truncated
			// the cache mgr will purge the truncated pages right away since they will no longer be needed
			CcUninitializeCacheMap(file_obj, NULL, NULL);

			// 11. invoke IoRemoveShareAccess
			IoRemoveShareAccess(file_obj, &fcb->share_access);
			
		} else {
			//vcb->open_count--;
			complete_irp = TRUE;
		}
try_exit:
		;
	}
	__finally {
		// 10. don't forget setting FO_CLEANUP_COMPLETE - i do this here in order to do it for both volume or file close
		if (file_obj)
			SET_FLAG(file_obj->Flags, FO_CLEANUP_COMPLETE);
		// cleanup
		if (resource_acquired)
			RELEASE(resource_acquired);
		if (vcb_acquired)
			RELEASE(&vcb->vcb_resource);
		if (post_request)
			status = VfsPostRequest(irp_context, irp);
		if (status != STATUS_PENDING) {
			VfsDbgPrintComplete(irp);
			VfsFreeIrpContext(irp_context);
			IoCompleteRequest(irp, IO_DISK_INCREMENT);
		}
	}
	return status;
}