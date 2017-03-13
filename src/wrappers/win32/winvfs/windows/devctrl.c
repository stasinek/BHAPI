#include "vfs.h"

//
//	IRP_MJ_DEVICE_CONTROL
//
NTSTATUS VfsDeviceControl(PDEVICE_OBJECT device, PIRP irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	struct vfs_irp_context *irp_context;
	BOOLEAN top_level;

	VfsDbgPrintCall(device, irp);

	FsRtlEnterFileSystem();
	ASSERT(device);
	ASSERT(irp);

	top_level = VfsIsIrpTopLevel(irp);

	__try
	{
		irp_context = VfsAllocIrpContext(irp, device);
		ASSERT(irp_context);
		status = VfsCommonDeviceControl(irp_context, irp);
	}
	__except (VfsExceptionFilter(irp_context, GetExceptionInformation()))
	{
		status = VfsExceptionHandler(irp_context, irp);
	}

	if (top_level)
		IoSetTopLevelIrp(NULL);

	FsRtlExitFileSystem();

	return status;
}

//
//	device ioctls
//
NTSTATUS VfsCommonDeviceControl(struct vfs_irp_context *irp_context, PIRP irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stack_location = NULL;
	PIO_STACK_LOCATION next_stack_location = NULL;
	ULONG ioctl = 0;
	//struct vfs_ccb *ccb = NULL;
	struct vfs_vcb *vcb = NULL;
	struct vfs_fcb *fcb = NULL;
	PFILE_OBJECT file_obj = NULL;

	__try
	{
		stack_location = IoGetCurrentIrpStackLocation(irp);
		ASSERT(stack_location);

		ioctl = stack_location->Parameters.DeviceIoControl.IoControlCode;

		if (ioctl == IOCTL_PREPARE_TO_UNLOAD) {
			status = VfsPrepareToUnload(irp_context, irp);
			__leave;
		}

		//DbgPrint("unknown device control request\n");

		file_obj = stack_location->FileObject;
		ASSERT(stack_location);

		// stream_file_obj has ccb=NULL
		// who keeps sending these requests to the stream_file_obj?
		//ccb = (struct vfs_ccb*)file_obj->FsContext2;
		//ASSERT(ccb);

		fcb = (struct vfs_fcb*)file_obj->FsContext;
		ASSERT(fcb);
		
		if (fcb->identifier.type == VFS_NODE_TYPE_VCB) {
			vcb = (struct vfs_vcb*)fcb;
		} else {
			vcb = fcb->vcb;
		}
		ASSERT(vcb);

		next_stack_location = IoGetNextIrpStackLocation(irp);
		*next_stack_location = *stack_location;
		IoSetCompletionRoutine(irp, VfsIoctlCompletion, NULL, TRUE, TRUE, TRUE);
		status = IoCallDriver(vcb->target_device, irp);
	}
	__finally
	{
		if (!FLAG_ON(irp_context->flags, VFS_IRP_CONTEXT_EXCEPTION))
			VfsFreeIrpContext(irp_context);
	}
	return status;
}

//
//	IOCTL_PREPARE_TO_UNLOAD
//
NTSTATUS VfsPrepareToUnload(struct vfs_irp_context *irp_context, PIRP irp)
{
	PDEVICE_OBJECT device = NULL;
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN acq_resource = FALSE;

	//DbgPrint("PREPARE TO UNLOAD CALLED\n");

	__try
	{
		ASSERT(irp_context);

		device = irp_context->target_device;
		CHECK_OUT(device != vfs_data.device, STATUS_INVALID_DEVICE_REQUEST);

		ExAcquireResourceExclusiveLite(&vfs_data.global_resource, TRUE);
		acq_resource = TRUE;

		CHECK_OUT(FLAG_ON(vfs_data.flags, VFS_UNLOAD_PENDING), STATUS_ACCESS_DENIED);
		//DbgPrint("NO UNLOAD PENDING\n");
		CHECK_OUT(!IsListEmpty(&vfs_data.vcb_list), STATUS_ACCESS_DENIED);
		//DbgPrint("THE VCB LIST IS EMPTY\n");

		IoUnregisterFileSystem(vfs_data.device);
		IoDeleteDevice(vfs_data.device);
		
		SET_FLAG(vfs_data.flags, VFS_UNLOAD_PENDING);
try_exit:
		;
	}
	__finally
	{
		if (acq_resource)
			RELEASE(&vfs_data.global_resource);
		if (!AbnormalTermination()) {
			irp->IoStatus.Status = status;
			irp->IoStatus.Information = 0;
			VfsDbgPrintComplete(irp);
			IoCompleteRequest(irp, NT_SUCCESS(status) ? IO_DISK_INCREMENT : IO_NO_INCREMENT);
		}
	}
	return status;
}

//
//	mark the irp pending if it was sent to the underlying device
//
NTSTATUS VfsIoctlCompletion(PDEVICE_OBJECT device, PIRP irp, PVOID context)
{
	if (irp->PendingReturned)
		IoMarkIrpPending(irp);
	return STATUS_SUCCESS;
}
