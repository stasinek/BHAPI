#include "vfs.h"

//
//	IRP_MJ_CLOSE
//
NTSTATUS VfsClose(PDEVICE_OBJECT device, PIRP irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	struct vfs_irp_context *irp_context = NULL;
	BOOLEAN top_level = FALSE;

	VfsDbgPrintCall(device, irp);

	FsRtlEnterFileSystem();

	top_level = VfsIsIrpTopLevel(irp);

	__try {
		irp_context = VfsAllocIrpContext(irp, device);
		status = VfsCommonClose(irp_context, irp, TRUE);
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
//	close code
//
NTSTATUS VfsCommonClose(struct vfs_irp_context *irp_context, PIRP irp, BOOLEAN first_attempt)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stack_location = NULL;
	PFILE_OBJECT file_obj = NULL;
	struct vfs_vcb* vcb = NULL;
	struct vfs_fcb* fcb = NULL;
	struct vfs_ccb* ccb = NULL;
	struct vfs_required_fcb* required_fcb = NULL;
	PERESOURCE resource_acquired = NULL;
	IO_STATUS_BLOCK iosb;
	BOOLEAN complete_irp = TRUE;
	BOOLEAN post_request = FALSE;
	BOOLEAN vcb_acquired = FALSE;
	BOOLEAN can_wait = FALSE;
	BOOLEAN block_for_resource;
	BOOLEAN free_vcb = FALSE;
	int i=1;

	__try {
		// always succed for fs device
		CHECK_OUT(irp_context->target_device == vfs_data.device, STATUS_SUCCESS);

//		DbgPrint("not fs device \n");

		// stack location
		stack_location = IoGetCurrentIrpStackLocation(irp);
		ASSERT(stack_location);

		// file object
		file_obj = stack_location->FileObject;
		ASSERT(file_obj);

		// always succed for this kind - FsContext2 should only be NULL for the stream object we created
		// but i dont understand how this routine is called
		//if (!file_obj->FsContext2) {
		//	DbgPrint("CLOSE stream object\n");
		//	TRY_RETURN(STATUS_SUCCESS);
		//}

		// 1. get a pointer to the fcb and ccb for the file object
		VfsGetFcbCcbVcbFromFileObject(file_obj, &fcb, &ccb, &vcb);
		// just to make sure ...
		vcb = (struct vfs_vcb*)(irp_context->target_device->DeviceExtension);
		ASSERT(vcb);

		// 2. sync close/cleanup/create - acquire vcb exclusively
		// - NEVER EVER BLOCK IN A CLOSE REQUEST -
		if (!ExAcquireResourceExclusiveLite(&vcb->vcb_resource, FALSE)) {
			post_request = TRUE;
			TRY_RETURN(STATUS_PENDING);
		} else {
			vcb_acquired = TRUE;
		}
		/*block_for_resource = !first_attempt;
		i = 1;
		while (!vcb_acquired) {
			if (!ExAcquireResourceExclusiveLite(&vcb->vcb_resource, FALSE)) {
				if (block_for_resource && i != 1000) {
					LARGE_INTEGER delay;
					delay.QuadPart = -1;//-500*i;
					KeDelayExecutionThread(KernelMode, FALSE, &delay);
				} else {
					post_request = TRUE;
					TRY_RETURN(STATUS_PENDING);
				}
			} else
				vcb_acquired = TRUE;
			i *= 10;
		}*/

		// this is a file, not a volume close
		if (fcb && fcb->identifier.type == VFS_NODE_TYPE_FCB) {
			// acquire fcb lock - i'm not sure it's necessary
			i = 1;
			required_fcb = &(fcb->required_fcb);
			if (!ExAcquireResourceExclusiveLite(&(fcb->required_fcb.main_resource), FALSE)) {
				post_request = TRUE;
				TRY_RETURN(STATUS_PENDING);
			} else
				resource_acquired = &(fcb->required_fcb.main_resource);
			/*if (!ExAcquireResourceExclusiveLite(&(fcb->required_fcb.main_resource), FALSE)) {
				if (block_for_resource && i != 1000) {
					LARGE_INTEGER delay;
					delay.QuadPart = -500*i;
					KeDelayExecutionThread(KernelMode, FALSE, &delay);
				} else {
					post_request = TRUE;
					TRY_RETURN(STATUS_PENDING);
				}
			} else
				resource_acquired = &(fcb->required_fcb.main_resource);*/

			// 3. delete the ccb - close the file
			ASSERT(ccb);
			RemoveEntryList(&ccb->next);

			VfsCloseAndFreeCcb(ccb);

			file_obj->FsContext2 = NULL;

			// decrement reference count
			ASSERT(fcb->reference_count);
			ASSERT(vcb->reference_count);
			fcb_dec_ref_count(fcb);
			vcb_dec_ref_count(vcb);

			// 4. if this is the last close on this fcb, delete the fcb, call iput
			if (fcb->reference_count == 0) {
				//iput(fcb->inode);	// nu cred ca mai e nevoie cred ca in filp_close se face asta daca e nevoie
				RemoveEntryList(&fcb->next);
				RELEASE(resource_acquired);
				resource_acquired = NULL;
				VfsFreeFcb(fcb);
			}
			complete_irp = TRUE;
		} else {
			ASSERT(vcb->reference_count);
			vcb_dec_ref_count(vcb);

			// if this is a volume close and the volume is being dismounted, free the vcb
			if (vcb->reference_count == 0 && FLAG_ON(vcb->flags, VFS_VCB_FLAGS_BEING_DISMOUNTED))
				free_vcb = TRUE;
			complete_irp = TRUE;
		}
try_exit:
		;
	}
	__finally {
		if (resource_acquired)
			RELEASE(resource_acquired);
		if (vcb_acquired)
			RELEASE(&vcb->vcb_resource);
		if (post_request) {
			DbgPrint("CLOSE POSTED REQUEST\n");
			status = VfsPostRequest(irp_context, irp);
			//status = STATUS_SUCCESS;	// close should not return status pending
			// danny -------- todo: adaugat file object in irp context + terminat aici cu STATUS_SUCCESS
			// + completat irp-ul + setat irp NULL si terminat aici numai in cazul in care e diferit de NULL
		}
		else if (complete_irp && status != STATUS_PENDING) {
			VfsDbgPrintComplete(irp);
			IoCompleteRequest(irp, IO_DISK_INCREMENT);
			VfsFreeIrpContext(irp_context);
			if (free_vcb)
				VfsFreeVcb(vcb);
		}
	}
	return status;
}